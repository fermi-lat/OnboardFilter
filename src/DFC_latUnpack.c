/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  DFC_latUnpack.c
   \brief Implementation to get the size the data structure needed to hold
          an unpacked LAT event
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/EBF_glt.h"
#include "DFC/DFC_latRecord.h"
#include "DFC/DFC_latUnpack.h"
#include "DFC/CFC_latUnpack.h"
#include "DFC/TFC_latUnpack.h"
#include "DFC_ctlDef.h"


/* ---------------------------------------------------------------------- *//*!

  \fn      void DFC_latRecordInit (struct _DFC_latRecord *dlr)
  \brief   Returns
  \return  The size, in bytes, of an unpacked LAT event
                                                                          */  
/* ---------------------------------------------------------------------- */
void DFC_latRecordInit (struct _DFC_latRecord *dlr)
{
   TFC_latUnpackInit (&dlr->tkr);
   return;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn      int DFC_latRecordSizeof (void)
  \brief   Returns
  \return  The size, in bytes, of an unpacked LAT event
                                                                          */  
/* ---------------------------------------------------------------------- */
int DFC_latRecordSizeof (void)
{
   return (sizeof (DFC_latRecord));
}
/* ---------------------------------------------------------------------- */




int DFC_latRecordUnpack (struct _DFC_latRecord  *dlr,
                         const unsigned int     *evt,
                         const struct _DFC_ctl  *ctl)
{
   EBF_directoryCompose (&dlr->dir, evt);
   CFC_latUnpack        (&dlr->cal, &dlr->dir, ctl->cfc.constants);
   TFC_latUnpackInit    (&dlr->tkr);
   TFC_latUnpack        (&dlr->tkr, &dlr->dir, -1);

   return 0;
}
