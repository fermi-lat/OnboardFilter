#ifndef DFC_CTLCFCDEF_H
#define DFC_CTLCFCDEF_H



/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



#include "DFC/CFC_constants.h"


#ifdef __cplusplus
extern "C" {
#endif


/* ---------------------------------------------------------------------- *//*!
   
   \file   DFC_ctlCfcDef.h
   \brief  Defines the structure containing the CAL control parameters
   \author JJRussell - russell@slac.stanford.edu

                                                                         */
/* --------------------------------------------------------------------- */


/* --------------------------------------------------------------------- *//*!

  \struct _DFC_ctlCfc
  \brief   Contains the CAL filtering control parameters
                                                                         *//*!
  \typedef DFC_ctlCfc
  \brief   Typedef for struct _DFC_ctlCfc
                                                                         */
/* --------------------------------------------------------------------- */
typedef struct _DFC_ctlCfc
{
    const struct _CFC_logConstants constants[CFC_K_LOGS_PER_LAT];
    /*!< The calibration constants for the entire CAL */
}
DFC_ctlCfc;


#ifdef __cplusplus
}
#endif

#endif


