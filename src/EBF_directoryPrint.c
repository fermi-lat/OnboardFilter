/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/




/* ---------------------------------------------------------------------- *//*!
   
   \file  EBF_directoryPrint.c
   \brief EBF Print Utilities
   \author JJRussell - russell@slac.stanford.edu

                                                                         */
/* --------------------------------------------------------------------- */

#include <stdio.h>
#include "DFC/EBF_directoryPrint.h"
#include "DFC/EBF_directory.h"
#include "DFC/EBF_cid.h"


#ifdef __cplusplus
extern "C" {
#endif

void printData (const char *name, int cid, const unsigned int *dat, int cnt);
    
//static int printGroup (const struct directory *dir, int max);

#ifdef __cplusplus
}
#endif    



/* --------------------------------------------------------------------- *//*!

  \fn  void EBF_dirPrint (const struct _EBF_directory *dir)
  \brief     Provides a formatted print out of the contributors in dir
  \param dir The directory for this event

                                                                         */
/* --------------------------------------------------------------------- */
void EBF_dirPrint (const struct _EBF_directory *dir)
{
   const unsigned int            *evt;
   int                           cids;
   int                         nbytes;
   const EBF_contributor *contributor;
   
   evt    = dir->contributors[EBF_K_CID_EVT].ptr;
   nbytes = dir->contributors[EBF_K_CID_EVT].len;
   printf ("Event Sequence: 0x%8.8x Event Size: %d bytes\n",
           evt[0],
           nbytes);

   cids        = dir->cids;
   contributor = dir->contributors;

   
   /* Now print each event contribution */
   do
   {
       enum _EBF_K_cid cid;

       if (cids > 0) continue;
       cid  = (enum _EBF_K_cid)contributor->cid;

       if      (cid <  EBF_K_CID_TWR + 16) EBF_twrPrint (contributor);
       else if (cid == EBF_K_CID_ACD     ) EBF_acdPrint (contributor);
   }
   while (contributor++, cids <<= 1);
   
   return;
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \fn        void EBF_acdPrint (const struct _EBF_contributor *acd)

  \brief     Provides a formatted print out of the contents of the
             ACD contribution.
  \param acd Pointer to the ACD contributor record.
                                                                         */
/* --------------------------------------------------------------------- */
void EBF_acdPrint (const struct _EBF_contributor *acd)
{
   printData ("ACD", acd->cid, acd->ptr, acd->len / sizeof (unsigned int));
   return;
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \fn        void EBF_calPrint (const struct _EBF_contributor *twr)
  \brief     Provides a formatted print out of the contents of a CAL tower
             contribution.
  \param twr Pointer to the TWR contributor record containing the
             CAL record to be printed
                                                                         */
/* --------------------------------------------------------------------- */
void EBF_calPrint (const struct _EBF_contributor *twr)
{
   int              calcnt;
   const unsigned int *cal;

   
   calcnt = twr->calcnt;
   cal    = EBF__calLocate (twr->ptr);
   if (calcnt > 1) printData ("CAL", twr->cid, cal, calcnt);

   return;
}
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!

  \fn        void EBF_tkrPrint (const struct _EBF_contributor *twr)
  \brief     Provides a formatted print out of the contents of a TKR tower
             contribution.
  \param twr Pointer to the TWR contributor record containing the TKR
             data to be printed

                                                                         */
/* --------------------------------------------------------------------- */
void EBF_tkrPrint (const struct _EBF_contributor *twr)
{
   int                 cnt;
   int              calcnt;
   const unsigned int *tkr;
   
   cnt    = twr->len / sizeof (int);
   calcnt = twr->calcnt;
   tkr    = (unsigned int *)EBF__tkrLocate (twr->ptr, calcnt);

   cnt   -= calcnt;
   printData ("TKR", twr->cid, tkr, cnt);

   return;
}
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!

  \fn        void EBF_trgPrint (const struct _EBF_contributor *trg)
  \brief     Provides a formatted print out of the contents of a trigger
             contribution
  \param trg A pointer to the trigger contributor record.

                                                                         */
/* --------------------------------------------------------------------- */
void EBF_trgPrint (const struct _EBF_contributor *trg)
{
   return;
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \fn        void EBF_twrPrint (const struct _EBF_contributor *twr)
  \brief     Provides a formatted print out of the contents of a tower
             contribution
  \param twr Pointer to the tower contributor data to be printed

                                                                         */
/* --------------------------------------------------------------------- */
void EBF_twrPrint (const struct _EBF_contributor *twr)
{
   printf ("TWR %1.1x %8.8x\n", twr->cid, twr->ptr[0]);
   EBF_calPrint  (twr);
   EBF_tkrPrint  (twr);

   return;
}
/* --------------------------------------------------------------------- */


/* --------------------------------------------------------------------- *//*!

  \fn  void printData (const char        *name,
                       int                 cid,
                       const unsigned int *dat,
                       int                 cnt)
  \brief Simple print routine
  \param name The contributor's name
  \param  cid The contributor's id
  \param  dat The contributor's data
  \param  cnt The length of the data section in words
                                                                         */
/* --------------------------------------------------------------------- */
void printData (const char *name, int cid, const unsigned int *dat, int cnt)
{
   int    idx;
   int      n;
   int margin;

   margin = n = printf ("%s  ", name, cid);             
   for (idx = 0; idx < cnt; idx++)
   {
       if (n > 72) n = printf ("\n%*c", margin, ' ');
       n += printf (" %8.8x", dat[idx]);
   }

   printf ("\n");
   
   return;
   
}
/* --------------------------------------------------------------------- */






