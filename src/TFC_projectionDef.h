#ifndef TFC_PROJECTIONDEF_H
#define TFC_PROJECTIONDEF_H



/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   TFC_projectionDef.h
   \brief  Defines the data structure describing a track projection and
           a collection of track projections.
   \author JJRussell - russell@slac.stanford.edu

                                                                         */
/* --------------------------------------------------------------------- */


#include "DFC/TFC_stripDef.h"

#ifdef __cplusplus
extern "C" {
#endif    




/* --------------------------------------------------------------------- *//*!

  \struct _TFC_projection
  \brief   The data structure describing a track projection
                                                                         *//*!
  \typedef TFC_projection
  \brief   Typedef for struct _TFC_projection.
                                                                         */
/* --------------------------------------------------------------------- */  
typedef struct _TFC_projection
{
  int           intercept; /*!< Intercept at the beginning layer         */
  int               slope; /*!< Slope                                    */
  int          acdTopMask; /*!< ACD top tile candidates                  */
  int            acdXMask; /*!< ACD x facing candidates                  */
  int            acdYMask; /*!< ACD y facing candidates                  */
  unsigned char skirtMask; /*!< Mask of which skirt region prj strikes   */
  unsigned char       min; /*!< Beginning layer number of the projection */
  unsigned char       max; /*!< Ending    layer number of the projection */
  unsigned char     nhits; /*!< Number of hits assigned                  */
  unsigned         layers; /*!< Bit mask representing the struck layers  */
  TFC_strip      hits[18]; /*!< Hits assigned to proj                    */
}
TFC_projection;
/* --------------------------------------------------------------------- */  


/* --------------------------------------------------------------------- *//*!

  \struct  _TFC_projectionDir
  \brief    Directory structure to locate the projections
									 *//*!
  \typedef  TFC_projectionDir
  \brief    Typedef for struct \e TFC_projectionDir
									 */
/* --------------------------------------------------------------------- */
typedef struct _TFC_projectionDir
{
  unsigned short int  idx; /*!< Index to the first projection            */
  unsigned char      xCnt; /*!< Count of the X projections               */
  unsigned char      yCnt; /*!< Count of the Y projections               */
}
TFC_projectionDir;
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \struct _TFC_projections
  \brief   The data structure describing a collection of track projections
                                                                         *//*!
  \typedef TFC_projections
  \brief   Typedef for struct _TFC_projections.
                                                                         */
  
/* --------------------------------------------------------------------- */  
typedef struct _TFC_projections
{
  unsigned short int    maxCnt; /*!< Maximum # of projections available  */
  unsigned short int    curCnt; /*!< Current # of projections in use     */
  unsigned short int    twrMsk; /*!< Mask of the tower with projections  */
  TFC_projectionDir    dir[16]; /*!< Directory of the projections by twr */
  TFC_projection    prjs[1000]; /*!< List of projections                 */
}
TFC_projections;
/* --------------------------------------------------------------------- */  
    


#ifdef __cplusplus
}
#endif    



#endif
