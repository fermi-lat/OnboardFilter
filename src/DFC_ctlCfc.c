/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  DFC_ctlCfc.c
   \brief Routines to size and initialize the CAL filtering control
          parameters
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/DFC_ctl.h"
#include "DFC_ctlCfcDef.h"


/* ---------------------------------------------------------------------- *//*!

  \fn DFC_ctlCfcSizeof (void)
  \brief  Returns the size, in bytes, of the CAL filtering control
          parameters
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_ctlCfcSizeof (void)
{
   return sizeof (struct _DFC_ctlCfc);
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!
   
  \fn int DFC_ctlCfcInit (struct _DFC_ctlCfc *cfc)
  \brief      Initializes the CAL filtering control parameters
  \param  cfc Pointer to the CAL filtering parameters to initialize
  \return     Status
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_ctlCfcInit (struct _DFC_ctlCfc *cfc)
{
   CFC_constantsLatInit ((struct _CFC_logConstants *)cfc->constants);
   return 0;
}
