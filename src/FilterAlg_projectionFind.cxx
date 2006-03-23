/* ---------------------------------------------------------------------- *//*!
   
   \file  TFC_projectionFind.c
   \brief Finds projections in the TKR
   \author JJRussell - russell@slac.stanford.edu

\verbatim

    CVS $Id
\endverbatim 

                                                                          */
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *\
 * 
 * HISTORY
 * -------
 *
 * DATE     WHO WHAT
 * -------- --- ---------------------------------------------------------
 * 03.12.05 jjr Eliminated TFC_stripDef.h, it has been incorporated into
 *              TFC_projectionDef.h
 * 02.02.05 jjr Error in the findProjection when replacing the top layer
 *              seed hit with a better one. The code should have reinstated
 *              the original hit, then replaced it with the new and
 *              improved hit. Instead it reinstated the original hit, then
 *              promptly removed. The code failed to refresh the index
 *              to the new hit before eliminating it.
 * 01.13.05 jjr Changed the logic on seed hit elimination. This was either
 *              a bug or a misdesign depending on your viewpoint. If the
 *              top seed hit was replaced by a better one, one now 
 *              eliminates the new one from further use and restores the
 *              original one so that it can be used later.
 * 01.13.05 jjr Readdressed problem in 01.02.05. Have added an extra 
 *              argument to all the findMatch routines. There is now
 *              1 argument setting the acceptable tolerance for a track
 *              and one argument giving the best track so far. This 
 *              allows usage of these routines both when initially looking
 *              for a match and when trying to better a match. If may
 *              still be better to separate these two styles to avoid 
 *              logic in the inner loop..
 * 01.03.05 jjr Corrected error in extendProjectionUp. When finding
 *              the next layer up, the reassignment of the mid and 
 *              bottom strips was incorrectly done. The routine reassigned
 *              the top and mid strips, likely a cut and paste error
 *              from the extendProjectionDwn.
 * 01.02.05 jjr Corrected error in findMatchTop. This routine really
 *              should become two routines, One to find a virgin match,
 *              the other to improve an existing match.
 * 01.02.05 jjr Corrected mistake in the extend Up/Dwn routines that 
 *              allowed the 2 layer projection tolerance to reach 96
 * 12.22.04 jjr Cloned from DFC/src/TFC_projectionTowerFind.c
 *
\* ---------------------------------------------------------------------- */



#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "EDS/FFS.h"
#include "EFC/EDM.h"
#include "EFC/TFC_projectionDef.h"
#include "TFC_projectionFind.h"
#include "TFC_geometryDef.h"
#include "EDS/EDR_tkr.h"
#include "EFC/TFC_geos.h"
#include "EFC/TFC_geoIds.h"


/* ---------------------------------------------------------------------- *//*!

  \fn     int _abs (int x)
  \brief  Returns the absolute value of the signed integer x
  \return The absolute value of the signed integer x

   This function at one time was an intrinsic function of the GNU-C
   compiler. For some reason (at least I think it did), it disappeared and
   was replaced was with a call. I do not want that, so this function
   places the absolute value function as an inline.

   The algorithm is simple and uses no if's

  \code
                   
     X               2 = 0x00000002    -2 = 0xfffffffe
     Y = X >> 31         0x00000000         0xffffffff
     Z = X ^ Y           0x00000002         0x00000001
     ABS(X) = Z - Y      0x00000002         0x00000002
  \endcode
									  */
/* ---------------------------------------------------------------------- */  
static __inline int _abs(int x)
{
  int y = x >> 31;
  return (x ^ y) - y;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \def    FIND_TOLERANCE
  \brief  The tolerance, in units of strip widths, to accept a middle strip.
          This tolerance is used when comparing an actual hit strip address
          to the predicted strip address when initially finding a seed
          projection. Since this is a comparison with an absolute value,
          it is specified as an unsigned quantity.
									  *//*!
  \def    BIG_TOLERANCE
  \brief  An impossibly large tolerance to cover the first time through.
          Since this is a comparison with an absolute value, it is specified
          as an unsigned quantity.
                                                                          *//*!
  \def    EXTEND_1LAYER_TOLERANCE
  \brief  The tolerance, in units of strip widths, to accept a strip when
          extending a projection 1 layer. Since this is a comparison with
          an absolute value, it is specified as an unsigned quantity.
                                                                          *//*!
  \def    EXTEND_2LAYER_TOLERANCE
  \brief  The tolerance, in units of strip widths, to accept a strip when
          extending a projection 2 layers. Since this is a comparison with
          an absolute value, it is specified as an unsigned quantity.
                                                                          */
/* ---------------------------------------------------------------------- */
#define          FIND_TOLERANCE ((unsigned int)32)
#define           BIG_TOLERANCE ((unsigned int)0xffffffff)
#define EXTEND_1LAYER_TOLERANCE ((unsigned int)32)
#define EXTEND_2LAYER_TOLERANCE ((unsigned int)64)
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *\
 |                                                                        |
 |                      DEBUGGING/DEVELOPMENT MACROS                      |
 |                      ----------------------------                      |
 | The following macros are used to when debugging of developing the code.|
 | They are not compiled into the code unless the EDM facility is active. |
 | They produce no output unless the print level is DEBUG or lower.       |
 |                                                                        |
\* ---------------------------------------------------------------------- */


EDM_CODE(EDM_level TFC_ProjectionTowerFind_edm = EDM_K_DEBUG;)


/* ---------------------------------------------------------------------- *//*!

  \def    _PRINT_TITLE(_tower)
  \brief  Prints a header line if EDM is active and the print level
          is DEBUG or lower. otherwise is a NOP.
  \param _tower The tower number being used to find the projections        
									  */
/* ---------------------------------------------------------------------- */
#define _PRINT_TITLE(_tower)                                              \
   EDM_DEBUGPRINTF ((                                                     \
       TFC_ProjectionTowerFind_edm,                                       \
      "\n"                                                                \
      "%1.1X%c OP Lyr  Top  Mid  Bot Zfactor Pred   Diff  Tol Status\n"   \
           " ---- --- ---- ---- ---- ------- ---- ------ ---- ------\n",  \
           _tower&0xf, (int)_tower < 0 ? 'y' : 'x'))
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \def    _PRINT_CHK(_lay, _top, _mid, _bot, _zex, _pred, _res, _tol)
  \brief  Prints the status of checking a middle point when finding a
          seed projection if EDM is active and the print level is DEBUG
          or lower; otherwise is a NOP.
  \param  _lay  The layer number of the top layer being used.
  \param  _top  The strip number in the top    layer.
  \param  _mid  The strip number in the middle layer.
  \param  _bot  The strip number in the bottom layer.
  \param  _zex  The zextension factor being used to predict the middle
                strip address.
  \param  _pred The predicted middle layer strip number.
  \param  _res  The residual (actual - predicted).
  \param  _tol  The tolerance used to declare a match between the actual
                and predicted hit number.

   This macro prints a status line for every triple of strip addresses
   being examined as a possible seed projection.
									  */
/* ---------------------------------------------------------------------- */
#define _PRINT_CHK(_lay, _top, _mid, _bot, _zex, _pred, _res, _tol)       \
  EDM_DEBUGPRINTF ((                                                      \
           TFC_ProjectionTowerFind_edm,                                   \
           " Seed %3d %4d %4d %4d %7d %4d %6d %4d %s\n",                  \
            _lay, _top, _mid, _bot, _zex, _pred, _res, _tol,              \
            _abs(_res) <= _tol ? "found" : ""))
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \def   _PRINT_EXTI(_msk)
  \brief  Prints a line introducing the beginning of looking for
          additional layers to extend an existing projection, if
          EDM is active and the print level is DEBUG or lower; otherwise
          is a NOP.
  \param  _msk  A bit mask of the layers which have hits in them and,
                thus, are possible candidate extension layers.
									  */
/* ---------------------------------------------------------------------- */
#define _PRINT_EXTI(_msk);                                                \
  EDM_DEBUGPRINTF ((TFC_ProjectionTowerFind_edm,                          \
                   "  Ext %*c %8.8x\n", 43, ' ',  _msk))
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \def   _PRINT_EXT(_lay, _top, _mid, _bot, _zex, _pred, _res, _tol)
  \brief  Prints the status of checking a point when extending a
          projection if EDM is active and the print level is DEBUG or
          lower; otherwise is a NOP.
  \param  _lay  The layer number of the top layer being used.
  \param  _top  The strip number in the top    layer.
  \param  _mid  The strip number in the middle layer.
  \param  _bot  The strip number in the bottom layer.
  \param  _zex  The zextension factor being used to predict the strip
                address in the extension layer
  \param  _pred The predicted extension layer strip number.
  \param  _res  The residual (actual - predicted).
  \param  _tol  The tolerance used to declare a match between the actual
                and predicted hit number.
                                                                              
                                                                          */
/* ---------------------------------------------------------------------- */
#define _PRINT_EXT(_lay, _top, _mid,  _bot, _zex, _pred, _res, _tol)      \
  EDM_DEBUGPRINTF ((TFC_ProjectionTowerFind_edm,                          \
                   "  Ext %3d %4d %4d %4d %7d %4d %6d %4d %s\n",          \
                     _lay, _top, _mid, _bot, _zex, _pred, _res, _tol,     \
                     _abs(_res) <= _tol ? "extended" : ""))
   
/* ---------------------------------------------------------------------- */



#ifdef __cplusplus
extern "C" {
#endif


/* ---------------------------------------------------------------------- *//*!

  \struct _MatchVal_bf
  \brief  The return value of the various 'match' routines, expressed as
          bit fields.

   Each 'match' routine attempts to match a predicted strip address with
   an actual strip address. They each return a status word which is packed
   with index of the hit strip, the absolute value of the residual (actual-
   predicted) and  the hit strip address.
                                                                          *//*!
  \typedef MatchVal_bf
  \brief   Typedef for struct \a _MatchVal_bf
                                                                          *//*!
  \union  _MatchVal
  \brief   Union of the bit field representation of the match routine's
           status word and a 32-bit signed integer.
                                                                          *//*!
  \typedef MatchVal
  \brief   Typedef for union \a _MatchVal
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _MatchVal_bf
{
    unsigned int      idx:8;   /*!< Index of strip */
      signed int residual:8;   /*!< Residual       */
    unsigned int    strip:16;  /*!< Strip number   */
}
MatchVal_bf;

/* ---------------------------------------------------------------------- */
    
typedef union _MatchVal
{
    int         si;           /*!< Value as a signed integer   */
    MatchVal_bf bf;           /*!< Value as a series of fields */
}
MatchVal;
/* ---------------------------------------------------------------------- */


    
static __inline int           project (int                           top,
                                       int                           bot,
                                       int                            dz);

static __inline int         projectDx (int                            x0,
                                       int                            dx,
                                       int                            dz);

static __inline int        projectMid (int                           top,
                                       int                           bot,
                                       int                            dz);


static __inline MatchVal findMatchTop (unsigned int            tolerance,
				       unsigned int         bestResidual,
                                       int                    prediction,
                                       int                        topMsk,
                                       const EDR_tkrCluster *topClusters,
                                       int                   topLayerNum,
                                       int                      midStrip,
                                       int                      botStrip,
                                       int                            dz);


static __inline MatchVal findMatchMid (unsigned int            tolerance,
				       unsigned int         bestResidual,
                                       int                    prediction,
                                       int                        midMsk,
                                       const EDR_tkrCluster *midClusters,
                                       int                   topLayerNum,  
                                       int                      topStrip,
                                       int                      botStrip,
                                       int                            dz);
    
static __inline MatchVal findMatchBot (unsigned int            tolerance,
				       unsigned int         bestResidual,
                                       int                    prediction,
                                       int                        midMsk,
                                       const EDR_tkrCluster *midClusters,
                                       int                   botLayerNum,
                                       int                      topStrip,
                                       int                      botStrip,
                                       int                            dz);
    
static __inline void storeProjection  (TFC_projection               *prj,
                                       int                   topLayerNum,
                                       int                         paTop, 
                                       int                         paMid, 
                                       int                         paBot);
                                     

static          int    findProjection (int                       towerId,
                                       int                        layers,
				       int                       elayers,
                                       int                      layerNum,
                                       int                         zfind,
                                       int                   zseparation,
                                       EDR_tkrLayer                 *top,
                                       TFC_projection               *prj);

#if 0
static __inline int findProjection111 (int                   topLayerNum,
                                       int                        deltaz,
                                       const EDR_tkrCluster  *clusterTop,
                                       const EDR_tkrCluster  *clusterMid,
                                       const EDR_tkrCluster  *clusterBot,
                                       TFC_projection               *prj);
#endif

static __inline int extendProjectionDwn(TFC_projection              *prj,
                                        int                       layers,
                                        int                     layerNum,
                                        EDR_tkrLayer                *nxt,
                                        int                     topStrip,
                                        int                     midStrip,
                                        int                      zextend);

static __inline int extendProjectionUp (TFC_projection              *prj,
                                        int                       layers,
                                        int                     layerNum,
                                        EDR_tkrLayer                *nxt,
                                        int                     midStrip,
                                        int                     botStrip,
                                        int                      zextend);

#ifdef __cplusplus
}
#endif






/* ---------------------------------------------------------------------- *//*!

  \fn     int project (int top, int bot, int dz)
  \brief  Predicts the middle strip address between the \a top and \a bot
          strip addresses.
  \param  top The address of the top strip.
  \param  bot The address of the bottom strip.
  \param  dz  The fractional Z distance of the middle layer between the
              top and bottom layers and the halfway point in units of
              pf TFC_Z_FIND_SCALE_FACTOR. If the middle layer were
              exactly halfway, \a dz would be TFC_Z_FIND_SCALE_FACTOR.
  \return     The predicted address of the middle strip.
                                                                          */
/* ---------------------------------------------------------------------- */
static int project (int top, int bot, int dz)
{
  return  top + ((bot - top) * dz + TFC_Z_FIND_SCALE_FACTOR/2)
                 / TFC_Z_FIND_SCALE_FACTOR;

}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn    int projectDx (int p0, int dx, int dz)
  \brief A strict linear projection from the point \a p0 using a slope
         of \a dx / \a dz. (Note that \a dz is really 1/dz, thus
         avoiding a cycle soaking divide.)
  \param p0 The initial point
  \param dx The change is x
  \param dz The inverse of the change in z, ie the slope is dx * dz.
  \return   The predict point.
                                                                          */
/* ---------------------------------------------------------------------- */  
static int projectDx (int p0, int dx, int dz)
{
  return p0 + (dx * dz + TFC_Z_EXTEND_SCALE_FACTOR/2)
              / TFC_Z_EXTEND_SCALE_FACTOR;
}
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \fn     int projectMid (int top, int bot, int dz)
  \brief  Another projection function, used to predict a point in a skipped
          layer. 
  \return The predicted strip number in the missing layer.

  \param top    The top strip address
  \param bot    The bottom strip address
  \param dz     The scaled distance from the top strip to the missing strip

                                                                          */
/* ---------------------------------------------------------------------- */
static int projectMid (int top, int bot, int dz)
{
  return  (bot * TFC_Z_EXTEND_SCALE_FACTOR + top * dz)
        / (dz             + TFC_Z_EXTEND_SCALE_FACTOR);
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn  MatchVal findMatchTop (unsigned int            tolerance,
		              unsigned int         bestResidual,
                              int                    prediction,
                              int                        topMsk,
                              const EDR_tkrCluster *topClusters,
                              int                   topLayerNum,
                              int                      midStrip,
                              int                      botStrip,
                              int                            dz)
  \brief Finds the best match when extending a projection up.
  \param tolerance    The tolerance used to accept a match. If this value
                      is negative, then one must beat this value
  \param bestResidual Best residual so far
  \param prediction   The predicted strip number
  \param topMsk       The mask of available hits in the top layer
  \param topClusters  The array of clusters in the top layer
  \param topLayerNum  The top layer number
  \param midStrip     The middle strip number
  \param botStrip     The bottom strip number
  \param dz           A geometrical factor used when extending
                      the projection.
  \return             A packed status word, summarizing the
                      best match.
                                                                          */
/* ---------------------------------------------------------------------- */
static MatchVal findMatchTop (unsigned int            tolerance,
			      unsigned int         bestResidual,
                              int                    prediction,
                              int                        topMsk,
                              const EDR_tkrCluster *topClusters,
                              int                   topLayerNum,
                              int                      midStrip,
                              int                      botStrip,
                              int                            dz)
{
   MatchVal mv;

   mv.si = -1;
   
   /* Look for the best match */
   do
   {
       int      topIdx;
       int    topStrip;
       int    residual;
       int absResidual;

       /*
        | Locate the next unused strip address and calculate how far
        | it is from the prediction.
       */
       topIdx      = FFS (topMsk);       
       topMsk      = FFS_eliminate (topMsk, topIdx);
       topStrip    = topClusters[topIdx].strip;
       residual    = topStrip - prediction;
       absResidual = _abs (residual);


       _PRINT_EXT(topLayerNum,
                  topStrip,
                  midStrip,
                  botStrip,
                  dz,
                  prediction,
                  residual,
                  tolerance);

       
       /* Is this the best fit so far? */
       if (absResidual < bestResidual)
       {
           /* Yes, is it good enough */
	   if (absResidual < tolerance)
	   {
   	       /* Save its parameters */
               mv.bf.idx      = topIdx;
	       mv.bf.strip    = topStrip;
	       mv.bf.residual = residual;
	       bestResidual   = absResidual;
	   }
       }

       /* If the residual is positive, it isn't going to get any better */
       if (residual >= 0) break;

   }
   while (topMsk);

   return   mv;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn  MatchVal findMatchBot (unsigned int            tolerance,
                              unsigned int         bestResidual,
                              int                    prediction,
                              int                        botMsk,
                              const EDR_tkrCluster *botClusters,
                              int                   botLayerNum,
                              int                      topStrip,
                              int                      midStrip,
                              int                            dz)
  \brief Finds the best match when extending a projection down.
  \param tolerance    The tolerance used to accept a match
  \param bestResidual The best residual so far
  \param prediction   The predicted strip number
  \param botMsk       The mask of available hits in the bottom layer
  \param botClusters  The array of clusters in the bottom layer
  \param botLayerNum  The bottom layer number
  \param topStrip     The top    strip number
  \param midStrip     The middle strip number  
  \param dz           A geometrical factor used when extending
                      the projection.
  \return             A packed status word, summarizing the
                      best match.
                                                                          */
/* ---------------------------------------------------------------------- */
static MatchVal findMatchBot (unsigned int            tolerance,
			      unsigned int         bestResidual,
                              int                    prediction,
                              int                        botMsk,
                              const EDR_tkrCluster *botClusters,
                              int                   botLayerNum,
                              int                      topStrip,
                              int                      midStrip,
                              int                            dz)
{
   MatchVal mv;

   mv.si = -1;
   
   /* Look for the best match */
   do
   {
       int      botIdx;
       int    botStrip;
       int    residual;
       int absResidual;

       /*
        | Locate the next unused strip address and calculate how far
        | it is from the prediction.
       */
       botIdx      = FFS (botMsk);       
       botMsk      = FFS_eliminate (botMsk, botIdx);
       botStrip    = botClusters[botIdx].strip;
       residual    = botStrip - prediction;
       absResidual = _abs (residual);


       _PRINT_EXT(botLayerNum,
                  topStrip,
                  midStrip,
                  botStrip,
                  dz,
                  prediction,
                  residual,
                  tolerance);

       /* Is this a best fit so far? */
       if (absResidual < bestResidual)
       {
	   /* Yes, is it good enough? */
  	  if (absResidual < tolerance)
	  {
	     /* Save its parameters */
             mv.bf.idx    = botIdx;
             mv.bf.strip  = botStrip;
             bestResidual = absResidual;
	  }
       }

       /* If the residual is positive, it isn't going to get any better */
       if (residual >= 0) break;
   }
   while (botMsk);

   return  mv;
}



/* ---------------------------------------------------------------------- *//*!

  \fn     MatchVal findMatchMid (unsigned int            tolerance,
                                 unsigned int         bestResidual,
                                 int                    prediction,
                                 int                        midMsk,
                                 const EDR_tkrCluster *midClusters,
                                 int                   topLayerNum,
                                 int                      topStrip,
                                 int                      botStrip,
                                 int                            dz)
  \brief  Finds the best match when projecting to a middle layer
  \return A packed status word, summarizing the best match.

  \param tolerance    The tolerance used to accept a match
  \param bestResidual The best residual so far
  \param prediction   The predicted strip number
  \param midMsk       The mask of available hits in the middle layer
  \param midClusters  The array of clusters in the middle layer
  \param topLayerNum  The top layer number
  \param topStrip     The top strip number
  \param botStrip     The bottom strip number  
  \param dz           A geometrical factor used when extending
                      the projection to the middle layer.
                                                                          */
/* ---------------------------------------------------------------------- */
static MatchVal findMatchMid (unsigned int            tolerance,
			      unsigned int         bestResidual,
                              int                    prediction,
                              int                        midMsk,
                              const EDR_tkrCluster *midClusters,
                              int                   topLayerNum,
                              int                      topStrip,
                              int                      botStrip,
                              int                            dz)
{
   MatchVal mv;

   mv.si  = -1;
   
   /* Look for the best match */
   do
   {
       int      midIdx;
       int    midStrip;
       int    residual;
       int absResidual;

       /*
        | Locate the next unused strip address and calculate how far
        | it is from the prediction.
       */
       midIdx      = FFS (midMsk);       
       midMsk      = FFS_eliminate (midMsk, midIdx);
       midStrip    = midClusters[midIdx].strip;
       residual    = midStrip - prediction;
       absResidual = _abs (residual);

               
//       _PRINT_CHK(topLayerNum,
//                  topStrip,
//                  midStrip,
//                  botStrip,
//                  dz,
//                  prediction,
//                  residual,
//                  tolerance);

               
       /* Is this a better fit */
       if (absResidual < bestResidual)
       {
	  /* Yes it is good enough? */
          if (absResidual < tolerance)
          {
             /* Yes, save its parameters */
             mv.bf.idx      = midIdx;
             mv.bf.strip    = midStrip;
	     mv.bf.residual = residual;
             bestResidual   = absResidual;
	  }
       }

       /* If the residual is positive, it isn't going to get any better */
       if (residual >= 0) break;
   }
   while (midMsk);

   
   return mv;
}
/* ---------------------------------------------------------------------- */
  



/* ---------------------------------------------------------------------- *//*!

  \fn void storeProjection (TFC_projection *prj,
                            int     topLayerNum,
                            int           paTop, 
                            int           paMid, 
                            int           paBot)
  \brief Simple routine to store the initial parameters describing a
         projection
  \param         prj: The projection
  \param topLayerNum: The number of the upper most layer
  \param       paTop: Strip number of topmost layer of the projection
  \param       paMid: Strip number of middle  layer of the projection
  \param       paBot: Strip number of bottom  layer of the projection
                                                                          */
/* ---------------------------------------------------------------------- */
static __inline void storeProjection (TFC_projection *prj,
                                      int     topLayerNum,
                                      int           paTop, 
                                      int           paMid, 
                                      int           paBot) 
{
   int topLayerNumM2 = topLayerNum - 2;
   TFC_strip   *hits = prj->hits + topLayerNumM2;

   prj->intercept = paTop;
   prj->slope     = paTop - paMid;
   prj->max       = topLayerNum;
   prj->min       = topLayerNumM2;
   prj->layers    = 0x7 << (topLayerNumM2);
   hits[2]        = paTop;
   hits[1]        = paMid;
   hits[0]        = paBot;
   prj->nhits     = 3;
  
   return;
}
/* ---------------------------------------------------------------------- */



#if 0
static __inline int findProjection111 (int            topLayerNum,
                                       int                 deltaz,
                                       EDR_tkrCluster *clusterTop,
                                       EDR_tkrCluster *clusterMid,
                                       EdR_tkrCluster *clusterBot,
                                       TFC_projection        *prj)
/*
  DESCRIPTION
  -----------
  Special case routine to find an projection when there is only one
  hit in each layer. This case happens fairly frequently and can be
  optimized, since there is no looping.

  PARAMETERS
  ----------
 topLayerNum: The number of the upper most layer

      deltaz: The distance from the top layer to the middle layer in units
              TKR_Z_FIND_SCALE.

  clusterTop: Pointer to the top most layer cluster

  clusterMid: Pointer to the middle   layer cluster

  clusterBot: Pointer to the bottom   layer cluster

         prj: Where to store the projection

  RETURNS
  -------
  1 if a match was found, 0 if not. This is really the number of projections
  found, but in this special case, it can only be 1 or 0.
*/  
{
   int prediction;
   int   residual;

   
   int paTop = clusterTop->strip;
   int paMid = clusterMid->strip;
   int paBot = clusterBot->strip;
  

   /* If any of the hits are already used, no track possible */
   if (isUsed (paTop | paMid | paBot) ) return 0;

   
   prediction = project (paTop, paBot, deltaz);
   residual   = paMid - prediction;
  _PRINT_L2ALIGN_CHK (paTop, paMid, paBot, prediction, residual);
  
  
   if (_abs(residual) < ALIGNMENT_TOLERANCE)
   {
        storeProjection (topLayerNum,
                         paTop, stripTop,
                         paMid, stripMid,
                         paBot, stripBot,
                         prj);
        return 1;
   }

  _PRINT_L2ALIGN_NEWLINE;
  
  return 0;
  
}
#endif


/* ---------------------------------------------------------------------- *//*!

  \fn int extendProjectionDwn (TFC_projection    *prj,
                               int             layers,
                               int           layerNum,
                               EDR_tkrLayer      *bot,
                               int           topStrip,
                               int           midStrip,
                               int            zextend)
  \brief Attempts to extend a projection down through the next hit layers

  \param       prj The projection to be extended.
  \param    layers A bit mask of the hit layers. The next layer is in
                   the MSB of this mask.
  \param  layerNum The layer number of layer to be searched.
  \param       bot Pointer to the next layer structure, containing the
                   beginning and ending address to the hits on the layer
                   and array of hits.
  \param  topStrip The projection's top layer strip number
  \param  midStrip The projection's middle layer strip number
  \param   zextend The delta z's to the subsequent layers in units of
                   TFC_Z_FIND_SCALE_FACTOR strips. These are used when
                   projecting to the subsequent layers.
  \return          The bit mask of layers with unused hits.
  

  This routine attempts to extend a projection through the next layers.
  The routine examines the next layer. If it has hits, a prediction
  is calculated and the hits of that layer are examined. If there are no
  hits or no match, the next layer is examined. If two layers in a row
  fail to produce a match, the routine quits searching. 
                                                                          */  
/* ---------------------------------------------------------------------- */
static __inline int extendProjectionDwn (TFC_projection    *prj,
                                         int             layers,
                                         int           layerNum,
                                         EDR_tkrLayer      *bot,
                                         int           topStrip,
                                         int           midStrip,
                                         int            zextend)
{
   TFC_strip         *hits;
   unsigned int prj_layers;
   int             zdelta1;
   int             zdelta2;
   int           retLayers;
   int            minLayer;
   int               nhits;

   
   /* Remove the layers above this one, we are searching down  */
   retLayers = layers;
   layers  <<= (32 - layerNum);   

   
   /* If either of the next two layers are not hit, just return */
   if ((layers & 0xc0000000) == 0) return retLayers;

   
   minLayer   = layerNum;     // This just keeps the compiler happy
   prj_layers = 0;
   nhits      = 0;
   hits       = prj->hits + layerNum;
   zdelta1    = zextend & 0xffff;
   zdelta2    = zextend >>    16;

   
  _PRINT_EXTI(layers);

  
  /* Loop over the layers until miss matching 2 layers or when out of layers */
  while (1)
  {
      int                            dx;
      int                    prediction;
      int                     tolerance;   
      int                          skip;
      int                        botMap;
      const EDR_tkrCluster *botClusters;
      int                      botStrip;
      MatchVal                       mv;


      /* Always project down one layer */
      dx         = midStrip - topStrip;
      prediction = projectDx (midStrip, dx, zdelta1);
      

      /* Guaranteed that one of the next two layers are hit */
      if (layers < 0) 
      {
          /* Looking down one layer */
          skip       = 1;
          tolerance  = EXTEND_1LAYER_TOLERANCE;
      }
      else
      {
          /* Looking down two layers. */
          skip       = 2;
          tolerance  = EXTEND_2LAYER_TOLERANCE;
          prediction += dx;
      }


      /* Adjust the current layer to be searched */
      layers    <<= skip;
      bot        -= skip;
      layerNum   -= skip;
      botMap      = bot->map; // (signed int )0x80000000 >> (bot->cnt - 1);
      botClusters = bot->beg;
      
      /*
       | This loop scans until a match is found or 2 unmatched layers
       | have been encountered.
      */
      while (1)
      {
          mv = findMatchBot (tolerance,
			     BIG_TOLERANCE,
                             prediction,
                             botMap,
                             botClusters,
                             layerNum,
                             topStrip,
                             midStrip,
                             zdelta1 + (skip - 1) * TFC_Z_EXTEND_SCALE_FACTOR);
	  //FindMatchBotCnt++;
          
          /* Check if found an acceptable match */          
          if (mv.si >= 0) break;


          /*
           | Have already skipped at layer or
           | have looked for a hit but failed to find a match,
           | and the next layer is not hit, that's it.
          */
          if (skip == 2 || (layers >= 0)) goto EXIT;

          /*
           | Adjust the search parameters to be compatiable with looking
           | in the next layer down
          */
          skip        = 2;
          tolerance   = EXTEND_2LAYER_TOLERANCE;
          prediction += dx;

	  layers    <<= 1;
          bot        -= 1;
          layerNum   -= 1;
          botMap      = bot->map;
          botClusters = bot->beg;          
      }
          

      /*
       | Found a match
       | Mark the hit as used. The hit is not stored until it
       | has been determined whether one or two layers were skipped.
      */
      botMap      = FFS_eliminate (botMap, mv.bf.idx);
      botStrip    = mv.bf.strip;
      minLayer    = layerNum;
      prj_layers |= (1 << layerNum);
      nhits      += 1;
              

      /* Update the map, if no unused hits left, eliminate this layer */
      if ((bot->map &= botMap) == 0) retLayers &= ~(1 << layerNum);

      
      /* Swap the extension factors if did not skip a layer */
      if (skip == 1)
      {
          /* ... swap the extension factor */
          int tmp;

          *--hits = botStrip;
          
          /* If neither of the next two layers hit, that's it */
          if ((layers & 0xc0000000) == 0) break;
          
          tmp       = zdelta1;
          zdelta1   = zdelta2;
          zdelta2   = tmp;
          topStrip  = midStrip;
          midStrip  = botStrip;
      }
      else
      {
          /*
           | Move the top layer to the mid, the prediction to the mid
           | This makes it look like we are extending from the missing
           | layer.
           |
           | Since there was no actual hit for the middle strip, a guess
           | is made based on connecting the former middle strip with
           | the matched bottom strip and finding where it intersects
           | the missing layer. (This is not a great drawing, but maybe
           | it conveys the idea
           |
           |    topStrip +
           |              .  
           |                .
           |                  .
           |    midStrip        +          
           |                     \
           |                      \
           |    missing            *       => topStrip
           |                        \
           |    botStrip             +     => midStrip
           |
           | !!! KLUDGE !!!
           | --------------
           | There are two things wrong with this. What's really needed 
           | is to project the original midStrip and botStrip to a new
           | botStrip. This method just hokes this up so that the code
           | above has the variables it needs to do the projection.
           |
           | Even forgetting that, the projection should be done using
           | the middle layer projection technique. This would avoid
           | an unneccessary divide.
          */
          topStrip  = projectMid (midStrip, botStrip, zdelta1);
         *--hits    = topStrip;
         *--hits    = botStrip;

         
         /* If neither of the next two layers hit, that's it */
          if ((layers & 0xc0000000) == 0) break;

          midStrip  = botStrip;
	  //          tolerance = abs (mv.bf.residual) / 2;
      }
  }
  

  EXIT:
  if (nhits)
  {
      /* Count the number of hits, add the layer mask */
      prj->nhits  += nhits;
      prj->min     = minLayer;
      prj->layers |= prj_layers;
  }
  
  return retLayers;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn int extendProjectionUp (TFC_projection    *prj,
                              int             layers,
                              int           layerNum,
                              EDR_tkrLayer      *bot,
                              int           midStrip,
                              int           botStrip,
                              int            zextend)
  \brief Attempts to extend a projection up through the next hit layers
  
  \param       prj The projection to be extended.
  \param    layers A bit mask of the hit layers. The next layer is in
                   the MSB of this mask.
  \param  layerNum The layer number of layer to be searched.                   
  \param       top Pointer to the next layer structure, containing the
                   beginning and ending address to the hits on the layer
                   and array of hits.
  \param  midStrip The projection's middle layer strip number.
  \param  botStrip The projection's top layer strip number.
  \param   zextend The delta z's to the subsequent layers in units of
                   TFC_Z_FIND_SCALE_FACTOR strips. These are used when
                   projecting to the subsequent layers.
  \return          The bit mask of layers with unused hits.
  

  This routine attempts to extend a projection through the next layers.
  The routine examines the next layer. If it has hits, a prediction
  is calculated and the hits of that layer are examined. If there are no
  hits or no match, the next layer is examined. If two layers in a row
  fail to produce a match, the routine quits searching. 
                                                                          */  
/* ---------------------------------------------------------------------- */
static __inline int extendProjectionUp  (TFC_projection    *prj,
                                         int             layers,
                                         int           layerNum,
                                         EDR_tkrLayer      *top,
                                         int           midStrip,
                                         int           botStrip,
                                         int            zextend)
{
   TFC_strip         *hits;
   unsigned int prj_layers;
   int             zdelta1;
   int             zdelta2;
   int           retLayers;
   int            maxLayer;
   int               nhits;

   
   /* Remove the layers above this one, we are searching down  */
   retLayers = layers;
   layers  >>= layerNum + 1;   

   
   /* If either of the next two layers are not hit, just return */
   if ((layers & 0x3) == 0) return retLayers;

   
   maxLayer   = layerNum;     // This just keeps the compiler happy
   prj_layers = 0;
   nhits      = 0;
   hits       = prj->hits + layerNum + 1;
   zdelta1    = zextend & 0xffff;
   zdelta2    = zextend >>    16;

   
  _PRINT_EXTI(layers);

  
  /* Loop over the layers until miss matching 2 layers or when out of layers */
  while (1)
  {
      int                            dx;
      int                    prediction;
      int                          skip;
      int                     tolerance;
      int                        topMap;
      const EDR_tkrCluster *topClusters;
      int                      topStrip;
      MatchVal                       mv;


      /* Always project one layer up */
      dx         = midStrip - botStrip;
      prediction = projectDx (midStrip, dx, zdelta1);
      

      /* Guaranteed that one of the next two layers are hit */
      if ((layers&1)) 
      {
          /* Looking up one layer */
          skip       = 1;
          tolerance  = EXTEND_1LAYER_TOLERANCE;
      }
      else
      {
          /* Looking up two layers. */
          skip        = 2;
          tolerance   = EXTEND_2LAYER_TOLERANCE;
          prediction += dx;
      }


      /* Adjust the current layer to be searched */
      layers    >>= skip;
      top        += skip;
      layerNum   += skip;
      //topMap     = top->map;
      topMap      = (signed int)0x80000000 >> (top->cnt - 1);
      topClusters = top->beg;
      
      /*
       | This loop scans until a match is found or 2 unmatched layers
       | have been encountered.
      */
      while (1)
      {
          mv = findMatchTop (tolerance,
			     BIG_TOLERANCE,
                             prediction,
                             topMap,
                             topClusters,
                             layerNum,
                             midStrip,
                             botStrip,
                             zdelta1);
          
          /* Check if found an acceptable match */          
          if (mv.si >= 0) break;


          /*
           | Have already skipped at layer or
           | have looked for a hit but failed to find a match,
           | and the next layer is not hit, that's it.
          */
          if (skip == 2 || (layers & 1) == 0) goto EXIT;

          
          /*
           | Adjust the search parameters to be compatiable with looking
           | in the next layer up
          */
          skip        = 2;
          tolerance   = EXTEND_2LAYER_TOLERANCE;
          prediction += dx;
          top        += 1;
          layerNum   += 1;
          layers    >>= 1;
          topMap      = (signed int)0x80000000 >> (top->cnt - 1);
          topClusters = top->beg;
          
      }
          

      /*
       | Found a match
       | Mark the hit as used and
       | store the results
      */
      topMap      = FFS_eliminate (topMap, mv.bf.idx);
      topStrip    = mv.bf.strip;
      maxLayer    = layerNum;
      prj_layers |= (1 << layerNum);
      nhits      += 1;

              

      /* Update the map, if no unused hits left, eliminate this layer */
      if ( (top->map &= topMap) == 0) retLayers &= ~(1 << layerNum);

      
      /* Swap the extension factors if did not skip a layer */
      if (skip == 1)
      {
          /* ... swap the extension factor */
          int tmp;

	  *hits++   = topStrip;

	  /* If neither of the next two layers hit, that's it */
	  if ((layers & 0x3) == 0) break;

          tmp       = zdelta1;
          zdelta1   = zdelta2;
          zdelta2   = tmp;
          botStrip  = midStrip;
          midStrip  = topStrip;
      }
      else
      {
          /*
           | Move the bot layer to the mid, the prediction to the mid
           | This makes it look like we are extending from the missing
           | layer.
           |
           | Since there was no actual hit for the middle strip, a guess
           | is made based on connecting the former middle strip with
           | the matched bottom strip and finding where it intersects
           | the missing layer. (This is not a great drawing, but maybe
           | it conveys the idea
           |
           |    topStrip +                 => midStrip
           |              .  
           |                .
           |                  .
           |    missing         *          => botStrip
           |                     \
           |                      \
           |    midStrip           +       
           |                        \
           |    botStrip             +     
           |
           | !!! KLUDGE !!!
           | --------------
           | There are two things wrong with this. What's really needed 
           | is to project the original midStrip and botStrip to a new
           | botStrip. This method just hokes this up so that the code
           | above has the variables it needs to do the projection.
           |
           | Even forgiving that, the projection should be done using
           | the middle layer projection technique. This would avoid
           | an unneccessary divide.
          */
          botStrip  = projectMid (midStrip, topStrip, zdelta1);
          midStrip  = topStrip;
	  *hits++   = botStrip;
          *hits++   = midStrip;
	  //          tolerance = abs (mv.bf.residual) / 2;

	  /* If neither of the next two layers hit, that's it */
	  if ((layers & 0x3) == 0) break;
      }
  }
  

  EXIT:
  if (nhits)
  {
      /* Count the number of hits, add the layer mask */
      prj->nhits  += nhits;
      prj->max     = maxLayer;
      prj->layers |= prj_layers;
  }
  
  return retLayers;
}
/* ---------------------------------------------------------------------- */


  

/* ---------------------------------------------------------------------- *//*!

  \fn int findProjection (int               towerId,
                          int                layers,
                          int               elayers,
                          int           topLayerNum,
                          int                 zfind,
                          int               zextend,
                          EDR_tkrLayer         *top,
                          TFC_projection       *prj)
  \brief Looks for projection in the specified tower starting at
         \a topLayerNum.
  \param towerId     The id of the tower being searched.
  \param layers      A bit mask of the layers with available hits to do
                     the seed 3-in-a-row matching
  \param elayers     A bit mask of the layers with available hits to do
                     the extension
  \param topLayerNum The number of the top layer to begin the seach.
  \param zfind       Geometrical factor used when predicting the
                     middle hit of a seed projection.
  \param zextend     Geometrical factor used when extending a seed
                     projection either up or down.
  \param top         Pointer to the first (top) of the layers to
                     be examined for a projection, ie where the
                     search begins.
  \param prj         A structure to store the found projections
  \return            A bit mask representing the layers with available hits.

   This is your basic follow-your-nose permutation driven method of
   finding tracks. Three layers with candidate hits are examined. An
   attempt is made to find 3 hits which for a straight line simply by
   looping over all the hits in each layer. Each combination is tried
   with the 'best' projections being selected. If a projection is found,
   an attempt is made to extend the projection both upwards and downwards.
   The downwards direction is will be more successful. Since the seed
   projections are found by starting at the top (nearest the ACD) the
   hits in the layers above will most likely have already been assigned
   to tracks. Only if there is an inefficiency, will the extension upwards
   be successful.
                                                                          */
/* ---------------------------------------------------------------------- */
static          int findProjection (int               towerId,
                                    int                layers,
				    int               elayers,
                                    int           topLayerNum,
                                    int                 zfind,
                                    int               zextend,
                                    EDR_tkrLayer         *top,
                                    TFC_projection       *prj)
{
   int cnt    = 0;        /* Number of projections found           */

   const EDR_tkrCluster *topClusters = top[ 0].beg;
   const EDR_tkrCluster *midClusters = top[-1].beg;
   const EDR_tkrCluster *botClusters = top[-2].beg;

   int                    topMap = top[ 0].map;
   int                    midMap = top[-1].map;
   int                    botMap = top[-2].map;
   int                    topMsk = topMap;

//   _PRINT_TITLE (towerId);
   
   
   /*
    | Check for the special case of all layers having one and only one hit.
    | This happens rather frequently and the case can be highly optimized,
    | since no looping structures are needed. This routine is only called
    | when there is at least one hit in each of the relevant layers. Hence,
    | ORing the counts together as a check that each layer has one and only
    | hit works.
   */
#  ifdef PROJ_111
   if ((cntTop | cntMid | cntBot) == 2)
   {
       return findProjection111 (topLayerNum,
                                 zfind,
                                 topClusters,
                                 midClusters,
                                 botClusters,
                                 prj);
   }
#  endif  


   /*
    | Loop over the top 3 layers examining the outer 2 layers and attempting
    | to predict the hit in the middle layer.
   */
   do 
   {
       int topIdx;       
       int botMsk;
       int topStrip;
       
       
       topIdx   = FFS (topMsk);              /* Locate next available strip */
       topMsk   = FFS_eliminate (topMsk, topIdx); /* Eliminate it           */
       topStrip = topClusters[topIdx].strip; /* Get strip number            */
       botMsk   = botMap;                    /* Map of available bot strips */
       
       do
       {
           int        botIdx;
           int      botStrip;
           int    prediction;
           MatchVal       mv;
           
           
           botIdx      = FFS (botMsk);              /* Next available strip */
           botMsk      = FFS_eliminate (botMsk, botIdx);  /* Eliminate it   */
           botStrip    = botClusters[botIdx].strip; /* Get strip number     */
           prediction  = project (topStrip, botStrip, zfind);
           mv          = findMatchMid (FIND_TOLERANCE,
				       BIG_TOLERANCE,
				       prediction,
				       midMap,
				       midClusters,
				       topLayerNum,
				       topStrip,
				       botStrip,
				       zfind);

           
           /* Was a decent projection found */
           if (mv.si >= 0)
           {
               int midIdx   = mv.bf.idx;
               int midStrip = mv.bf.strip;

               /* Can it be improved */
               if (mv.bf.residual > 0 && topMsk)
	       {
		   int tolerance = abs(mv.bf.residual);

		   /* Always project one layer up */
		   prediction = projectDx (midStrip, 
					   midStrip - botStrip,
					   zextend & 0xffff);
		   mv         = findMatchTop (tolerance,
					      tolerance,
					      prediction,
					      topMsk,
					      topClusters,
					      topLayerNum,
					      midStrip,
					      botStrip,
					      zextend & 0xffff);
		   if (mv.si >= 0)
		   {
		       /* 
                        | Improved the seed. Question is now whether one 
                        | should make the original top hit eligible again.
                        | Technically, the only way this can be used in
                        | another combination is if there is a cross-over
                        | The assumption here is that there no cross-overs.
                        |
                        | Since the original top strip wasn't used, should
                        | put it back in the pile, but should eliminate the
                        | found one. 
		       */
 		       topMsk   = FFS_insert    (topMsk, topIdx);
		       topIdx   = mv.bf.idx;
                       topMsk   = FFS_eliminate (topMsk, topIdx);
		       topStrip = mv.bf.strip;
		   }
               }

	       //else if (mv.bf.residual < 0 && botMsk)
	       //{
	       //printf (" Looking to improve bot seed hit residual = %4d\n",
	       //	 mv.bf.residual);
               //}

                   
               /* Mark as used */
               topMap = FFS_eliminate (topMap, topIdx);
               botMap = FFS_eliminate (botMap, botIdx);
               midMap = FFS_eliminate (midMap, midIdx);

               
               storeProjection (prj,
                                topLayerNum,
                                topStrip,
                                midStrip,
                                botStrip);

               
               /* If have other layers hit, try extending the track */
               if (layers)
               {
                   layers &= extendProjectionUp  (prj,
                                                  elayers,
                                                  topLayerNum,
                                                  top,
                                                  topStrip,
                                                  midStrip,
                                                  zextend);
                                                 
                   layers &= extendProjectionDwn (prj,
                                                  layers,
                                                  topLayerNum - 2,
                                                  top - 2,
                                                  midStrip,
                                                  botStrip,
                                                  zextend);
               }
               prj++;
               cnt++;

               /* If exhausted the hits in any of the three layers, done */
               if ((topMap == 0) || (botMap == 0) || (midMap == 0)) goto EXIT;
           }
       }
       while (botMsk);
   }
   while (topMsk);

   
  EXIT:
   {
       /* If found any projections, store the marked hit maps */
       int msk = ~(1 << topLayerNum);
       if (cnt)
       {

           /*
            | Keep track of the map of unused hits.
            | If the number of unused hits in a layer goes to 0,
            | eliminate the layer from future pattern recognition usage.
            |
            | The top layer is always eliminated. Its had its chance
            | at being the seeding the recognition.
           */ 
           top[ 0].map = topMap;

           if ((top[-1].map = midMap) == 0) layers &= msk >> 1;
           if ((top[-2].map = botMap) == 0) layers &= msk >> 2;

           
           /* Note the number of projections found */
           layers |= (cnt << 18);
       }

       /* Always eliminate the top layer from future consideration */
       layers &= msk;
   }
   
   
   return layers;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \def   _PRINT_TKRFIND_BEG(_towerId)
  \brief  Prints a header line. This is used when debugging these routines
          and is only defined if the symbol \a DEBUG is defined.
  \param  _towerId The tower number being used search for tracks.
                                                                          *//*!
  \def   _PRINT_TKRFIND_END
          Prints a footer line. This is used when debugging these routines
          and is only defined if the symbol \a DEBUG is defined.
                                                                          */
/* ---------------------------------------------------------------------- */
#ifdef DEBUG

static const char Filler[] = "===============================================";

#define _PRINT_TKRFIND_BEG(_towerId)                                     \
   printf ("\n+%.*s Finding tracks for tower %1.1X %.*s+\n"              \
           "|%*c|\n",                                                    \
           (78-30)/2, Filler,                                            \
           towerId,                                                      \
           (78-30)/2, Filler,                                            \
           (78- 2)  , ' ')


#define _PRINT_TKRFIND_END                                               \
   printf ("\n"                                                          \
           "|%*c|\n"                                                     \
           "+%.*s DONE %.*s+\n\n",                                       \
           (78-2)  , ' ',                                                \
           (78-8)/2, Filler,                                             \
           (78-8)/2, Filler)

#else

#define _PRINT_TKRFIND_BEG(_towerId)
#define _PRINT_TKRFIND_END

#endif
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn     int TFC_projectionsSizeof (void)
  \brief  Returns the size of a projections results record 
  \return The size, in bytes, of a projections results record 

   This routine is provided to hide the implemenation of a TFC_projections
   record allowing a caller to determine the size so it can be allocated.
									  */
/* ---------------------------------------------------------------------- */
int FilterAlg_projectionsSizeof (void)
{
  return sizeof (TFC_projections);
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn int TFC_projectionTowerFind (TFC_projection        *prjs,
                                   const TFC_geometryTkr  *geo,
                                   EDR_tkrTower         *tower,
                                   unsigned int        xLayers,
                                   unsigned int        yLayers)
  \brief Driver routine which looks for X and Y projections in the
         specified tower.

  \param prjs    Structure to store any found projections
  \param geo     The geometry to be used when finding projections
  \param tower   Description of the hit strips in this tower
  \param xLayers A bit mask representing those X layers with hit strips
                 (MSB = layer closest to the ACD top plane.)
  \param yLayers A bit mask representing those Y layers with hit strips
                 (MSB = layer closest to the ACD top plane.)
  \return        A packed number with the upper 16 bits representing
                 the count of Y projections found and the lower 16
                 bits representing the count of X projections found.
                                                                          */
/* ---------------------------------------------------------------------- */
int FilterAlg_projectionTowerFind (TFC_projection        *prjs,
                             EDR_tkrTower         *tower,
                             unsigned int        xLayers,
                             unsigned int        yLayers)
{
   EDR_tkrLayer      *layerBeg;
   int                    xCnt;
   int                    yCnt;
   int                 towerId;
   unsigned int    zfindMaxMin;
   unsigned int  zextendMaxMin;
   unsigned int        eLayers;

  TFC_geoId      geo_id;
    const TFC_geometry     *geo;
    int geoPrint = 0;
   geo_id   = TFC_K_GEO_ID_DEFAULT;
    geo = TFC_geosLocate (TFC_Geos, -1, geo_id);
//   const TFC_geometryTkr   *geo = bgeo->tkr;
   
   /* Get the variables that apply to both the X and Y projection finders */
   towerId       = tower->id;      
   zfindMaxMin   = geo->tkr.zfindMaxMin;
   zextendMaxMin = geo->tkr.zextendMaxMin;
   printf("FilterAlg_projectionTowerFind: towerID %d zmx %d zext %d\n",
         towerId,zfindMaxMin,zextendMaxMin);

   
   _PRINT_TKRFIND_BEG(towerId);   
   // EDM_CODE (EDR_tkrTowerPrint (tower));
   
   
   /*-------------------------------------------------------------------*\
    |                                                                   | 
    |               X Projection finder                                 |
    |                                                                   |
   \*-------------------------------------------------------------------*/
   layerBeg = tower->layers;
   xCnt     = 0;
   eLayers  = xLayers;
   
   while (1)
   {
       unsigned int iss;
       int      sLayers;
       int     layerNum;
       int        zfind;
       int      zextend;

       /*
        | Find a seed layer, i.e. somewhere where there is a
        | 3-in-a-row
       */
       sLayers = xLayers & (xLayers << 1) & (xLayers << 2);
       if (sLayers == 0) break;

       
       /* Find the top layer of the first 3-in-a-row */
       layerNum = 31 - FFS (sLayers);

       if (layerNum&1)
       {
           zfind   =   zfindMaxMin & 0xffff;
           zextend = zextendMaxMin;
       }
       else
       {
           zfind   = (   zfindMaxMin >>    16) & 0xffff;
           zextend = ((zextendMaxMin & 0xffff) <<    16)
                   | ((zextendMaxMin >>    16) & 0xffff);

       }

       iss = findProjection (towerId,
                             xLayers,
			     eLayers,
                             layerNum,
                             zfind,
                             zextend,
                             layerBeg + layerNum,
                             &prjs[xCnt]);
       /*
        | If the return code is zero, no projections where found.
        | If non-zero, the lower 18 bits represent a bit mask
        | of the layers which no longer have any unused hits
       */
       if (iss)
       {
           xCnt    += iss >> 18;
           xLayers  = iss & 0x3ffff;
       }
   }


   /*-------------------------------------------------------------------*\
    |                                                                   | 
    |               Y Projection finder                                 |
    |                                                                   |
   \*-------------------------------------------------------------------*/
   layerBeg  = &tower->layers[18];
   yCnt      = xCnt;
   eLayers  = yLayers;

   while (yLayers)
   {
       unsigned int iss;
       int      sLayers;
       int     layerNum;
       int        zfind;
       int      zextend;

       /*
        | Find a seed layer, i.e. somewhere where there is a
        | 3-in-a-row
       */
       sLayers = yLayers & (yLayers << 1) & (yLayers << 2);
       if (sLayers == 0) break;

       
       /* Find the top layer of the first 3-in-a-row */
       layerNum = 31 - FFS (sLayers);

       if (layerNum&1)
       {
           zfind   =  (  zfindMaxMin >> 16) & 0xffff;
           zextend = ((zextendMaxMin & 0xffff) << 16)
                   | ((zextendMaxMin >> 16   ) & 0xffff);
       }
       else
       {
           zfind   =   zfindMaxMin & 0xffff;
           zextend = zextendMaxMin;

       }

  
       iss = findProjection (towerId | 0x80000000,
                             yLayers,
			     eLayers,
                             layerNum,
                             zfind,
                             zextend,
                             layerBeg + layerNum,
                             &prjs[yCnt]);
       
       /*
        | If the return code is zero, no projections where found.
        | If non-zero, the lower 18 bits represent a bit mask
        | of the layers which no longer have any unused hits
       */
       if (iss)
       {
           yCnt    += iss >> 18;
           yLayers  = iss & 0x3ffff;
       }

   }


   return ((yCnt - xCnt) << 16) | xCnt;
}
/* ---------------------------------------------------------------------- */













