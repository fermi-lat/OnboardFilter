/* ---------------------------------------------------------------------- *//*!
   
   \file  DFC_evt.c
   \brief Implementation to get the size the data structure needed to hold
          an unpacked LAT event and its processing storage.
   \author JJRussell - russell@slac.stanford.edu

\verbatim
   CVS $id
\endverbatim
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/DFC_evt.h"
#include "DFC/DFC_latUnpack.h"



/* ---------------------------------------------------------------------- *//*!

  \fn      void DFC_evtInit (struct _DFC_evt *dfcEvt)
  \brief   One time initialization of a DFC_evt structure
  \return  Status

  \param   dfcEvt the DFC_evt to initialize
                                                                          */  
/* ---------------------------------------------------------------------- */
void DFC_evtInit (struct _DFC_evt *dfcEvt)
{
   DFC_latRecordInit (&dfcEvt->dlr);
   return;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn      int DFC_evtSizeof (void)
  \brief   Returns, the size, in bytes of a DFC_evt structure.
  \return  The size, in bytes, of a DFC_evt structure.
                                                                          */  
/* ---------------------------------------------------------------------- */
int DFC_evtSizeof (void)
{
   return (sizeof (struct _DFC_evt));
}
/* ---------------------------------------------------------------------- */



