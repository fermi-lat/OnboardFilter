/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  DFC_ctlTfc.c
   \brief Routines to size and initialize the TKR filtering control
          parameters
   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $id
\endverbatim
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/DFC_ctl.h"
#include "DFC_ctlTfcDef.h"


struct _TFC_geometry;


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
   
  \fn int DFC_ctlTfcInit (      struct _DFC_ctlTfc   *tfc,
                          const struct _TFC_geometry *geo)
  \brief      Initializes the TKR filtering control parameters
  \param  tfc Pointer to the TKR filtering parameters to initialize
  \param  geo The TKR geometry. Only the pointer is saved, so
              the backing memory must remain valid.
  \return     Status
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_ctlTfcInit (      struct _DFC_ctlTfc   *tfc,
		    const struct _TFC_geometry *geo)
{
   tfc->geo = geo;
   return 0;
}
/* ---------------------------------------------------------------------- */











