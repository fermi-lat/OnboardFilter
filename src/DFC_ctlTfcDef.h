#ifndef DFC_CTLTFCDEF_H
#define DFC_CTLTFCDEF_H


/* ---------------------------------------------------------------------- *//*!
   
   \file   DFC_ctlTfcDef.h
   \brief  Defines the structure containing the Tkr control parameters
   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $Id
\endverbatim
                                                                         */
/* --------------------------------------------------------------------- */





#ifdef __cplusplus
extern "C" {
#endif

struct _TFC_geometry;

    
/* --------------------------------------------------------------------- *//*!

  \struct _DFC_ctlTfc
  \brief   Contains the Tracker filtering control parameters
                                                                         *//*!
  \typedef DFC_ctlTfc
  \brief   Typedef for struct _DFC_ctlTfc
                                                                         */
/* --------------------------------------------------------------------- */
typedef struct _DFC_ctlTfc
{
   const struct _TFC_geometry *geo; /*!< Pointer to the tracker geometry */
}
DFC_ctlTfc;



#ifdef __cplusplus
}
#endif

#endif


