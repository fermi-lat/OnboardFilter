/* ---------------------------------------------------------------------- *//*!
   
   \file  EBF_directory.c
   \brief EBF directory Utilities
   \author JJRussell - russell@slac.stanford.edu

   \verbatim
    CVS $Id$
   \endverbatim 
                                                                         */
/* --------------------------------------------------------------------- */


#include "DFC/EBF_cid.h"
#include "DFC/LCB.h"
#include "DFC/EBF_directory.h"

#include "windowsCompat.h"

static inline unsigned char calLogCnt (unsigned int calCnt,
                                       unsigned int   mask);



/* --------------------------------------------------------------------- *//*!

   \def    _ADVANCE(_ptr, _nbytes)
   \brief   Advances \a _ptr by \a _nbytes. The return value is always
            recast to the type of \a _ptr.
   \param   _ptr    The pointer to advance
   \param   _nbytes The number of nbytes to advance \a _ptr by.
   \return  The advanced pointer, recast to the type of \a _ptr.
                                                                         */
/* --------------------------------------------------------------------- */
#define _ADVANCE(_ptr, _nbytes) \
         ((unsigned char *)(_ptr) + (_nbytes))
/* --------------------------------------------------------------------- */






/* --------------------------------------------------------------------- *//*!

   \fn unsigned char calLogCnt (unsigned int calcnt, unsigned int mask)
   
   \brief         Calculates the total number of logs struck in this
                  contributor's CAL record.
                  
   \param  calCnt 8 nibbles representing the number of counts in each
                  of the 8 CAL layers.
                  
   \param    mask A bit mask with every other nibble being 0 of 0xf.
                  This is really not a parameter, but including allows
                  the compiler to optimize its placement in a register.
   
   \return        The total number of logs struck in this contributor's
                  CAL record.
                                                                         */
/* --------------------------------------------------------------------- */
static unsigned char calLogCnt (unsigned int calcnt, unsigned int mask)
{
   /*
    | This code computes the number of struck logs in the CAL.
    |
    | This method of adding the log counts is somewhat more efficient than
    | just picking off each nibble and adding them (roughly 16 instructions).
    | This method masks off the odd nibbles and then the even nibbles. The
    | word with the odd nibbles remaining is shifted down by one nibble
    | before adding. The first add sums 8 nibbles to 4 nibbles, the second
    | sums 4 nibbles to 2 nibbles and the third sums 2 nibbles to 1. Masking
    | off the every other nibble prevents the adds from 'leaking' across
    | the boundaries. The final sum is in the lower byte.
    |
    | Note that there is no danger of overflow. The biggest possible number
    | is 15 * 8 = 120 (the largest practical number is 12 * 8 = 96, ie the
    | number of logs in one tower.)
    |
    | Example:
    | Consider the following CAL count word, where the nibbles a,b,c,d,e,f,g,h
    | represent the number of logs hit for layers 0-7. The objecct is to find
    | the sum of a + b + c + d + e + f + g + h
    |
    |      CAL Count Word           = hgfedcba
    |      Mask        even nibbles = 0g0e0c0a
    |      Mask shifted odd nibbles = 0h0f0d0b
    |      Add the above two        = 0s0r0q0p  (sum 1)
    |      Shift down by 16         = 00000s0r
    |      Add the above two        = 0X0X0u0t  (sum 2)
    |      Shift down by  8         = 0X0X0X0u
    |      Add the above two        = 0X0X0X0v  (sum 3)
    |
    | The answer is the lower byte. Notice that
    |
    | Sum 1   p = a + b     
    |         q = c + d
    |         r = e + f
    |         s = g + h
    |
    | Sum 2   t = p + r = a + b + e + f
    |         u = q + s = c + d + g + h
    |
    | Sum 3   v = t + u = a + b + e + f + c + d + g + h
    |
    | The following lines of code accomplish this task
    | in roughly 7 instructions compared to the roughly
    | 16 used in doing this the conventional way.
   */
   calcnt  = (calcnt & mask) + ((calcnt >>  4) & mask); /* Adds 8 => 4  */
   calcnt +=  calcnt >> 16;                             /* Adds 4 => 2  */
   calcnt +=  calcnt >>  8;                             /* Adds 2 => 1  */

   return calcnt;
}
/* --------------------------------------------------------------------- */






/* --------------------------------------------------------------------- *//*!

  \fn int EBF_directoryCompose (EBF_directory      *dir,
                                const unsigned int *evt)
                                
  \brief       Composes the directory of the contributors
  \param   dir An array of directories to be filled in
  \param   evt The source event.
  \return      The number of directories (contributors). If this number
               is negative, an error occurred during the decoding. 
                                                                         */
/* --------------------------------------------------------------------- */
int EBF_directoryCompose (EBF_directory      *dir,
                          const unsigned int *evt)
{
   int                     ncids;
   int                      elen;
   unsigned int             cids;
   unsigned int            ctids;
   EBF_contributor *contributors;
   unsigned int             mask;

   
   /*
    |  Retrieve the total size of this event (in bytes). Since the event
    |  length word is included in the total length, the remaining number
    |  of bytes (elen) is reduced by the size of the event length word.
   */
   elen = *evt++ - sizeof (*evt);

   
   /*
    |  Initialize the number of contributors. This is started at one,
    |  since every event has an artificial contributor which describes
    |  the event as a whole.
   */
   ncids = 1;
   cids  = (0x80000000 >> EBF_K_CID_EVT);

   
   /*
    |  Initialize a bit mask indicating the CAL and TKR towers which
    |  have non-empty contributions to this event.
   */
   ctids = 0;

   
   /* Get a pointer to the array contributor descriptors. */
   contributors = dir->contributors;

   
   /*
    |  This is an obscure varible used when doing the optimized sum
    |  over the CAL log counts in each layer. Ideally it would appear
    |  in the summer, but putting it here encourages the compiler
    |  to put it in a register, saving the summing code from loading
    |  this variable into a register every time.
   */
   mask  = 0x0f0f0f0f;

   
   /*
    |  Save the global event information. Note that the saved pointer
    |  points at the first contributor, not the event length word.
    |  The saved event length, similarly, reflects the length of all
    |  the contributors, excluding the event length word.
   */
   contributors[EBF_K_CID_EVT].cid = EBF_K_CID_EVT;
   contributors[EBF_K_CID_EVT].ptr = evt;
   contributors[EBF_K_CID_EVT].len = elen;
   
   
   while (elen > 0)
   {
       unsigned int        hdr_clen;
       unsigned int            clen;
       unsigned int          cidmsk;
       int                      cid;
       const unsigned int      *ptr;
       EBF_contributor *contributor;
       

       /*
        |  Get the length of the event in bytes and reduce the number of
        |  unprocessed bytes
       */
       hdr_clen = ((LCB_header *)evt)->hdr_clen;
       clen     = LCB_EXTRACT_CLEN (hdr_clen);
       elen    -=  clen;

       
       /*
        | If the contributor's length is precisely 0 or not a multiple of
        | an LCB packet, something's wrong
       */
       if ( (clen == 0) || (clen & LCB_M_PACKET_ALIGN) ) return -1;

       
       /* Extract the contributor's identification and form its mask */
       cid    = LCB_EXTRACT_CID (hdr_clen);
       cidmsk = (unsigned int)0x80000000 >> cid;

       
       /* Count and make a mask of the contributors seen so far */
       ncids += 1;
       cids  |= cidmsk;

       
       /*
        | Advance the contributor's pointer past the LCB header,
        | Advance the event pointer to the next contribution.
       */
       ptr  = (const unsigned int *)_ADVANCE (evt, sizeof (LCB_header));
       evt  = (const unsigned int *)_ADVANCE (evt, clen);
       
       
       /* Locate and fill in the relevant contributor record     */
       contributor      = contributors + cid;
       contributor->cid = cid;
       contributor->len = clen - sizeof (LCB_header);
       contributor->ptr = ptr;

       
       /* Is this one of the tower contributors */
       if (_EBF_CID_IS_TOWER (cid))
       {
           const unsigned int *tkr;
           int              calcnt;

           /*
            |  The first word of a tower contributor is the number of
            |  CAL logs struck in each of the 8 layers, one count
            |  per nibble.
           */
           calcnt = ptr[0];


           /*  If there are any logs struck in this tower... */
           if (calcnt)
           {
               /*
                | ... mark up this tower as having a contributed a CAL rec
                | ... count the total number of struck logs in this tower
               */
               ctids  |= cidmsk;
               calcnt  = calLogCnt (calcnt, mask);
           }

           
           /* Must include the length of the count word itself */
           calcnt             += 1;
           contributor->calcnt = calcnt;


           /*
            | Locate the TKR record. Note that 'ptr' is a 32 bit pointer
            | and that the 'calcnt' word and each struck log contributes
            | 1 32 bit word. Therefore, 'ptr' is advanced by the number of
            | struck logs + the CAL count word to get to the TKR record.
           */
           tkr = ptr + calcnt;

           
           /*
            | See if this is an empty TKR contribution. The right way
            | to do this is to look at the 72 layer accept bits. There
            | is an optimization one can do which avoids looking at all
            | 3 words containing the 72 bits. This optimization is based
            | on the fact that only rarely is a tower completely devoid
            | of tracker hits.
            |
            | There are only 8 layer accept bits in the last accept word.
            | If there are no hits in this tower, then this word must be
            | zero. However, it is only necessary that it is 0 for the
            | track record to be empty; it is not sufficient. It may be
            | that there are hits in other layers and that strip address(es)
            | and TOT are 0. Therefore, in the case where this word is 0,
            | the other 2 layer accept words must be checked to ensure that
            | the tracker record really is empty. This is a good
            | optimization, since layer accept word 2 will rarely be 0, so
            | these final checks are avoided in almost all cases.
           */
           ctids |= cidmsk >> 16;     /* Assume it is hit */
           if (tkr[2] == 0)
           {
               /* If any accepts bits in the first 2 hit, bad assumption */
               if ( (tkr[0] | tkr[1]) == 0) ctids &= ~(cidmsk >> 16);
           }
       }
   }

   
   /* Fill in the summary information */
   dir->ncids = ncids;
   dir->cids  =  cids;
   dir->ctids = ctids;

   
   /*
    | Return the number of contributors encountered. If have an error
    | negate this number as an indication that there was an error.
    | Currently the only error checked for is that the total LCB event
    | length exactly match the sum of the LCB packet lengths of the
    | contributors.
   */
   return elen == 0 ? ncids : -ncids;
}
/* --------------------------------------------------------------------- */



