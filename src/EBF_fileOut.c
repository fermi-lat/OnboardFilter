/* ---------------------------------------------------------------------- *//*!
   
   \file  EBF_fileOut.c
   \brief EBF File Output Utilities
   \author JJRussell - russell@slac.stanford.edu

    Writes an Event Builder File


\verbatim
    CVS $Id

\endverbatim    
                                                                         */
/* --------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include "DFC/DFC_endianness.h"
#include "DFC/EBF_fileOut.h"

#include "windowsCompat.h"

struct _EBF_ofile
{
    FILE *fd;              /*!< The output file descriptor */
    unsigned int buffer[0x10000 / sizeof (unsigned int)];
                           /*!< Provide 64K output buffer  */
};


    


/* --------------------------------------------------------------------- *\

    Files read on little endian machines, must be swapped around a bit.
    There is no fixup needed on big endian machines.
        
\* --------------------------------------------------------------------- */
#if DFC_C_ENDIANNESS == DFC_K_ENDIANNESS_LITTLE

#include "EBF_swap.h"
#include "DFC/LCB.h"
#include "DFC/EBF_cid.h"




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




/* --------------------------------------------------------------------- */
static inline void halfswap  (unsigned short int *beg, int cnt);

static inline int fixup_acd  (unsigned int *evt,
                              int           elen,
                              int           ievt);

static const unsigned int *fixup     (       unsigned int *output,
                                      const  unsigned int *evt,
                                                      int  elen);
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \fn         int fixup (      unsigned int *output,
                         const unsigned int *evt,
                         int                 elen)
  \brief      Fixups up the data for little endian machines
  \param evt  Pointer to the data
  \param elen Size of the data, in bytes
  \return     Address of the output buffer
                                                                         */
/* --------------------------------------------------------------------- */
static const unsigned int *fixup (unsigned int    *output,
                                  const unsigned int *evt,
                                  int               esize)
{
   int status;

   /* First copy the event */
   memcpy (output, evt, esize);

   
   /* Then fixup the acd contribution */
   status = fixup_acd (output   + 1,
                       esize - sizeof (*evt),
                       0);
   if (status) return NULL;

   /* Finally swap all the 32 bit words */ 
   EBF_swap32_lclXbigN (output, esize / sizeof (int));
       
   
   return output;
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \fn         int fixup_acd (unsigned int *evt, int elen)
  \brief      Fixups up the ACD data for little endian machines
  \param evt  Pointer to the ACD data
  \param elen Size of the ACD data, in bytes
  \return     Status
                                                                         */
/* --------------------------------------------------------------------- */
static int fixup_acd (unsigned int *evt, int elen, int ievt)
{
   while (elen > 0)
   {
       unsigned int hdr_clen;
       unsigned int     clen;
       unsigned int      cid;
       

       /*
        |  Get the length of the event in bytes and reduce the number of
        |  unprocessed bytes
       */
       hdr_clen = ((LCB_header *)evt)->hdr_clen; 
       clen     =  LCB_EXTRACT_CLEN (hdr_clen);
       elen    -=  clen;

       
       /*
        | If the contributor's length is precisely 0 or not a multiple of
        | an LCB packet, something's wrong
       */
       if ( (clen == 0) || (clen & LCB_M_PACKET_ALIGN) )
       {
           printf (" Bad contributor length at event = %d\n"
                   "                          length = %d\n"
                   "     Length cannot be 0 and must be an event number of\n"
                   "     LCB packets packets (16 bytes)",
                   ievt,
                   clen);
           return -1;
       }

       
       /* Check if this is the ACD */
       cid = LCB_EXTRACT_CID (hdr_clen);
       if (cid == EBF_K_CID_ACD)
       {
           unsigned short int *ptr;
           
           /* Skip over the LCB header, decrease contribution length by same */
           ptr   = (unsigned short int *)_ADVANCE (evt, sizeof (LCB_header));
           clen -= sizeof (LCB_header);
           
           halfswap (ptr, clen / sizeof (int));
           
           return 0;
       }
   }

   
   /* No ACD contribution, weird, but okay */
   return 0;
       
}
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*

  \fn  unsigned int *halfswap (unsigned short int *beg, int cnt)
  \brief     Swaps all the 16 bit words in the specified range.
  \param ptr The starting point to begin the swap
  \param cnt The number of longwords to swap
                                                                         */
/* --------------------------------------------------------------------- */
static void halfswap (unsigned short int *ptr, int len) 
{
    while (--len >= 0)
    {
        unsigned short int tmp0 = ptr[0];
        unsigned short int tmp1 = ptr[1];
        
        ptr[0] = tmp1;
        ptr[1] = tmp0;
        
        ptr += 2;
    }
    
    return;
}
/* --------------------------------------------------------------------- */


#else
#define fixup(output, evt, elen) evt
#endif
/* --------------------------------------------------------------------- */






/* --------------------------------------------------------------------- *//*!

  \fn      EBF_ofile *EBF_create (const char *name)
  \brief   Creates a file to receive the EBF data

  \param  name  The name of the file to create
  \return       A handle to write the file
                                                                         */
/* --------------------------------------------------------------------- */  
EBF_ofile *EBF_create (const char *name)
{
   EBF_ofile *ebo;
   FILE       *fd;
   
   fd = fopen (name, "w");
   if (fd <= 0) return NULL;

   ebo = (EBF_ofile *)malloc (sizeof (*ebo));
   ebo->fd = fd;
   return ebo;
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \fn      int EBF_write (EBF_ofile      *ebo,
                          const void     *evt,
                          unsigned int nbytes)
  \brief   Writes the specified data to the output file
  
  \param   ebo  The output file handle
  \param   evt  Pointer to the event to write
  \param nbytes The number of bytes to write
  \return       Status

   It is assumed that the event is in the local machine endianness. If
   the endianness is not BIG ENDIAN, the event will be byte-swapped to
   be 32-bit big-endian.
                                                                         */
/* --------------------------------------------------------------------- */  
int EBF_write (EBF_ofile *ebo, const void *evt, unsigned int elen)
{
    const unsigned int *output;
 
    output = fixup (ebo->buffer, evt, elen);
    return fwrite (output, elen, 1, ebo->fd);
}
/* --------------------------------------------------------------------- */


/* --------------------------------------------------------------------- *//*!

  \fn    int EBF_oclose (EBF_ofile *ebo)
  \brief Closes a previously open output EBF file
  \param ebo  The output file handle.
                                                                         */
/* --------------------------------------------------------------------- */  
int EBF_oclose (EBF_ofile *ebo)
{
   return fclose (ebo->fd);
}
/* --------------------------------------------------------------------- */
