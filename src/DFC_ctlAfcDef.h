#ifndef DFC_CTLAFCDEF_H
#define DFC_CTLAFCDEF_H



/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/





#ifdef __cplusplus
extern "C" {
#endif
    

/* ---------------------------------------------------------------------- *//*!
   
   \file   DFC_ctlAfcDef.h
   \brief  Defines the structure containing the AFC control parameters.
   \author JJRussell - russell@slac.stanford.edu

   The AFC is the ACD only  Fast filter code. This code uses only the
   ACD tile masks to implement the filtering. Contrast this with the
   ATC code which uses both the ACD struck tiles and the TKR layer
   hit masks.
                                                                         */
/* --------------------------------------------------------------------- */

#include "AFC_splashMap.h"

    
/* --------------------------------------------------------------------- *//*!

  \struct _DFC_ctlAfc
  \brief   Contains the Afc filtering control parameters
                                                                         *//*!
  \typedef DFC_ctlAfc
  \brief   Typedef for struct _DFC_ctlAfc
                                                                         */
/* --------------------------------------------------------------------- */
typedef struct _DFC_ctlAfc
{
    struct _AFC_splashMap splashMap; /*!< The splash map to use, this
                                          defines the near and far
                                          neighbors                      */
}
DFC_ctlAfc;


#ifdef __cplusplus
}
#endif

#endif
