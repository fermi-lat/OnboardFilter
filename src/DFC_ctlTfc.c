/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  DFC_ctlTfc.c
   \brief Routines to size and initialize the TKR filtering control
          parameters
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/DFC_ctl.h"
#include "DFC_ctlTfcDef.h"


/* ---------------------------------------------------------------------- *//*!

  \fn DFC_ctlTfcSizeof (void)
  \brief  Returns the size, in bytes, of the TKR filtering control
          parameters
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_ctlTfcSizeof (void)
{
   return sizeof (DFC_ctlTfc);
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!
   
  \fn int DFC_ctlTfcInit (struct _DFC_ctlTfc *tfc)
  \brief      Initializes the TKR filtering control parameters
  \param  tfc Pointer to the TKR filtering parameters to initialize
  \return     Status
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_ctlTfcInit (struct _DFC_ctlTfc *tfc)
{
   return 0;
}
/* ---------------------------------------------------------------------- */











