/*------------------------------------------------------------------------
| CVS $Id: TFC_projectionTowerFind.c,v 1.1.1.1 2003/07/07 16:50:48 golpa Exp $
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  TFC_projectionTowerFind.c
   \brief Finds projections in the TKR
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */
#include <stdlib.h>

#include "ffs.h"
#include "DFC/TFC_towerRecord.h"
#include "DFC/TFC_projectionTowerFind.h"

#include "TFC_projectionDef.h"
#include "TFC_geometryDef.h"




/* ---------------------------------------------------------------------- *//*!

  \def    FIND_TOLERANCE
  \brief  The tolerance, in units of strip widths, to accept a middle strip.
          This tolerance is used when comparing an actual hit strip address
          to the predicted strip address when initially finding a seed
          projection.
                                                                          *//*!
  \def    EXTEND_1LAYER_TOLERANCE
  \brief  The tolerance, in units of strip widths, to accept a strip when
          extending a projection 1 layer.
                                                                          *//*!
  \def    EXTEND_2LAYER_TOLERANCE
  \brief  The tolerance, in units of strip widths, to accept a strip when
          extending a projection 2 layers.
                                                                          */
/* ---------------------------------------------------------------------- */
#define          FIND_TOLERANCE 32
#define EXTEND_1LAYER_TOLERANCE 32
#define EXTEND_2LAYER_TOLERANCE 64
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


    
typedef union _MatchVal
{
    int         si;           /*!< Value as a signed integer   */
    MatchVal_bf bf;           /*!< Value as a series of fields */
}
MatchVal;
/* ---------------------------------------------------------------------- */


    
static inline int             project (int                    top,
                                       int                    bot,
                                       int                     dz);

static inline int           projectDx (int                     x0,
                                       int                     dx,
                                       int                     dz);

static inline int          projectMid (int                    top,
                                       int                    bot,
                                       int                     dz);


static inline MatchVal   findMatchTop (int              tolerance,
                                       int             prediction,
                                       int                 topMsk,
                                       const TFC_strip *topStrips,
                                       int            topLayerNum,
                                       int               midStrip,
                                       int               botStrip,
                                       int                     dz);


static inline MatchVal   findMatchMid (int              tolerance,
                                       int             prediction,
                                       int                 midMsk,
                                       const TFC_strip *midStrips,
                                       int            topLayerNum,  
                                       int               topStrip,
                                       int               botStrip,
                                       int                     dz);
    
static inline MatchVal   findMatchBot (int              tolerance,
                                       int             prediction,
                                       int                 midMsk,
                                       const TFC_strip *midStrips,
                                       int            botLayerNum,
                                       int               topStrip,
                                       int               botStrip,
                                       int                     dz);
    
static inline void   storeProjection  (TFC_projection        *prj,
                                       int            topLayerNum,
                                       int                  paTop, 
                                       int                  paMid, 
                                       int                  paBot);
                                     

static inline int      findProjection (int                towerId,
                                       int                 layers,
                                       int               layerNum,
                                       int                  zfind,
                                       int            zseparation,
                                       TFC_towerLayer        *top,
                                       TFC_projection        *prj);

#if 0
static inline int   findProjection111 (int            topLayerNum,
                                       int                 deltaz,
                                       const TFC_strip  *stripTop,
                                       const TFC_strip  *stripMid,
                                       const TFC_strip  *stripBot,
                                       TFC_projection        *prj);
#endif

static inline int extendProjectionDwn (TFC_projection        *prj,
                                       int                 layers,
                                       int               layerNum,
                                       TFC_towerLayer        *nxt,
                                       int               topStrip,
                                       int               midStrip,
                                       int                zextend);

static inline int extendProjectionUp  (TFC_projection        *prj,
                                       int                 layers,
                                       int               layerNum,
                                       TFC_towerLayer        *nxt,
                                       int               midStrip,
                                       int               botStrip,
                                       int                zextend);

#ifdef __cplusplus
}
#endif


/* ---------------------------------------------------------------------- *//*!

  \def    ELIMINATE(_bitNumber)
  \brief  Creates a mask used to clears bit \a _bitNumber (MSB = 0)
  \return A mask with all but bit = \a _bitNumber set.

  This macro is used to clear a bit after it has been found by the FFS
  routine.
                                                                          */
/* ---------------------------------------------------------------------- */
#define ELIMINATE(_bitNumber) (~(0x80000000 >> (_bitNumber))) 
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \def    _DBG(_statement)
  \brief  A macro, defined to either, drop the C \a _statement or drop
          nothing. The macro is defined to drop the statement if the
          symbol \e DEBUG is defined, or do nothing if it is not defined.

   The obvious use of this macro is too drop debugging statements in
   the code when the symbol \e DEBUG is defined and to be unobtrusive,
   otherwise.
                                                                          *//*!
  \def    _PRINT_TITLE(_tower)
  \brief  Prints a header line if the symbol \a DEBUG is defined,
          otherwise, does nothing.
  \param _tower The tower number being used to find the projections        
                                                                          *//*!
  \def    _PRINT_CHK(_lay, _top, _mid, _bot, _zex, _pred, _res, _tol)
  \brief  Prints the status of checking a middle point when finding a
          seed projection if the symbol \a DEBUG is defined, otherwise
          does nothing.
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
   
                                                                          *//*!
  \def   _PRINT_EXTI(_msk)
  \brief  Prints a line introducing the beginning of looking for
          additional layers to extend an existing projection, if
          the symbol \a DEBUG is defined, otherwise it does nothing.
  \param  _msk  A bit mask of the layers which have hits in them and,
                thus, are possible candidate extension layers.
                
                                                                          *//*!
  \def   _PRINT_EXT(_lay, _top, _mid, _bot, _zex, _pred, _res, _tol)
  \brief  Prints the status of checking a point when extending a
          projection if the symbol \a DEBUG is defined, otherwise
          does nothing.
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
#ifdef DEBUG //THB disable this for now
# undef DEBUG
#endif

#ifdef DEBUG

#include <stdio.h>
#include "DFC/TFC_towerRecordPrint.h"
#include "DFC/TFC_projectionPrint.h"

#define _DBG(_statement)  _statement 

#define _PRINT_TITLE(_tower)                                                \
   printf ("\n"                                                             \
      "%1.1X%c OP Lyr  Top  Mid  Bot Zfactor Pred   Diff  Tol Status\n"     \
           " ---- --- ---- ---- ---- ------- ---- ------ ---- ------\n",    \
           _tower&0xf, (int)_tower < 0 ? 'y' : 'x')

#define _PRINT_CHK(_lay, _top, _mid, _bot, _zex, _pred, _res, _tol)        \
   printf (" Seed %3d %4d %4d %4d %7d %4d %6d %4d %s\n",                   \
            _lay, _top, _mid, _bot, _zex, _pred, _res, _tol,               \
            abs(_res) <= _tol ? "found" : "")

#define _PRINT_EXTI(_msk);                                                \
   printf ("  Ext %*c %8.8x\n", 43, ' ',  _msk)


#define _PRINT_EXT(_lay, _top, _mid,  _bot, _zex, _pred, _res, _tol)      \
   printf ("  Ext %3d %4d %4d %4d %7d %4d %6d %4d %s\n",                  \
            _lay, _top, _mid, _bot, _zex, _pred, _res, _tol,              \
            abs(_res) <= _tol ? "extended" : "")
   
#else

#define _DBG(_statement)

#define _PRINT_TITLE(_tower)
#define _PRINT_CHK(_lay, _top, _mid, _bot, _zex, _pred, _res, _tol)

#define _PRINT_EXTI(_msk)   
#define _PRINT_EXT(_lay, _top, _mid, _bot, _zex, _pred, _res, _tol)


#endif
/* ---------------------------------------------------------------------- */



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

  \fn  int projectMid (int top, int bot, int dz)
  \brief  Another projection function, used to predict a point in a skipped
          layer. 
  \top    The top strip address
  \bot    The bottom strip address
  \dz     The scaled distance from the top strip to the missing strip
  \return The predicted strip number in the missing layer.
                                                                          */
/* ---------------------------------------------------------------------- */
static int projectMid (int top, int bot, int dz)
{
  return  (bot * TFC_Z_EXTEND_SCALE_FACTOR + top * dz)
        / (dz             + TFC_Z_EXTEND_SCALE_FACTOR);
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn  MatchVal findMatchTop (int              tolerance,
                              int             prediction,
                              int                 topMsk,
                              const TFC_strip *topStrips,
                              int            topLayerNum,
                              int               midStrip,
                              int               botStrip,
                              int                     dz)
  \brief Finds the best match when extending a projection up.
  \param tolerance   The tolerance used to accept a match
  \param prediction  The predicted strip number
  \param topMsk      The mask of available hits in the top layer
  \param topStrips   The array of strip addresses in the top layer
  \param topLayerNum The top layer number
  \param midStrip    The middle strip number
  \param botStrip    The bottom strip number
  \param dz          A geometrical factor used when extending
                     the projection.
  \return            A packed status word, summarizing the
                     best match.
                                                                          */
/* ---------------------------------------------------------------------- */
static MatchVal findMatchTop (int              tolerance,
                              int             prediction,
                              int                 topMsk,
                              const TFC_strip *topStrips,
                              int            topLayerNum,
                              int               midStrip,
                              int               botStrip,
                              int                     dz)
{
   MatchVal mv;
   int bestResidual = 0x7fffffff;

   
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
       topMsk     &= ELIMINATE(topIdx);
       topStrip    = topStrips[topIdx];
       residual    = topStrip - prediction;
       absResidual = abs (residual);


       _PRINT_EXT(topLayerNum,
                  topStrip,
                  midStrip,
                  botStrip,
                  dz,
                  prediction,
                  residual,
                  tolerance);

       
       /* Is this a better fit */
       if (absResidual < bestResidual)
       {
           /* Yes, save its parameters */
           mv.bf.idx    = topIdx;
           mv.bf.strip  = topStrip;
           bestResidual = absResidual;
       }
       else
       {
           break;
       }
       
   }
   while (topMsk);

   return   (bestResidual <= tolerance)
          ? (mv.bf.residual = bestResidual,
             mv)
          : (mv.si = -1,
             mv);
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn  MatchVal findMatchBot (int              tolerance,
                              int             prediction,
                              int                 botMsk,
                              const TFC_strip *botStrips,
                              int            botLayerNum,
                              int               topStrip,
                              int               midStrip,
                              int                     dz)
  \brief Finds the best match when extending a projection down.
  \param tolerance   The tolerance used to accept a match
  \param prediction  The predicted strip number
  \param botMsk      The mask of available hits in the bottom layer
  \param botStrips   The array of strip addresses in the bottom layer
  \param botLayerNum The bottom layer number
  \param topStrip    The top    strip number
  \param midStrip    The middle strip number  
  \param dz          A geometrical factor used when extending
                     the projection.
  \return            A packed status word, summarizing the
                     best match.
                                                                          */
/* ---------------------------------------------------------------------- */
static MatchVal findMatchBot (int              tolerance,
                              int             prediction,
                              int                 botMsk,
                              const TFC_strip *botStrips,
                              int            botLayerNum,
                              int               topStrip,
                              int               midStrip,
                              int                     dz)
{
   MatchVal      mv;
   int bestResidual = 0x7fffffff;

   
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
       botMsk     &= ELIMINATE(botIdx);
       botStrip    = botStrips[botIdx];
       residual    = botStrip - prediction;
       absResidual = abs (residual);


       _PRINT_EXT(botLayerNum,
                  topStrip,
                  midStrip,
                  botStrip,
                  dz,
                  prediction,
                  residual,
                  tolerance);

       
       /* Is this a better fit */
       if (absResidual < bestResidual)
       {
           /* Yes, save its parameters */
           mv.bf.idx    = botIdx;
           mv.bf.strip  = botStrip;
           bestResidual = absResidual;
       }
       else
       {
           break;
       }
       
   }
   while (botMsk);

   return   (bestResidual <= tolerance)
          ? (mv.bf.residual = bestResidual,
             mv)
          : (mv.si = -1,
             mv);
}



/* ---------------------------------------------------------------------- *//*!

  \fn  MatchVal findMatchMid (int              tolerance,
                              int             prediction,
                              int                 midMsk,
                              const TFC_strip *midStrips,
                              int            topLayerNum,
                              int               topStrip,
                              int               botStrip,
                              int                     dz)
  \brief Finds the best match when projecting to a middle layer
  \param tolerance   The tolerance used to accept a match
  \param prediction  The predicted strip number
  \param midMsk      The mask of available hits in the middle layer
  \param midStrips   The array of strip addresses in the middle layer
  \param topLayerNum The top layer number
  \param topStrip    The top strip number
  \param midStrip    The bottom strip number  
  \param dz          A geometrical factor used when extending
                     the projection to the middle layer.
  \return            A packed status word, summarizing the
                     best match.
                                                                          */
/* ---------------------------------------------------------------------- */
static MatchVal findMatchMid (int              tolerance,
                              int             prediction,
                              int                 midMsk,
                              const TFC_strip *midStrips,
                              int            topLayerNum,
                              int               topStrip,
                              int               botStrip,
                              int                     dz)
{
   MatchVal mv;
   int bestResidual = 0x7fffffff;

   
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
       midIdx      = FFS      (midMsk);       
       midMsk     &= ELIMINATE(midIdx);
       midStrip    = midStrips[midIdx];
       residual    = midStrip - prediction;
       absResidual = abs (residual);

               
       _PRINT_CHK(topLayerNum,
                  topStrip,
                  midStrip,
                  botStrip,
                  dz,
                  prediction,
                  residual,
                  tolerance);

               
       /* Is this a better fit */
       if (absResidual < bestResidual)
       {
           /* Yes, save its parameters */
           mv.bf.idx    = midIdx;
           mv.bf.strip  = midStrip;
           bestResidual = absResidual;
       }
       else
       {
           break;
       }
       
   }
   while (midMsk);

   
   return   (bestResidual <= tolerance)
          ? (mv.bf.residual = bestResidual,
             mv)
          : (mv.si = -1,
             mv);
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
static inline void storeProjection (TFC_projection *prj,
                                    int     topLayerNum,
                                    int           paTop, 
                                    int           paMid, 
                                    int           paBot) 
{
   int topLayerNumM2 = topLayerNum - 2;
 
   prj->intercept = paTop;
   prj->slope     = paTop - paMid;
   prj->max       = topLayerNum;
   prj->min       = topLayerNumM2;
   prj->layers    = 0x7 << (topLayerNumM2);
   prj->hits[0]   = paTop;
   prj->hits[1]   = paMid;
   prj->hits[2]   = paBot;
   prj->nhits     = 3;
  
   return;
}
/* ---------------------------------------------------------------------- */



#if 0
static inline int findProjection111 (int        topLayerNum,
                                     int             deltaz,
                                     TFC_strip    *stripTop,
                                     TFC_strip    *stripMid,
                                     TFC_strip    *stripBot,
                                     TFC_projection    *prj)
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

    stripTop: Pointer to the hit strip number in the topmost layer

    stripMid: Pointer to the hit strip number in the middle  layer

    stripBot: Pointer to the hit strip number int the bottom layer

  RETURNS
  -------
  1 if a match was found, 0 if not. This is really the number of projections
  found, but in this special case, it can only be 1 or 0.
*/  
{
   int prediction;
   int   residual;

   
   int paTop = *stripTop;
   int paMid = *stripMid;
   int paBot = *stripBot;
  

   /* If any of the hits are already used, no track possible */
   if (isUsed (paTop | paMid | paBot) ) return 0;

   
   prediction = project (paTop, paBot, deltaz);
   residual   = paMid - prediction;
  _PRINT_L2ALIGN_CHK (paTop, paMid, paBot, prediction, residual);
  
  
   if (abs(residual) < ALIGNMENT_TOLERANCE)
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

  \fn int extendProjectionDwn (TFC_projection   *prj,
                               int            layers,
                               int          layerNum,
                               TFC_towerLayer   *bot,
                               int          topStrip,
                               int          midStrip,
                               int           zextend)
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
static inline int extendProjectionDwn (TFC_projection   *prj,
                                       int            layers,
                                       int          layerNum,
                                       TFC_towerLayer   *bot,
                                       int          topStrip,
                                       int          midStrip,
                                       int           zextend)
{
   TFC_strip         *hits;
   unsigned int prj_layers;
   int             zdelta1;
   int             zdelta2;
   int           retLayers;
   int            minLayer;
   int           tolerance;   

   
   /* Remove the layers above this one, we are searching down  */
   retLayers = layers;
   layers  <<= (32 - layerNum);   

   
   /* If either of the next two layers are not hit, just return */
   if ((layers & 0xc0000000) == 0) return retLayers;

   
   minLayer   = layerNum;   
   prj_layers = prj->layers;
   hits       = prj->hits + prj->nhits;
   zdelta1    = zextend & 0xffff;
   zdelta2    = zextend >>    16;
   tolerance  = 0;

   
  _PRINT_EXTI(layers);

  
  /* Loop over the layers until miss matching 2 layers or when out of layers */
  while (1)
  {
      int             prediction;
      int                   skip;
      int                     dx;
      int                 botMap;
      const TFC_strip *botStrips;
      int               botStrip;
      MatchVal                mv;


      /* Always project down one layer */
      dx         = midStrip - topStrip;
      prediction = projectDx (midStrip, dx, zdelta1);
      

      /* Guaranteed that one of the next two layers are hit */
      if (layers < 0) 
      {
          /* Looking down one layer */
          skip       = 1;
          tolerance += EXTEND_1LAYER_TOLERANCE;
      }
      else
      {
          /* Looking down two layers. */
          skip        = 2;
          tolerance   = EXTEND_2LAYER_TOLERANCE;
          prediction += dx;
      }


      /* Adjust the current layer to be searched */
      layers   <<= skip;
      bot       -= skip;
      layerNum  -= skip;
      botMap     = bot->map;
      botStrips  = bot->beg;
      
      /*
       | This loop scans until a match is found or 2 unmatched layers
       | have been encountered.
      */
      while (1)
      {
          mv = findMatchBot (tolerance,
                             prediction,
                             botMap,
                             botStrips,
                             layerNum,
                             topStrip,
                             midStrip,
                             zdelta1 + (skip - 1) * TFC_Z_EXTEND_SCALE_FACTOR);
          
          /* Check if found an acceptable match */          
          if (mv.si >= 0) break;


          /*
           | Have already skipped at layer or
           | have looked for a hit but failed to find a match,
           | and the next layer is not hit, that's it.
          */
          if (skip == 2 || layers >= 0) goto EXIT;

          
          /*
           | Adjust the search parameters to be compatiable with looking
           | in the next layer down
          */
          skip        = 2;
          tolerance  += EXTEND_2LAYER_TOLERANCE;
          prediction += dx;
          layers    <<= 1;
          bot        -= 1;
          layerNum   -= 1;
          botMap      = bot->map;
          botStrips   = bot->beg;          
      }
          

      /*
       | Found a match
       | Mark the hit as used. The hit is not stored until it
       | has been determined whether one or two layers were skipped.
      */
      botMap     &= ELIMINATE (mv.bf.idx);
      botStrip    = mv.bf.strip;
      minLayer    = layerNum;
      prj_layers |= (1 << layerNum);
              

      /* If no unused hits left, eliminate this layer */
      if (botMap == 0) retLayers &= ~(1 << layerNum);

      
      /* Swap the extension factors if did not skip a layer */
      if (skip == 1)
      {
          /* ... swap the extension factor */
          int tmp;

          *hits++ = botStrip;
          
          /* If neither of the next two layers hit, that's it */
          if ((layers & 0xc0000000) == 0) break;
          
          tmp       = zdelta1;
          zdelta1   = zdelta2;
          zdelta2   = tmp;
          topStrip  = midStrip;
          midStrip  = botStrip;
          tolerance = 0;

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
           | Even forgiving that, the projection should be done using
           | the middle layer projection technique. This would avoid
           | an unneccessary divide.
          */
          topStrip  = projectMid (midStrip, botStrip, zdelta1);
         *hits++    = topStrip;
         *hits++    = botStrip;
         
         /* If neither of the next two layers hit, that's it */
          if ((layers & 0xc0000000) == 0) break;

          midStrip  = botStrip;
          tolerance = mv.bf.residual / 2;
      }
  }
  

  EXIT:
  {
      /* Count the number of hits, add the layer mask */
      prj->nhits  = hits - prj->hits;
      prj->min    = minLayer;
      prj->layers = prj_layers;
      return retLayers;
  }
  

}
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \fn int extendProjectionUp (TFC_projection   *prj,
                              int            layers,
                              int          layerNum,
                              TFC_towerLayer   *bot,
                              int          topStrip,
                              int          midStrip,
                              int           zextend)
  \brief Attempts to extend a projection up through the next hit layers
  
  \param       prj The projection to be extended.
  \param    layers A bit mask of the hit layers. The next layer is in
                   the MSB of this mask.
  \param  layerNum The layer number of layer to be searched.                   
  \param       top Pointer to the next layer structure, containing the
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
static inline int extendProjectionUp  (TFC_projection   *prj,
                                       int            layers,
                                       int          layerNum,
                                       TFC_towerLayer   *top,
                                       int          midStrip,
                                       int          botStrip,
                                       int           zextend)
{
   TFC_strip         *hits;
   unsigned int prj_layers;
   int             zdelta1;
   int             zdelta2;
   int           retLayers;
   int            minLayer;
   int           tolerance;   

   
   /* Remove the layers above this one, we are searching down  */
   retLayers = layers;
   layers  >>= layerNum + 1;   

   
   /* If either of the next two layers are not hit, just return */
   if ((layers & 0x3) == 0) return retLayers;

   
   minLayer   = layerNum;   
   prj_layers = prj->layers;
   hits       = prj->hits + prj->nhits;
   zdelta1    = zextend & 0xffff;
   zdelta2    = zextend >>    16;
   tolerance  = 0;

   
  _PRINT_EXTI(layers);

  
  /* Loop over the layers until miss matching 2 layers or when out of layers */
  while (1)
  {
      int             prediction;
      int                   skip;
      int                     dx;
      int                 topMap;
      const TFC_strip *topStrips;
      int               topStrip;
      MatchVal                mv;


      /* Always project one layer up */
      dx         = midStrip - botStrip;
      prediction = projectDx (midStrip, dx, zdelta1);
      

      /* Guaranteed that one of the next two layers are hit */
      if ((layers&1)) 
      {
          /* Looking down one layer */
          skip       = 1;
          tolerance += EXTEND_1LAYER_TOLERANCE;
      }
      else
      {
          /* Looking down two layers. */
          skip        = 2;
          tolerance   = EXTEND_2LAYER_TOLERANCE;
          prediction += dx;
      }


      /* Adjust the current layer to be searched */
      layers   >>= skip;
      top       += skip;
      layerNum  += skip;
      topMap     = top->map;
      topStrips  = top->beg;
      
      /*
       | This loop scans until a match is found or 2 unmatched layers
       | have been encountered.
      */
      while (1)
      {
          mv = findMatchTop (tolerance,
                             prediction,
                             topMap,
                             topStrips,
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
          if (skip == 2 || ((layers >>= 1) & 1) == 0) goto EXIT;

          
          /*
           | Adjust the search parameters to be compatiable with looking
           | in the next layer down
          */
          skip        = 2;
          tolerance  += EXTEND_2LAYER_TOLERANCE;
          prediction += dx;
          top        -= 1;
          layerNum   -= 1;
          topMap      = top->map;
          topStrips   = top->beg;
          
      }
          

      /*
       | Found a match
       | Mark the hit as used and
       | store the results
      */
      topMap     &= ELIMINATE (mv.bf.idx);
      topStrip    = mv.bf.strip;
      minLayer    = layerNum;
     *hits++      = topStrip;
      prj_layers |= (1 << layerNum);
              

      /* If no unused hits left, eliminate this layer */
      if (topMap == 0) retLayers &= ~(1 << layerNum);

      
      /* If neither of the next two layers hit, that's it */
      if ((layers & 0x3) == 0) break;

      
      /* Swap the extension factors if did not skip a layer */
      if (skip == 1)
      {
          /* ... swap the extension factor */
          int tmp;
          tmp       = zdelta1;
          zdelta1   = zdelta2;
          zdelta2   = tmp;
          topStrip  = midStrip;
          midStrip  = botStrip;
          tolerance = 0;
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
          tolerance = mv.bf.residual / 2;
      }
  }
  

  EXIT:
  {
      /* Count the number of hits, add the layer mask */
      //prj->nhits  = hits - prj->hits;
      //prj->min    = minLayer;
      //prj->layers = prj_layers;
      return retLayers;
  }
  

}
/* ---------------------------------------------------------------------- */


  

/* ---------------------------------------------------------------------- *//*!

  \fn int findProjection (int               towerId,
                          int                layers,
                          int           topLayerNum,
                          int                 zfind,
                          int               zextend,
                          TFC_towerLayer       *top,
                          TFC_projection       *prj)
  \brief Looks for projection in the specified tower starting at
         \a topLayerNum.
  \param towerId     The id of the tower being searched.
  \param layers      A bit mask of the layers with available hits.
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
static inline int findProjection (int               towerId,
                                  int                layers,
                                  int           topLayerNum,
                                  int                 zfind,
                                  int               zextend,
                                  TFC_towerLayer       *top,
                                  TFC_projection       *prj)
{
   int cnt    = 0;        /* Number of projections found           */

   const TFC_strip *topStrips = top[ 0].beg;
   const TFC_strip *midStrips = top[-1].beg;
   const TFC_strip *botStrips = top[-2].beg;

   int                    topMap = top[ 0].map;
   int                    midMap = top[-1].map;
   int                    botMap = top[-2].map;
   int                    topMsk = topMap;

   _PRINT_TITLE (towerId);
   
   
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
                                 topStrips,
                                 midStrips,
                                 botStrips,
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
       topMsk  &= ELIMINATE(topIdx);         /* Eliminate it                */
       topStrip = topStrips[topIdx];         /* Get strip number            */
       botMsk   = botMap;                    /* Map of available bot strips */
       
       do
       {
           int        botIdx;
           int      botStrip;
           int    prediction;
           MatchVal       mv;
           
           
           botIdx       = FFS (botMsk);            /* Next available strip */
           botMsk      &= ELIMINATE(botIdx);       /* Eliminate it         */
           botStrip     = botStrips[botIdx];       /* Get strip number     */
           prediction   = project (topStrip, botStrip, zfind);
           mv           = findMatchMid (FIND_TOLERANCE,
                                        prediction,
                                        midMap,
                                        midStrips,
                                        topLayerNum,
                                        topStrip,
                                        botStrip,
                                        zfind);

           
           /* Was a decent projection found */
           if (mv.si >= 0)
           {
               int midIdx   = mv.bf.idx;
               int midStrip = mv.bf.strip;
               
                   
               /* Mark as used */
               topMap &= ELIMINATE (topIdx);
               botMap &= ELIMINATE (botIdx);
               midMap &= ELIMINATE (midIdx);

               
               storeProjection (prj,
                                topLayerNum,
                                topStrip,
                                midStrip,
                                botStrip);

               
               /* If have other layers hit, try extending the track */
               if (layers)
               {
                   layers = extendProjectionUp  (prj,
                                                 layers,
                                                 topLayerNum,
                                                 top,
                                                 topStrip,
                                                 midStrip,
                                                 zextend);
                                                 
                   layers = extendProjectionDwn (prj,
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

  \fn int TFC_projectionTowerFind (struct _TFC_projections       *prjs,
                                   const struct _TFC_geometryTkr  *geo,
                                   struct _TFC_towerRecord      *tower,
                                   unsigned int                xLayers,
                                   unsigned int                yLayers)
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
int TFC_projectionTowerFind (struct _TFC_projections       *prjs,
                             const struct _TFC_geometryTkr  *geo,
                             struct _TFC_towerRecord      *tower,
                             unsigned int                xLayers,
                             unsigned int                yLayers)
{
   TFC_towerLayer    *layerBeg;
   TFC_projection         *prj;
   int                    xCnt;
   int                    yCnt;
   int                 towerId;
   unsigned int    zfindMaxMin;
   unsigned int  zextendMaxMin;

   

   
   /* Get the variables that apply to both the X and Y projection finders */
   prj           = prjs->prjs;
   towerId       = tower->id;      
   zfindMaxMin   = geo->zfindMaxMin;
   zextendMaxMin = geo->zextendMaxMin;

   
   _PRINT_TKRFIND_BEG(towerId);   
   // _DBG (TFC_towerRecordPrint (tower));
   
   
   /*-------------------------------------------------------------------*\
    |                                                                   | 
    |               X Projection finder                                 |
    |                                                                   |
   \*-------------------------------------------------------------------*/
   layerBeg = tower->layers;
   xCnt     = 0;
   
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
                             layerNum,
                             zfind,
                             zextend,
                             layerBeg + layerNum,
                             &prj[xCnt]);
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
                             layerNum,
                             zfind,
                             zextend,
                             layerBeg + layerNum,
                             &prj[yCnt]);
       
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

   prjs->curCnt = yCnt;
   prjs->xy[0]  = xCnt;
   prjs->xy[1]  = yCnt -= xCnt;

   // _DBG (TFC_projectionsPrint (prjs, towerId));
   _PRINT_TKRFIND_END;
   
   return (yCnt << 16) | xCnt;
}
/* ---------------------------------------------------------------------- */













