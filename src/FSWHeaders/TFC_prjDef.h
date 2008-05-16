#ifndef TFC_PRJDEF_H
#define TFC_PRJDEF_H


/* ---------------------------------------------------------------------- *//*!

  \file   TFC_prjDef.h
  \brief  Defines the data structure describing a track projection and
          a collection of track projections.
  \author JJRussell - russell@slac.stanford.edu

\verbatim

   CVS $Id: TFC_prjDef.h,v 1.1 2007/06/17 22:14:18 russell Exp $
\endverbatim

                                                                         */
/* --------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *\
 *
 * HISTORY
 * -------
 *
 * DATE     WHO WHAT
 * -------- --- ---------------------------------------------------------
 * 03.17.07 jjr Cloned from TFC_projectionDef.h to make transition to the
 *              new structure and code.
 * 03.12.05 jjr Moved definition of TFC_stripDef into this file,
 *              eliminated TFC_stripDef.h
 * 12.22.04 jjr Cloned from DFC/src/TFC_projectionDef.h
 *
\* ---------------------------------------------------------------------- */


#include "EFC/TFC_prjList.h"


/* ====================================================================== */
/* Opaque References                                                      */
/* ---------------------------------------------------------------------- */

#ifndef         TFC_PRJ_TD
#define         TFC_PRJ_TD
typedef struct _TFC_prj        TFC_prj;
#endif

/* ====================================================================== */



#ifdef __cplusplus
extern "C" {
#endif


/* ====================================================================== */
/* Data structures, public                                                */
/* ---------------------------------------------------------------------- *//*!

  \struct  _TFC_hit
  \brief    Information about a hit on a projection
                                                                          *//*!
  \typedef  TFC_hit
  \brief    Typedef for struct _TFC_hit
                                                                          */
/* ---------------------------------------------------------------------- */
struct _TFC_hit
{
      signed short int strip;  /*!< The strip number                      */
    unsigned char      tower;  /*!< The tower number of the strip         */
    unsigned char        idx;  /*!< The index of the hit in EDR_tkr       */
};
/* ---------------------------------------------------------------------- */
#ifndef TFC_HIT_TD
#define TFC_HIT_TD
typedef struct _TFC_hit TFC_hit;
#endif
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \struct _TFC_prjPrms
  \brief   The parameters that describe a projection
                                                                          *//*!
  \typedef TFC_prjPrms
  \brief   Typedef struct _TFC_prjPrms
                                                                          */
/* ---------------------------------------------------------------------- */
struct _TFC_prjPrms
{
    short int           xy0; /*!< Position at the top/bottom layer       */
    short int           dxy; /*!< Change in strip number                 */
    unsigned short int   dz; /*!< Change in z layer position (always > 0)
                                  between the two adjacent layers
                                  This is in um and has values between
                                  about 29500 and 35000                  */
    unsigned char     tower; /*!< Tower number                           */
    unsigned char     layer; /*!< Layer number                           */
    unsigned short  rsvd[2]; /*!< Reserved                               */

};
/* ---------------------------------------------------------------------- */
#ifndef         TFC_PRJPRMS_TD
#define         TFC_PRJPRMS_TD
typedef struct _TFC_prjPrms TFC_prjPrms;
#endif
/* ---------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \struct _TFC_prj
  \brief   The data structure describing a track projection.
                                                                         *//*!
  \typedef TFC_prj
  \brief   Typedef for struct _TFC_prj
                                                                         */
/* --------------------------------------------------------------------- */
struct _TFC_prj
{
  TFC_prjNode     topNode; /*!< Top node links                           */
  TFC_prjNode     botNode; /*!< Bottom node links                        */
  TFC_prjPrms         top; /*!< Parameters at top    of the projection   */
  TFC_prjPrms         bot; /*!< Parameters at bottom of the projection   */
  int          acdTopMask; /*!< ACD top tile candidates                  */
  int            acdXMask; /*!< ACD x facing candidates                  */
  int            acdYMask; /*!< ACD y facing candidates                  */
  unsigned char skirtMask; /*!< Mask of which skirt region prj strikes   */
  unsigned char       min; /*!< Beginning layer number of the projection */
  unsigned char       max; /*!< Ending    layer number of the projection */
  unsigned char     nhits; /*!< Number of hits assigned                  */
  unsigned         layers; /*!< Bit mask representing the struck layers  */
  TFC_hit        hits[18]; /*!< Hits assigned to proj                    */
};
/* --------------------------------------------------------------------- */
#ifndef         TFC_PRJ_TD
#define         TFC_PRJ_TD
typedef struct _TFC_prj    TFC_prj;
#endif
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \struct  _TFC_prjDir
  \brief    Directory structure to locate the projections
                                                                         *//*!
  \typedef  TFC_prjDir
  \brief    Typedef for struct \e TFC_prjDir
                                                                         */
/* --------------------------------------------------------------------- */
struct _TFC_prjDir
{
  unsigned short int  idx; /*!< Index to the first projection            */
  unsigned char      xCnt; /*!< Count of the X projections               */
  unsigned char      yCnt; /*!< Count of the Y projections               */
};
/* --------------------------------------------------------------------- */
#ifndef         TFC_PRJDIR_TD
#define         TFC_PRJDIR_TD
typedef struct _TFC_prjDir    TFC_prjDir;
#endif
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \struct _TFC_prjs
  \brief   The data structure describing a collection of track projections
                                                                         *//*!
  \typedef TFC_prjs
  \brief   Typedef for struct _TFC_prjs.
                                                                         */
/* --------------------------------------------------------------------- */
struct _TFC_prjs
{
  unsigned short int    maxCnt; /*!< Maximum # of projections available  */
  unsigned short int    curCnt; /*!< Current # of projections in use     */
  unsigned short int    twrMsk; /*!< Mask of the tower with projections  */
  TFC_prjDir           dir[16]; /*!< Directory of the projections by twr */
  TFC_prjList       top[2][16]; /*!< List of projections sorted by
                                     their top most tower                */
  TFC_prjList       bot[2][16]; /*!< List of projections sorted by
                                     their bottom most tower             */
  TFC_prj           prjs[1000]; /*!< List of projections                 */
};
/* --------------------------------------------------------------------- */
#ifndef         TFC_PRJS_TD
#define         TFC_PRJS_TD
typedef struct _TFC_prjs    TFC_prjs;
#endif
/* ====================================================================== */




/* ====================================================================== */
/* Inline, prototypes                                                     */
/* ---------------------------------------------------------------------- */

static __inline TFC_prj *TFC__prjFromTopNode (const TFC_prjNode *node)
                                                               ATTR_UNUSED_OK;

static __inline TFC_prj *TFC__prjFromBotNode (const TFC_prjNode *node)
                                                               ATTR_UNUSED_OK;

/* ====================================================================== */




/* ====================================================================== */
/* Inline, implementation                                                 */
/* ---------------------------------------------------------------------- *//*!

  \fn           static __inline                                           \
                TFC_prj *TFC__prjFromTopNode (const TFC_prjNode *node)

  \brief        Given a top projection node, returns a pointer to the
                projection
  \return       A pointer to the projection

  \param node   The top projection node
                                                                          */
/* ---------------------------------------------------------------------- */
static __inline TFC_prj *TFC__prjFromTopNode (const TFC_prjNode *node)
{
    return (TFC_prj *)((char *)node - (char *)(&((TFC_prj *)(0))->topNode));
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn           static __inline                                           \
                TFC_prj *TFC__prjFromBotNode (const TFC_prjNode *node)

  \brief        Given a bottom projection node, returns a pointer to the
                projection

  \return       A pointer to the projection

  \param node   The bottom projection node
                                                                          */
/* ---------------------------------------------------------------------- */
static __inline TFC_prj *TFC__prjFromBotNode (const TFC_prjNode *node)
{
    return (TFC_prj *)((char *)node - (char *)(&((TFC_prj *)(0))->botNode));
}
/* ====================================================================== */

#ifdef __cplusplus
}
#endif

#endif
