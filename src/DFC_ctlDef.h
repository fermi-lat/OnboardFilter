#ifndef DFC_CTLDEF_H
#define DFC_CTLDEF_H

 /*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  DFC_ctlDef.h
   \brief Contains the control parameters for the filtering process
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/DFC_ctlSysDef.h"
#include "DFC_ctlAfcDef.h"
#include "DFC_ctlAtfDef.h"
#include "DFC_ctlCfcDef.h"
#include "DFC_ctlTfcDef.h"


#ifdef __cplusplus
extern "C"{
#endif


/* ---------------------------------------------------------------------- *//*!

  \struct _DFC_ctl
  \brief   Control structure for the filtering process
                                                                          *//*!
  \typedef DFC_ctl
  \brief   Typedef for struct _DFC_ctl
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _DFC_ctl
{
    DFC_ctlSys  sys;  /*!< SYS contribution to the control parameters    */
    DFC_ctlAfc  afc;  /*!< AFC contribution to the control parameters    */    
    DFC_ctlAtf  atf;  /*!< ATF contribution to the control parameters    */    
    DFC_ctlCfc  cfc;  /*!< CAL contribution to the control parameters    */
    DFC_ctlTfc  tfc;  /*!< TKR contribution to the control parameters    */
}
DFC_ctl;
/* ---------------------------------------------------------------------- */

 

#ifdef __cplusplus
}
#endif


#endif




