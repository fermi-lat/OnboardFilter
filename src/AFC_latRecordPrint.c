/* ---------------------------------------------------------------------- *//*!
   
   \file   AFC_latRecordPrint.c
   \brief  Provides an ASCII print out of the unpacked ACD data and
           also its supporting data structures
   \author JJRussell - russell@slac.stanford.edu
   
   \code
    $ squeeze  in.ebf out.ebf 4
   \endcode

   
\verbatim
    CVS $Id: AFC_latRecordPrint.c,v 1.2 2004/07/12 17:37:41 golpa Exp $
    
\endverbatim
                                                                         */
/* --------------------------------------------------------------------- */


#include <stdio.h>
#include "ffs.h"
#include "DFC/AFC_latRecord.h"
#include "DFC/AFC_latRecordPrint.h"
#include "AFC_remapDef.h"


/* --------------------------------------------------------------------- *//*!

  \struct AcdLists
  \brief  Internal only structure to hold the bit maps of the ACD values.
									 */
/* --------------------------------------------------------------------- */
struct AcdLists
{
    unsigned int m[EBF_K_ACD_SIDE_CNT][EBF_K_ACD_LIST_CNT];
  /*!< The ACD bit maps by side (A/B) and plane (X-,X+,Y-,Y+,TOP)        */
};
/* --------------------------------------------------------------------- */



static unsigned int classify (int          *chn,
                              unsigned int *_a0,
                              unsigned int *_a1);


static void printPhas   (const unsigned  short *phas,
                         const struct AcdLists *accepts);


/* --------------------------------------------------------------------- *//*!

  \fn     void AFC_latRecordPrint (const struct _AFC_latRecord *acd)
  \brief  Prints an ASCII display of the unpacked ACD record
  
  \param  acd   Pointer to the unpacked ACD data to display
                                                                         */
/* --------------------------------------------------------------------- */  
void AFC_latRecordPrint (const struct _AFC_latRecord *acd)
{
  printf (" ACD Record\n"
          " ----------\n"
          " Missing Start Bit:  %4.4x    Header Parity Errors : %4.4x\n"
          " PHA Unmatched    :  %4.4x    PHA    Parity Summary: %4.4x\n"
          "    ACD   hits    :  %8.8x %8.8x %8.8x %8.8x\n"
          "                     %8.8x %8.8x %8.8x %8.8x\n"
          "    PAccepts      :  %8.8x %8.8x %8.8x %8.8x\n"
          "                     %8.8x %8.8x %8.8x %8.8x\n"
          "    NAccepts      :  %8.8x %8.8x %8.8x %8.8x\n"
          "                     %8.8x %8.8x %8.8x %8.8x\n"
          " Number of PHAs   :  %8d\n",
          acd->start_parity     >> 16, acd->start_parity     & 0xffff,
          acd->unmatched_parity >> 16, acd->unmatched_parity & 0xffff,
          
          acd->hits[0][0],     acd->hits[0][1],
          acd->hits[0][2],     acd->hits[0][3],
          acd->hits[1][0],     acd->hits[1][1],
          acd->hits[1][2],     acd->hits[1][3],
          
          acd->paccepts[0][0], acd->paccepts[0][1],
          acd->paccepts[0][2], acd->paccepts[0][3],
          acd->paccepts[1][0], acd->paccepts[1][1],
          acd->paccepts[1][2], acd->paccepts[1][3],
          
          acd->naccepts[0][0], acd->naccepts[0][1],
          acd->naccepts[0][2], acd->naccepts[0][3],
          acd->naccepts[1][0], acd->naccepts[1][1],
          acd->naccepts[1][2], acd->naccepts[1][3],
          acd->nphas);
  
  printPhas (acd->phas, (struct AcdLists *)acd->paccepts);
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \fn     static void printPhas (const unsigned  short *phas,
                                 const struct AcdLists *accepts)
  \brief  Internal routine to display the ACD PHA values
  \param  phas     The list of PHA values
  \param  accepts  The lists of PHA accept channels
                                                                         */
/* --------------------------------------------------------------------- */
static void printPhas (const unsigned  short *phas,
                       const struct AcdLists *accepts)  
{
   static const char *Format[4] = {"               ",
                                   " %d%d%d %4.4x ",
                                   " %d%d%d      %4.4x ",
                                   " %d%d%d %4.4x %4.4x " };
                                    
   const char **format = Format;
   struct AcdLists   a = *accepts;
 

   printf ("\n"
           "  XZ PhaA PhaB   YZ PhaA PhaB   XY PhaA PhaB   RU PhaA PhaB\n" 
           " --- ---- ----  --- ---- ----  --- ---- ----  --- ---- ----\n");

   while(1)
   {
       int       list;
       unsigned int m;
       int    chns[4];
      

       /* For a bit mask of the active channels */
       for (m = list = 0; list < EBF_K_ACD_LIST_CNT; list++)
       {
           m |= classify (chns +  list,
                          &a.m[0][list],
                          &a.m[1][list]) << (2 * list);
       }

      
       /* If no channels active, that's it */
       if (m == 0) break;
      

       /* Display the PHAs */
       for (list = 0; list < EBF_K_ACD_LIST_CNT; list++)
       {
           int code = m & 0x3;
           if (code == 0) printf (format[0]);
           else
           {
               static const int Face[8] = { 1,3, 2,4, 0,0, 6,5 };

               int gchn = chns[list] + 32 * list;
               int lchn = gchn % 16;
               int  row = lchn / 5;
               int  col = lchn % 5;
               
               printf (format[code],
                       Face[gchn / 16], row, col,
                       phas[(m & 1) ? gchn : gchn + 128],
                       phas[gchn + 128]);
           }
           m >>= 2;
       }
       puts ("");
   }


   return;
}
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \fn     unsigned int classify (int          *chn,
                                 unsigned int *_a0,
                                 unsigned int *_a1)
  \brief  Classifies the next specified PHA value as being either a
          member of the A list, the B list or both.
  \param  chn   Returned as the channel number of the next PHA value
  \param _a0    Pointer to the current list of "A" accepts
  \param _a1    Pointer to the current list of "B" accepts
  \return       A mask indicating whether the next active PHA value
                is a member of only the "A" list (1), only the "B"
                list (2) or both (3)
                                                                         */
/* --------------------------------------------------------------------- */  
static unsigned int classify (int *chn, unsigned int *_a0, unsigned int *_a1)
{
   unsigned int a0 = *_a0;
   unsigned int a1 = *_a1;
   unsigned int  m =    0;
   int           c;
   
 
   /* If a0 is active */
   if (a0)
   {
       int c0;
       int c1;
       
       c0 = FFS (a0);
       if (a1)
       {
           /* Both are active */
           c1 = FFS (a1);

           /* If they the same channels or c0 is smaller... */
           if (c0 <= c1)
           {
               c  = c0; 
               m |= 1;
               if (c0 == c1) m |= 0x2;
           }

           /* C1 is smaller, display it */
           else
           {
               c  = c1;
               m |= 0x2;
           }
       }
       /* Only a0 is active */
       else
       {
           c  = c0;
           m |= 0x1;
       }
   }
   else 
   {
       /* Only a1 is active, it must be active since a0 wasn't */
       c  = FFS (a1);
       m |= 0x2;
   }

   if (m & 1) *_a0 = FFS_eliminate (a0, c);
   if (m & 2) *_a1 = FFS_eliminate (a1, c);
   *chn = 31 - c;
       
   return m;
   
}


       
static void printBarray (const unsigned char *title,
                         const unsigned char *barray,
                         int                  cnt);
static void printDecode (const unsigned char *pha,
                         int                  cnt);



/* --------------------------------------------------------------------- *//*!

  \fn     void AFC_remapPrint (const struct _AFC_remap *remap)
  \brief  Prints a display of the ACD remapping structure
  \param remap  Pointer to the ACD remapping structure
                                                                         */
/* --------------------------------------------------------------------- */
void AFC_remapPrint (const struct _AFC_remap *remap)
{
   int ibrd;
   const AFC_remapBrd *brd = remap->brds;
   
    for (ibrd = 0;  ibrd < EBF_K_ACD_NBRDS; ibrd++, brd++)  
   {
       printf (
           "\n"
           " Board %2d Masks  XZ: %8.8x   YZ: %8.8x  XY: %8.8x  RU: %8.8x\n",
           ibrd,
           brd->msks[EBF_K_ACD_LIST_XZ], brd->msks[EBF_K_ACD_LIST_YZ],
           brd->msks[EBF_K_ACD_LIST_XY], brd->msks[EBF_K_ACD_LIST_RU]);

       printBarray (" Chn: ", brd->rma, EBF_K_ACD_NCHNS_PER_BRD);
       printBarray (" Pha: ", brd->rmc, EBF_K_ACD_NCHNS_PER_BRD);
       printDecode (brd->rmc, EBF_K_ACD_NCHNS_PER_BRD);
   }

    return;
    
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

   \fn    void printBarray (const unsigned char *title,
                            const unsigned char *barray,
                            int                  cnt)
   \brief Internal utility to display an array of bytes
   \param title    The title of the array of bytes
   \param barray   The array of bytes to display
   \param cnt      The number of elements in \e barray
                                                                         */
/* --------------------------------------------------------------------- */    
static void printBarray (const unsigned char *title,
                         const unsigned char *barray,
                         int                  cnt)
{
   int idx;
   printf (title);
       
   for (idx = 0; idx < cnt; idx++) printf (" %2.2x", barray[idx]);
   printf ("\n"); 
   
   return;
}
/* --------------------------------------------------------------------- */    




/* --------------------------------------------------------------------- *//*!

   \fn    void printDecode (const unsigned char *pha,
                            int                  cnt)
   \brief Internal utility to display an PHA decode array.
   \param pha   The PHA decode array
   \param cnt   Number of elements in the pha decode array
                                                                         */
/* --------------------------------------------------------------------- */
static void printDecode (const unsigned char *pha,
                         int                  cnt)
{
   int idx;
   printf (" Lst: ");
       
   for (idx = 0; idx < cnt; idx++)
   {
       printf (" %2.2x", (pha[idx] / 32) & 0x3);
   }
   printf ("\n Chn: ");

   for (idx = 0; idx < cnt; idx++)
   {
       printf (" %2.2x", pha[idx] & 0x1f);
   }
   
   printf ("\n");
   
   return;
}
/* --------------------------------------------------------------------- */
