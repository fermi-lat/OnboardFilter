#ifndef DFC_CTLATFDEF_H
#define DFC_CTLATFDEF_H



/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



#include "ATF_shadowedTowers.h"


#ifdef __cplusplus
extern "C" {
#endif
    

/* ---------------------------------------------------------------------- *//*!
   
   \file   DFC_ctlAtfDef.h
   \brief  Defines the structure containing the ATF control parameters
   \author JJRussell - russell@slac.stanford.edu

   The ATF is the Acd/Tracker Fast filter code. This code uses only layer
   level bit masks to implement the filtering.
                                                                         */
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \struct _DFC_ctlAtf
  \brief   Contains the Atf filtering control parameters
                                                                         *//*!
  \typedef DFC_ctlAtf
  \brief   Typedef for struct _DFC_ctlAtf
                                                                         */
/* --------------------------------------------------------------------- */
typedef struct _DFC_ctlAtf
{
    struct _ATF_shadowedTowersCtl shadowed;
                                        /*!< The shadowed towers by tile */
}
DFC_ctlAtf;


#ifdef __cplusplus
}
#endif

#endif


