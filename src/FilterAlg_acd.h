#ifndef FilterAlg_ACD_H
#define FilterAlg_ACD_H






/// ----------------------------------------------------------------------

   ///@file   FilterAlg_acd.h
   ///@brief  Interface to TKR->ACD projection and matching routines
   ///@author JJRussell - russell@slac.stanford.edu
   ///@author David Wren - modified JJ's TFC_acd.h to get this


/// ---------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif

struct _TFC_projection;
struct _TFC_geometry;



extern int FilterAlg_acdProject (const OnboardFilterTds::projections        *prj,
						     int prjstart,
                             int                           xCnt,
                             int                           yCnt,
                             const struct _TFC_geometry    *geo,
                             int                        towerId,
                             int                       dispatch,
                             unsigned int                 acd_x,
                             unsigned int                 acd_y,
                             unsigned int                 acd_z);

extern int FilterAlg_acdProjectTemplate (unsigned int     acd_x,
                                         unsigned int     acd_y,
                                         unsigned int     acd_z);

#ifdef __cplusplus
}
#endif



#endif


#include "TFC_projectionDef.h"
#include "TFC_geometryDef.h"
#include "OnboardFilter/FilterStatus.h"

using namespace OnboardFilterTds;

/// ---------------------------------------------------------------------
#ifndef CMX_DOXYGEN
/// ---------------------------------------------------------------------

/// ---------------------------------------------------------------------
#endif
/// ---------------------------------------------------------------------

static unsigned int findAcdSideMask   (int                        z,
				                       const short int      *zSides);

static unsigned int findAcdTopMask    (int                      acdTop,
                                       unsigned int             _template,
				                       int                      amount,
				                       const short int          *edges);

static __inline int projectXYtoAcdTop (const OnboardFilterTds::projections             *prj,
				                      int                    prjstart,
									  int                      offset,
				                      int                    _template,
				                      int                 shiftAmount,
				                      const unsigned short int  *toAcd,
				                      const short int          *edges);

static __inline int projectXYMtoAcd   (const OnboardFilterTds::projections             *prj,
				                       int                    prjstart,
									   int                   zposition,
				                       int                  toAcdPlane,
				                       int                      deltaZ,
				                       const short int         *zSides);

static __inline int projectXYPtoAcd   (const OnboardFilterTds::projections           *prj,
				                       int prjstart,
									   int                   zposition,
				                       int                   towerEdge,
				                       int                  toAcdPlane,
				                       int                      deltaZ,
				                       const short int         *zSides);

static __inline int projectXYStoAcd   (int                   zposition,
				                       int                  toAcdPlane,
				                       int                      deltaZ,
				                       int                     deltaXY,
				                       const short int         *zSides);


static         int prjAcd_Top         (const OnboardFilterTds::projections             *prj,
				                       int                    prjstart,
					                   int                        xCnt,
				                       int                        yCnt,
				                       const TFC_geometry         *geo,
				                       int                     towerId);

static         int prjAcd_XM          (const OnboardFilterTds::projections             *prj,
				                       int prjstart,
					                   int                        xCnt,
				                       int                        yCnt,
				                       const TFC_geometry         *geo,
				                       int                     towerId);

static         int prjAcd_XP          (const OnboardFilterTds::projections             *prj,
				                       int prjstart,
					                   int                        xCnt,
				                       int                        yCnt,
				                       const TFC_geometry         *geo,
				                       int                     towerId);


static         int prjAcd_YM          (const OnboardFilterTds::projections            *prj,
				                       int prjstart,
				                       int                        xCnt,
				                       int                        yCnt,
				                       const TFC_geometry         *geo,
				                       int                     towerId);

static         int prjAcd_YP          (const OnboardFilterTds::projections             *prj,
				                      int prjstart,
					                  int                        xCnt,
				                      int                        yCnt,
				                      const TFC_geometry         *geo,
				                      int                     towerId);

static         int prjAcd_XMS         (const OnboardFilterTds::projections             *prj,
				                       int prjstart,
			                           int                        xCnt,
				                       int                        yCnt,
				                       const TFC_geometry         *geo);

static         int prjAcd_XPS         (const OnboardFilterTds::projections             *prj,
				                       int prjstart,
					                   int                        xCnt,
				                       int                        yCnt,
				                       const TFC_geometry         *geo);

static         int prjAcd_YMS         (const OnboardFilterTds::projections             *prj,
				                       int prjstart,
					                   int                        xCnt,
				                       int                        yCnt,
				                       const TFC_geometry         *geo);

static         int prjAcd_YPS         (const OnboardFilterTds::projections            *prj,
				                       int prjstart,
					                   int                        xCnt,
				                       int                        yCnt,
				                       const TFC_geometry         *geo);



/// ---------------------------------------------------------------------
#ifndef CMX_DOXYGEN  // These are internal only definitions
/// ---------------------------------------------------------------------

///
/// | ACD_X_TEMPLATE marks off the possible ACD top tile hits for X row 0
/// | ACD_Y_TEMPLATE masks off the possible ACD top tile hits for Y row 0
/// |
/// | The templates for the other rows can be generated by left shifting the
/// | X template by 1 and the Y template by 5.
///
#define _ACD_X_TEMPLATE    (1 << (4*5)) | (1 << (3*5)) | (1 << (2*5)) \
                        | (1 << (1*5)) | (1 << (0*5))
#define _ACD_X_SHIFT_AMOUNT 1
#define _ACD_Y_TEMPLATE    (0x1f)
#define _ACD_Y_SHIFT_AMOUNT 5

/// ---------------------------------------------------------------------
#endif
/// ---------------------------------------------------------------------


/** ---------------------------------------------------------------------
 *
 *@enum  _ACD_V_prj
 *@brief Bit offset used to construct the ACD dispatch mask
 *
 *@typedef ACD_V_prj
 *@brief   Typedef for enum \e _ACD_V_prj
 *
 * See ACD_M_prj for further details
 *
/** --------------------------------------------------------------------- */
typedef enum _ACD_V_prj
{
  ACD_V_PRJ_XM  =  0,  //!< Project to XM  plane
  ACD_V_PRJ_XP  =  1,  //!< Project to XP  plane
  ACD_V_PRJ_YM  =  2,  //!< Project to YM  plane
  ACD_V_PRJ_YP  =  3,  //!< Project to YP  plane
  ACD_V_PRJ_TOP =  4,  //!< Project to TOP plane
  ACD_V_PRJ_XMS =  5,  //!< Project ot XM  plane after skipping a tower
  ACD_V_PRJ_XPS =  6,  //!< Project ot XP  plane after skipping a tower
  ACD_V_PRJ_YMS =  7,  //!< Project ot YM  plane after skipping a tower
  ACD_V_PRJ_YPS =  8   //!< Project ot YP  plane after skipping a tower
}
ACD_V_prj;
/** --------------------------------------------------------------------- */



/** ---------------------------------------------------------------------
*
* @enum  _ACD_M_prj
* @brief Bit mask used to construct the ACD dispatch mask
*
* @typedef ACD_M_prj
* @brief   Typedef for enum \e _ACD_M_prj
*
*  For a given set of struck ACD tiles, one can construct a mask of
*  which planes should ever be checked. When a track projection is found,
*  the projection type (X or Y) and tower can be used to further limit
*  the scope of the check.
*
*  The obvious things appear here, check X-,X+, Y-,Y+ and the TOP planes.
*  There also exists an additional set of masks used to extend the
*  search when the projection is very shallow. In these cases, it is
*  possible that only 2 layers where hit in the tower that is
*  juxtaposition to the struck ACD tiles. It is very possible that
*  the projection was not successfully extrapolated into the that tower
*  and the additional hits where missed. To cover this case, projections
*  with very shallow slopes are allowed to be projected all the way
*  through a tower to an ACD side plane. These are the ACD_M_PRJ_xxS
*  enumerations.
*
/** --------------------------------------------------------------------- */
typedef enum _ACD_M_prj
{
   ACD_M_PRJ_XM  = (1 << ACD_V_PRJ_XM ), //!< Project to XM  plane
   ACD_M_PRJ_XP  = (1 << ACD_V_PRJ_XP ), //!< Project to XP  plane
   ACD_M_PRJ_YM  = (1 << ACD_V_PRJ_YM ), //!< Project to YM  plane
   ACD_M_PRJ_YP  = (1 << ACD_V_PRJ_YP ), //!< Project to YP  plane
   ACD_M_PRJ_TOP = (1 << ACD_V_PRJ_TOP), //!< Project to TOP plane
   ACD_M_PRJ_XMS = (1 << ACD_V_PRJ_XMS), //!< Project to XM  plane (skip)
   ACD_M_PRJ_XPS = (1 << ACD_V_PRJ_XPS), //!< Project to XP  plane (skip)
   ACD_M_PRJ_YMS = (1 << ACD_V_PRJ_YMS), //!< Project to YM  plane (skip)
   ACD_M_PRJ_YPS = (1 << ACD_V_PRJ_YPS)  //!< Project to YP  plane (skip)
}
ACD_M_prj;
//! ---------------------------------------------------------------------



/** ----------------------------------------------------------------------
*
*  @struct _AcdDispatchCtl
*  @brief   Provides a set of 16 masks, one per tower, indicating which of
*           the 5 ACD planes this tower interfaces to.
*
*  @typedef AcdDispatchCtl
*  @brief   Typedef for struct \e _AcdDispatchCtl
*
* ---------------------------------------------------------------------- */
typedef struct _AcdDispatchCtl
{
    unsigned short int masks[16];  //< Masks, by tower, of which ACD
    		 		               //planes this tower interfaces to
}
AcdDispatchCtl;
//! ----------------------------------------------------------------------


/** ----------------------------------------------------------------------
*
* @var   AcdDispatchCtl AcdDispatch
* @brief Static, readonly structure indicating which ACD planes each
*       tower interfaces with.
*
* ---------------------------------------------------------------------- */
static const AcdDispatchCtl AcdDispatch =
{
   {  /* Controls which projections are permitted for which towers */
      ACD_M_PRJ_TOP | ACD_M_PRJ_YM  | ACD_M_PRJ_XM,     /* Tower 0 */
      ACD_M_PRJ_TOP | ACD_M_PRJ_YM  | ACD_M_PRJ_XMS,    /* Tower 1 */
      ACD_M_PRJ_TOP | ACD_M_PRJ_YM  | ACD_M_PRJ_XPS,    /* Tower 2 */
      ACD_M_PRJ_TOP | ACD_M_PRJ_YM  | ACD_M_PRJ_XP,     /* Tower 3 */

      ACD_M_PRJ_TOP | ACD_M_PRJ_XM  | ACD_M_PRJ_YMS,    /* Tower 4 */
      ACD_M_PRJ_TOP | ACD_M_PRJ_XMS | ACD_M_PRJ_YMS,    /* Tower 5 */
      ACD_M_PRJ_TOP | ACD_M_PRJ_XPS | ACD_M_PRJ_YMS,    /* Tower 6 */
      ACD_M_PRJ_TOP | ACD_M_PRJ_XP  | ACD_M_PRJ_YMS,    /* Tower 7 */

      ACD_M_PRJ_TOP | ACD_M_PRJ_XM  | ACD_M_PRJ_YPS,    /* Tower 8 */
      ACD_M_PRJ_TOP | ACD_M_PRJ_XMS | ACD_M_PRJ_YPS,    /* Tower 9 */
      ACD_M_PRJ_TOP | ACD_M_PRJ_XPS | ACD_M_PRJ_YPS,    /* Tower a */
      ACD_M_PRJ_TOP | ACD_M_PRJ_XP  | ACD_M_PRJ_YPS,    /* Tower b */

      ACD_M_PRJ_TOP | ACD_M_PRJ_YP  | ACD_M_PRJ_XM,     /* Tower c */
      ACD_M_PRJ_TOP | ACD_M_PRJ_YP  | ACD_M_PRJ_XMS,    /* Tower d */
      ACD_M_PRJ_TOP | ACD_M_PRJ_YP  | ACD_M_PRJ_XPS,    /* Tower e */
      ACD_M_PRJ_TOP | ACD_M_PRJ_YP  | ACD_M_PRJ_XP,     /* Tower f */
   }
};
/* ---------------------------------------------------------------------- */


/** ----------------------------------------------------------------------
*
* @fn        unsigned int findAcdSideMask (int                   z,
*                                           const short int *zSides)
* @brief     Translates a Z position into a mask which represents the row
*            of ACD tiles associated with that Z
*
* @param z      The z position in units of TFC_Z_ABS_SCALE_FACTOR
* @param zSides The z positions of the side tiles
* @return       A mask representing the ACD associated with \a z.
*
* ---------------------------------------------------------------------- */
static unsigned int findAcdSideMask (int                   z,
                                     const short int *zSides)
{
   if (z >= zSides[0]) return 0;
   if (z >= zSides[1]) return (0x1f << 0*5);
   if (z >= zSides[2]) return (0x1f << 1*5);
   if (z >= zSides[3]) return (0x1f << 2*5);
   if (z >= zSides[4]) return (0x1f << 3*5);

   return 0;
}
/** ---------------------------------------------------------------------- */


/** ----------------------------------------------------------------------
*
* @fn        unsigned int findAcdTopMask (int                  acdTop,
*                                         unsigned int       _template,
*                                         int                  amount,
*                                         const short int      *edges)
* @brief          Translates a X/Y position into a mask which represents
*                 the row or column of ACD top tiles associated with that
*                 X/Y position.
* @param acdTop   The X/Y position a the Z of the ACD top face of tiles.
* @param template A template mask that is valid for row or column 0
* @param amount   The amount to shift the template mask if the row or
*                 column is not 0. This will be 1 for X (columns) and
*                 5 for Y (rows).
* @param edges    The edges of the tiles in this row/col
* @return         A mask representing the ACD tiles associated with
*                 \a acdTop.
*
* ---------------------------------------------------------------------- */
static unsigned int findAcdTopMask (int                  acdTop,
                                    unsigned int       _template,
                                    int                  amount,
                                    const short int      *edges)
{
   int mask = 0;

   if (acdTop >= edges[0])
   {
       mask = _template;

       if (acdTop >= edges[1])
       {
           mask <<= amount;
           if (acdTop >= edges[2])
           {
               mask <<= amount;
               if (acdTop >= edges[3])
               {
                  mask <<= amount;
                  if (acdTop >= edges[4])
                  {
                      mask <<= amount;
                      if (acdTop > edges[5])
                          mask = 0;
                  }
               }
           }
       }
   }

   return mask;

}
/**--------------------------------------------------------------------- */


/** ----------------------------------------------------------------------
*
* @fn  int projectXYtoAcdTop (const OnboardFilterTds::projections    *prj,
*                             int prjstart,
*						      int             offset,
*                             int           _template,
*                             int        shiftAmount,
*                             const unsigned short int *toAcd,
*                             const short int *edges)
*
* @brief Projects an X/Y projects to the top face of the ACD and computes
*        the mask of which possible ACD tiles.
*
* @param prj         The projection
* @param towerId     The tower number. (Used only for diagnostic messages.)
* @param offset      The offset of this tower's local coordinate system from
*                    the absolute coordinate system.
* @param width       UNUSED
* @param template    The template for the ACD mask. This template is valid
*                    if the row/col is 0.
* @param shiftAmount The shift amount to apply to the template mask
*                    according to the actual row or col.
* @param toAcd       Array of projection distances from a layer to the top
*                    planes of the ACD
* @param edges       Array giving the edges of the tiles. These must be
*                    ordered from the most negative to most positive edge.
*
* ---------------------------------------------------------------------- */
static __inline int projectXYtoAcdTop (const OnboardFilterTds::projections      *prj,
				       int prjstart,
					   int               offset,
				       int             _template,
				       int          shiftAmount,
				       const unsigned short int   *toAcd,
				       const short int   *edges)
{
    int topLayerNum;
    int  acdTopMask;

    acdTopMask  = 0;
    topLayerNum = prj->prjs[prjstart].max;


    /*
     | Do the projection if the track starts in a layer close to the ACD
     | top face.
    */
    if (topLayerNum  >= 8)
    {
        int    acdPrj;
        int    acdTop;
        int     slope;
        int intercept;

        /*
         | 1. Get the distance to project to the ACD top tiles
         | 2. Do the projection
         | 3. Translate the projected coordinate into a row or col of tiles
         | 4. Keep track of the accumulated tiles
        */
        slope     = prj->prjs[prjstart].hits[0] - prj->prjs[prjstart].hits[1];
        intercept = prj->prjs[prjstart].hits[0];
        acdPrj    = toAcd[topLayerNum];
        acdTop    = intercept + offset
                  + (acdPrj*slope) / TFC_Z_ACD_TOP_TO_TKR_SCALE_FACTOR;

        acdTopMask  = findAcdTopMask (acdTop, _template, shiftAmount, edges);

    }

    return acdTopMask;
}
/* ---------------------------------------------------------------------- */


/**----------------------------------------------------------------------
*
* @fn    int projectXYMtoAcd (const OnboardFilterTds::projections     *prj,
*                             int prjstart,
*						  int           zposition,
*                             int          toAcdPlane,
*                             int              deltaZ,
*                             const short int *zSides)
*
*
* @brief Projects an X or Y track projection to the X-/Y- face
* @param prj        The track projection
* @param zposition  Starting z position
* @param toAcdPlane Distance from the strip 0 of the tracker to the ACD plane
*                   in units of the strip pitch.
* @param deltaZ     Distance between the top and bottom hit planes
* @param zSides     The z positions of the side tiles
* @param towerId    The tower number. This is currently only used in
*                   diagnostic messages.
*
*  There is a parallel routine routine \e projectXMPtoAcd for projecting
*  off the right/high side of a tower. The calling convention is not
*  the same because the left/lower edge of the tower is, by definition,
*  0, so the check for leaving a tower on the left/lower side is a simple
*  check against 0. For the right/high side, the check is against the
*  width of the tower (in strips).
*
/**---------------------------------------------------------------------- */
static __inline int projectXYMtoAcd (const OnboardFilterTds::projections     *prj,
				     int prjstart,
					 int           zposition,
				     int          toAcdPlane,
				     int              deltaZ,
				     const short int *zSides)
{
   int  mask;
   int slope;

   mask  = 0;
   slope = prj->prjs[prjstart].hits[0] - prj->prjs[prjstart].hits[1];

   /*
    | The slope of the track must be negative to consider this projection
    | for exiting the left/lower edge of the tower.
   */
   if (slope < 0)
   {
       int   topHit;
       int  deltaXY;
       int    check;


       /*
        | Find distance between the top hit (hit 0) and the 2nd hit.
        | This represents how much X/Y distance (measured in strip units)
        | the particle moves in two layers. It is demanded that this
        | the particle leave the tower within 2 layers before performing
        | the check. A more stringent check would be that the track leaves
        | the tower before crossing the next layer. This looser condition
        | allows the track to be missing the hit in the next layer up
       */
       topHit  = prj->prjs[prjstart].hits[0];
       deltaXY = topHit - prj->prjs[prjstart].hits[2];
       check   = topHit + deltaXY;

       /*
        | Demand not only that the projection be heading towards the
        | left/lower edge of the tower, but also that it leave the tower
        | before crossing 1 more layers (projection distance is 2 - episilon).
       */
       if (check < 0)
       {
           int z;


           /*
            | Project to the left/lower edge finding the Z associated
            | with the intersection point. Translate the Z into a
            | mask of possible tiles.
           */
           toAcdPlane -= topHit;
           z           = zposition + toAcdPlane * deltaZ / deltaXY;
           mask        = findAcdSideMask (z, zSides);

       }
   }

   return mask;
}
/* ---------------------------------------------------------------------- */


/**----------------------------------------------------------------------
*
* @fn    int projectXYPtoAcd (const OnboardFilterTds::projections     *prj,
*                             int prjstart,
*						      int           zposition,
*                             int           towerEdge,
*                             int          toAcdPlane,
*                             int              deltaZ,
*                             const short int *zSides)
*
* @brief Projects an X or Y track projection to the X+/Y_ face
* @param prj        The track projection
* @param zposition  Starting z position
* @param towerEdge  The right/high edge of the tower in units of strips
* @param toAcdPlane Distance from the strip 0 of the tracker to the ACD plane
*                   in units of the strip pitch.
* @param deltaZ     Distance between the top and bottom hit planes
* @param zSides     The z positions of the side tiles
* @param towerId    The tower number. This is currently only used in
*                   diagnostic messages.
*
*  There is a parallel routine routine \e projectXYMtoAcd for projecting
*  off the left/lower side of a tower. The calling convention is not
*  the same because the left/lower edge of the tower is, by definition,
*  0, so the check for leaving a tower on the left/lower side is a simple
*  check against 0. For the right/high side, the check is against the
*  width of the tower (in strips).
*
** ---------------------------------------------------------------------- */
static __inline int projectXYPtoAcd (const OnboardFilterTds::projections     *prj,
				     int prjstart,
					 int           zposition,
				     int           towerEdge,
				     int          toAcdPlane,
				     int              deltaZ,
				     const short int *zSides)
{
   int  mask;
   int slope;


   mask  = 0;
   slope = prj->prjs[prjstart].hits[0] - prj->prjs[prjstart].hits[1];

   /*
    | The slope of the track must be negative to consider this projection
    | for exiting the left/lower edge of the tower.
   */
   if (slope > 0)
   {
       int   topHit;
       int  deltaXY;
       int    check;

       /*
        | Find distance between the top hit (hit 0) and the 2nd hit.
        | This represents how much X/Y distance (measured in strip units)
        | the particle moves in two layers. It is demanded that this
        | the particle leave the tower within 2 layers before performing
        | the check. A more stringent check would be that the track leaves
        | the tower before crossing the next layer. This looser condition
        | allows the track to be missing the hit in the next layer up
       */
       topHit  = prj->prjs[prjstart].hits[0];
       deltaXY = topHit - prj->prjs[prjstart].hits[2];
       check   = topHit + deltaXY;

       /*
        | Demand not only that the projection be heading towards the
        | right/high edge of the tower, but also that it leave the tower
        | before crossing 1 more layers (projection distance is 2 - episilon).
       */
       if (check > towerEdge)
       {
           int z;

           toAcdPlane -= topHit;
           z           = zposition + toAcdPlane * deltaZ / deltaXY;
           mask        = findAcdSideMask (z, zSides);
       }
   }

   return mask;
}

/* ---------------------------------------------------------------------- */


/**----------------------------------------------------------------------
*
* @fn    int projectXYStoAcd (int           zposition,
*                             int          toAcdPlane,
*                             int              deltaZ,
*                             int             deltaXY,
*                             const short int *zSides)
*
*
* @brief Projects an X or Y track projection to an ACD face.
* @param zposition  Starting z position
* @param toAcdPlane Distance from the top strip hit of the track to the ACD
*                   plane in units of the strip pitch.
* @param deltaZ     Distance between the top and bottom hit planes. This
*                   should always be twice the nominal Z layer separation.
* @param deltaXY    Distance between the top and bottom hits.
* @param zSides     The z positions of the side tiles
* @return           A mask of possible side tiles coincidences.
*
*
** ---------------------------------------------------------------------- */
static __inline int projectXYStoAcd (int           zposition,
					                 int          toAcdPlane,
				                     int              deltaZ,
				                     int             deltaXY,
				                     const short int *zSides)
{
   int    mask = 0;

   /*
    | The slope of the track must be greater than the approximate slope of
    | the width of a tracker module / 2 layers. This means that there is
    | is a reasonable chance that the track may hit only 2 layers in the
    | adjacent tower.
   */
   if (deltaXY > 2*1536/3)
   {
       int z;

       /*
        | Project to the left/lower edge finding the Z associated
        | with the intersection point. Translate the Z into a
        | mask of possible tiles.
       */
       z     = zposition + toAcdPlane * deltaZ / deltaXY;
       mask  = findAcdSideMask (z, zSides);
   }

   return mask;
}
/* ---------------------------------------------------------------------- */


/** ----------------------------------------------------------------------
*
* @fn int FilterAlg_acdProjectTemplate (int acd_x, int acd_y, int acd_z)
* @brief  Builds a template dispatch mask for this event.
* @param  acd_x    The bit mask of struck X+/-    ACD tiles.
* @param  acd_y    The bit mask of struck Y+/-    ACD tiles.
* @param  acd_z    The bit mask of struck Z (top) ACD tiles.
* @returns         The template dispatch mask for this event.
*
* The dispatch mask is a bit mask of the possible ACD projections to
* consider. For example, if no top tiles are struck, there is no reason
* to even attempt to project the tracks to the top tile face. This calculation
* s broken out since this is a global property of the event. It is computed
* once, then used for each tower that has found projections.
*
** ---------------------------------------------------------------------- */
int FilterAlg_acdProjectTemplate (int acd_x, int acd_y, int acd_z)
{
   int dispatch = 0;

   /* Build the faces to check base on the struck tiles */
   if (acd_z                      ) dispatch |= ACD_M_PRJ_TOP;
   if ((acd_x    & 0xffff0000)>>16) dispatch |= ACD_M_PRJ_XP | ACD_M_PRJ_XPS;
   if ((acd_y    & 0xffff0000)>>16) dispatch |= ACD_M_PRJ_YP | ACD_M_PRJ_YPS;
   if ((acd_x    & 0x0000ffff)    ) dispatch |= ACD_M_PRJ_XM | ACD_M_PRJ_XMS;
   if ((acd_y    & 0x0000ffff)    ) dispatch |= ACD_M_PRJ_YM | ACD_M_PRJ_YMS;

   return dispatch;
}
/* ---------------------------------------------------------------------- */


/**----------------------------------------------------------------------
*
* @fn int FilterAlg_acdProject (const OnboardFilterTds::projections     *prj,
*                         int                        xCnt,
*                         int                        yCnt,
*                         const struct _TFC_geometry *geo,
*                         int                     towerId,
*                         int                    dispatch,
*                         int                       acd_x,
*                         int                       acd_y,
*                         int                       acd_z)
* @brief  Projects the track projections to the ACD faces and looks for
*         an match with a stuck ACD tile.
* @param  prj      The list of projections
* @param  xCnt     The number of x projections. The first \a xCnt
*                  projections are X projections.
* @param  yCnt     The number of y projections. The next  \a yCnt
*                  projections are Y projections.
* @param  geo      The geometry of the LAT. This is used to do the
*                  projections, defining where various boundaries are
*                  from the perspective of the TKR.
* @param  towerId  The tower id of these projections
* @param  dispatch A bit mask of the possible projections to try based
*                  on the struck ACD tile. This must be consistent
*                  with \a acd_z, \a acd_x and \a acd_y.
* @param  acd_x    A bit mask of the struck ACD X       face tiles.
* @param  acd_y    A bit mask of the struck ACD Y       face tiles.
* @param  acd_z    A bit mask of the struck ACD Z (TOP) face tiles.
* @return          The first coincidence with one of faces. The upper 4
*                  bits indicate which face (0=Top, 1=X-, 2=X+, 3=Y-, 4=Y+)
*
/**---------------------------------------------------------------------- */
int FilterAlg_acdProject   (const OnboardFilterTds::projections       *prj,
                      int prjstart,
					  int                           xCnt,
                      int                           yCnt,
                      const struct _TFC_geometry    *geo,
                      int                        towerId,
                      int                       dispatch,
                      unsigned int                  acd_x,
                      unsigned int                  acd_y,
                      unsigned int                  acd_z)
{
   /* Limit which faces to check based on the tower */
   dispatch &= AcdDispatch.masks[towerId];

   /* If no projections available, eliminate from the dispatch mask */
   if (xCnt == 0)
   {
       dispatch &= ~(ACD_M_PRJ_XM  | ACD_M_PRJ_XP  |
                     ACD_M_PRJ_XMS | ACD_M_PRJ_XPS | ACD_M_PRJ_TOP);
   }

   if (yCnt == 0)
   {
       dispatch &= ~(ACD_M_PRJ_YM  | ACD_M_PRJ_YP  |
                     ACD_M_PRJ_YMS | ACD_M_PRJ_YPS | ACD_M_PRJ_TOP);
   }


   /* If must project to the ACD TOP FACE */
   if (dispatch & ACD_M_PRJ_TOP)
   {
       int mask        = prjAcd_Top (prj, prjstart, xCnt, yCnt, geo, towerId);
       int coincidence = mask & acd_z;
       if (coincidence) return (ACD_V_PRJ_TOP << 28) | coincidence;
   }

   /* If must project to the ACD X- FACE */
   if (dispatch & ACD_M_PRJ_XM )
   {
       int mask        = prjAcd_XM  (prj, prjstart, xCnt, yCnt, geo, towerId);
       int xm          = acd_x & 0xffff;
       int coincidence = mask & xm;
       if (coincidence) return (ACD_V_PRJ_XM << 28) | coincidence;
   }

   /* If must project to the ACD X+ FACE */
   if (dispatch & ACD_M_PRJ_XP )
   {
       int mask        = prjAcd_XP  (prj, prjstart, xCnt, yCnt, geo, towerId);
       int xp          = (unsigned int)acd_x >> 16;
       int coincidence = mask & xp;
       if (coincidence)  return (ACD_V_PRJ_XP << 28) | coincidence;
   }

   /* If must project to the ACD Y- FACE */
   if (dispatch & ACD_M_PRJ_YM )
   {
       int mask        = prjAcd_YM  (prj, prjstart, xCnt, yCnt, geo, towerId);
       int ym          = acd_y & 0xffff;
       int coincidence = mask & ym;
       if (coincidence) return (ACD_V_PRJ_YM << 28) | coincidence;
   }

   /* If must project to the ACD Y+ FACE */
   if (dispatch & ACD_M_PRJ_YP )
   {
       int mask        = prjAcd_YP  (prj, prjstart, xCnt, yCnt, geo, towerId);
       int yp          = (unsigned int)acd_y >> 16;
       int coincidence = mask & yp;
       if (coincidence)   return (ACD_V_PRJ_YP << 28) | coincidence;
   }

   /* If must project to the ACD X- FACE, skip one tower */
   if (dispatch & ACD_M_PRJ_XMS)
   {
       int mask        = prjAcd_XMS (prj,prjstart, xCnt, yCnt, geo);
       int xm          = acd_x & 0xffff;
       int coincidence = mask & xm;
       if (coincidence) return (ACD_V_PRJ_XM << 28) | coincidence;
   }

   /* If must project to the ACD X+ FACE, skip one tower */
   if (dispatch & ACD_M_PRJ_XPS)
   {
       int mask        = prjAcd_XPS (prj, prjstart, xCnt, yCnt, geo);
       int xp          = (unsigned int)acd_x >> 16;
       int coincidence = mask & xp;
       if (coincidence)  return (ACD_V_PRJ_XP << 28) | coincidence;
   }

   /* If must project to the ACD Y- FACE, skip one tower */
   if (dispatch & ACD_M_PRJ_YMS)
   {
       int mask        = prjAcd_YMS (prj, prjstart, xCnt, yCnt, geo);
       int ym          = acd_y & 0xffff;
       int coincidence = mask & ym;
       if (coincidence) return (ACD_V_PRJ_YM << 28) | coincidence;
   }

   /* If must project to the ACD Y+ FACE, skip one tower */
   if (dispatch & ACD_M_PRJ_YPS)
   {
       int mask        = prjAcd_YPS (prj, prjstart, xCnt, yCnt, geo);
       int yp          = (unsigned int)acd_y >> 16;
       int coincidence = mask & yp;
       if (coincidence)   return (ACD_V_PRJ_YP << 28) | coincidence;
   }

   return 0;
}
/* ---------------------------------------------------------------------- */


/**----------------------------------------------------------------------
*
* @fn   int prjAcd_Top (const OnboardFilterTds::projections         *prj,
*                       int                    xCnt,
*                       int                    yCnt,
*                       const TFC_geometry     *geo,
*                       int                 towerId)
* @brief  Loops through the list of projections and checks for coincidences
*         with the TOP ACD face.
* @return A mask of possible ACD TOP tile coincidences.
*
* @param prj     The list of projections
* @param xCnt    The number of X projections
* @param yCnt    The number of Y projections
* @param  geo    The geometry of the LAT. This is used to do the
*                projections, defining where various boundaries are
*                from the perspective of the TKR.
* @param towerId The tower number. Used only for diagnostic messages.
*
*
/**---------------------------------------------------------------------- */
static int prjAcd_Top (const OnboardFilterTds::projections        *prj,
                       int                   prjstart,
					   int                   xCnt,
                       int                   yCnt,
                       const TFC_geometry    *geo,
                       int                towerId)
{
    int         row;
    int         col;
    int      offset;
    int acdTopMaskX;
    int acdTopMaskY;
    int    tkrWidth;

    /*
     | Project the X track projections to the top face of the ACD
     | A bit mask of candidate tiles is formed. This will be a column
     | of Y tiles.
    */
    col         = (towerId & 0x3);
    offset      = geo->tkr.twr.xy[0].offsets[col];
    tkrWidth    = geo->tkr.xyWidths[0];
    acdTopMaskX = 0;
    while (--xCnt >= 0)
    {
        int mask;

        mask = projectXYtoAcdTop (prj, prjstart,
                                  offset,
                                  _ACD_X_TEMPLATE,
                                  _ACD_X_SHIFT_AMOUNT,
                                  geo->acd.xProjections,
                                  geo->acd.xTopEdges);
        acdTopMaskX    |= mask;
        prjstart++;
    }

    /*
     | Project the Y track projections to the top face of the ACD
     | A bit mask of candidate tiles is formed. This will be a row
     | of X tiles.
    */
    row         = (towerId >> 2) & 0x3;
    offset      = geo->tkr.twr.xy[1].offsets[row];
    tkrWidth    = geo->tkr.xyWidths[0];
    acdTopMaskY = 0;
    while (--yCnt >= 0)
    {
        int mask;

		mask = projectXYtoAcdTop (prj,prjstart,
                                  offset,
                                  _ACD_Y_TEMPLATE,
                                  _ACD_Y_SHIFT_AMOUNT,
                                  geo->acd.yProjections,
                                  geo->acd.yTopEdges);
        acdTopMaskY    |= mask;
        prjstart++;
    }

    /*
     | Return the coincidence of the X and Y candidate tiles.
     | If non-zero, the caller can either use this to reject the event,
     | without matching X and Y projections, or it can be used as a
     | quick indicator that there is/is not a possibility of a coincidence
     | one does XY projection matching.
    */
    return acdTopMaskX & acdTopMaskY;
}
/* ---------------------------------------------------------------------- */


/**----------------------------------------------------------------------
*
* @fn   int prjAcd_XM (const OnboardFilterTds::projections        *prj,
*                      int prjstart,
*				       int                   xCnt,
*                      int                   yCnt,
*                      const TFC_geometry    *geo,
*                      int                towerId)
* @brief Loops through the list of X projections and checks for coincidences
*        with the X- ACD face.
* @param prj     The list of projections
* @param xCnt    The number of X projections
* @param yCnt    The number of Y projections
* @param geo     The geometry of the LAT. This is used to do the
*                projections, defining where various boundaries are
*                from the perspective of the TKR.
* @param towerId The tower number. Used only for diagnostic messages.
* @return        A mask of possible ACD X- tile coincidences.
*
** ---------------------------------------------------------------------- */
static int prjAcd_XM  (const OnboardFilterTds::projections        *prj,
                       int prjstart,
					   int                   xCnt,
                       int                   yCnt,
                       const TFC_geometry    *geo,
                       int                towerId)
{
    yCnt=0;//****
    int acdMask    = 0;
    int toAcdFace  = geo->acd.xySides[0] - geo->tkr.twr.xy[0].offsets[0];
    int     deltaZ = geo->acd.zNominal;

    while (--xCnt >= 0)
    {
        int mask;

        mask = projectXYMtoAcd (prj,prjstart,
                                geo->tkr.twr.xy[0].z[prj->prjs[prjstart].max],
                                toAcdFace,
                                deltaZ,
                                geo->acd.zSides);
        acdMask      |= mask;
        prjstart++;
    }

    return acdMask;
}
/* ---------------------------------------------------------------------- */


/**----------------------------------------------------------------------
*
* @fn   int prjAcd_XP (const OnboardFilterTds::projections        *prj,
*                      int prjstart,
*    				   int                   xCnt,
*                      int                   yCnt,
*                      const TFC_geometry    *geo,
*                      int                towerId)
* @brief Loops through the list of X projections and checks for coincidences
*        with the X+ ACD face.
* @param prj     The list of projections
* @param xCnt    The number of X projections
* @param yCnt    The number of Y projections
* @param geo     The geometry of the LAT. This is used to do the
*                projections, defining where various boundaries are
*                from the perspective of the TKR.
* @param towerId The tower number. Used only for diagnostic messages.
* @return        A mask of possible ACD X+ tile coincidences.
*
** --------------------------------------------------------------------- */
static int prjAcd_XP  (const OnboardFilterTds::projections        *prj,
                       int prjstart,
					   int                   xCnt,
                       int                   yCnt,
                       const TFC_geometry    *geo,
                       int                towerId)
{
    yCnt=0;
    int acdMask    = 0;
    int toAcdFace  = geo->acd.xySides[1] - geo->tkr.twr.xy[0].offsets[3];
    int towerWidth = geo->tkr.xyWidths[0];
    int     deltaZ = geo->acd.zNominal;

    while (--xCnt >= 0)
    {
        int mask;

        mask = projectXYPtoAcd (prj,prjstart,
                                geo->tkr.twr.xy[0].z[prj->prjs[prjstart].max],
                                towerWidth,
                                toAcdFace,
                                deltaZ,
                                geo->acd.zSides);
        acdMask      |= mask;
        prjstart++;
    }

    return acdMask;

}
/* ---------------------------------------------------------------------- */


/**----------------------------------------------------------------------
*
* @fn   int prjAcd_YM (const OnboardFilterTds::projections        *prj,
*                      int prjstart,
*    				   int                   xCnt,
*                      int                   yCnt,
*                      const TFC_geometry    *geo,
*                      int                towerId)
* @brief Loops through the list of Y projections and checks for coincidences
*        with the Y- ACD face.
* @param prj     The list of projections
* @param xCnt    The number of X projections
* @param yCnt    The number of Y projections
* @param geo     The geometry of the LAT. This is used to do the
*                projections, defining where various boundaries are
*                from the perspective of the TKR.
* @param towerId The tower number. Used only for diagnostic messages.
* @return        A mask of possible ACD Y- tile coincidences.
*
** ---------------------------------------------------------------------- */
static int prjAcd_YM  (const OnboardFilterTds::projections        *prj,
                       int prjstart,
					   int                   xCnt,
                       int                   yCnt,
                       const TFC_geometry    *geo,
                       int                towerId)
{
    int acdMask    = 0;
    int toAcdFace  = geo->acd.xySides[2] - geo->tkr.twr.xy[1].offsets[0];
    int     deltaZ = geo->acd.zNominal;

    prjstart += xCnt;
    while (--yCnt >= 0)
    {
        int mask;

        mask = projectXYMtoAcd (prj,prjstart,
                                geo->tkr.twr.xy[1].z[prj->prjs[prjstart].max],
                                toAcdFace,
                                deltaZ,
                                geo->acd.zSides);
        acdMask      |= mask;
        prjstart++;
    }

    return acdMask;
}
/* ---------------------------------------------------------------------- */


/**----------------------------------------------------------------------
*
* @fn   int prjAcd_YP (const OnboardFilterTds::projections        *prj,
*                      int prjstart,
*				   int                   xCnt,
*                      int                   yCnt,
*                      const TFC_geometry    *geo,
*                      int                towerId)
* @brief Loops through the list of Y projections and checks for coincidences
*        with the Y+ ACD face.
* @param prj     The list of projections
* @param xCnt    The number of X projections
* @param yCnt    The number of Y projections
* @param geo     The geometry of the LAT. This is used to do the
*                projections, defining where various boundaries are
*                from the perspective of the TKR.
* @param towerId The tower number. Used only for diagnostic messages.
* @return        A mask of possible ACD Y+ tile coincidences.
*
** ---------------------------------------------------------------------- */
static int prjAcd_YP  (const OnboardFilterTds::projections        *prj,
                       int prjstart,
					   int                   xCnt,
                       int                   yCnt,
                       const TFC_geometry    *geo,
                       int                towerId)
{
    int acdMask    = 0;
    int toAcdFace  = geo->acd.xySides[3] - geo->tkr.twr.xy[1].offsets[3];
    int towerWidth = geo->tkr.xyWidths[1];
    int     deltaZ = geo->acd.zNominal;

    prjstart += xCnt;
	while (--yCnt >= 0)
    {
        int mask;

        mask = projectXYPtoAcd (prj,prjstart,
                                geo->tkr.twr.xy[1].z[prj->prjs[prjstart].max],
                                towerWidth,
                                toAcdFace,
                                deltaZ,
                                geo->acd.zSides);
        acdMask      |= mask;
        prjstart++;
    }

    return acdMask;
}
/* ---------------------------------------------------------------------- */


/** ----------------------------------------------------------------------
*
*  @fn   int prjAcd_XMS (const OnboardFilterTds::projections        *prj,
*                        int prjstart,
*						int                   xCnt,
*                        int                   yCnt,
*                        const TFC_geometry    *geo)
*  @brief Loops through the list of X projections and checks for coincidences
*         with the X- ACD face.
*  @param prj     The list of projections
*  @param xCnt    The number of X projections
*  @param yCnt    The number of Y projections
*  @param geo     The geometry of the LAT. This is used to do the
*                 projections, defining where various boundaries are
*                 from the perspective of the TKR.
*  @param towerId The tower number. Used only for diagnostic messages.
*  @return        A mask of possible ACD X- tile coincidences.
*
*  Only projections with very shallow angles are considered. This routine
*  attempts to skip over a tower that may not have sufficient hits to
*  cause a 3-in-a-row, as will occassionally happen when the angle is very
*  small.
*
** ---------------------------------------------------------------------- */
static int prjAcd_XMS  (const OnboardFilterTds::projections        *prj,
                        int prjstart,
						int                   xCnt,
                        int                   yCnt,
                        const TFC_geometry    *geo)
{   yCnt=0;
    int acdMask    =  0;
    int toAcdFace  =  geo->acd.xySides[0] - geo->tkr.twr.xy[0].offsets[1];
    int     deltaZ = -geo->acd.zNominal;

    while (--xCnt >= 0)
    {
        int    mask;
        int  topHit;
        int  botHit;
        int deltaXY;

        topHit  = prj->prjs[prjstart].hits[0];
        botHit  = prj->prjs[prjstart].hits[2];
        deltaXY = botHit - topHit;

        mask = projectXYStoAcd (geo->tkr.twr.xy[0].z[prj->prjs[prjstart].max],
                                toAcdFace - topHit,
                                deltaZ,
                                deltaXY,
                                geo->acd.zSides);
        acdMask      |= mask;
        prjstart++;
    }

    return acdMask;
}
/**---------------------------------------------------------------------- */


/**----------------------------------------------------------------------
*
*  @fn   int prjAcd_XPS (const OnboardFilterTds::projections        *prj,
*                        int prjstart,
*						int                   xCnt,
*                        int                   yCnt,
*                        const TFC_geometry    *geo)
*  @brief Loops through the list of X projections and checks for coincidences
*         with the X+ ACD face.
*  @param prj     The list of projections
*  @param xCnt    The number of X projections
*  @param yCnt    The number of Y projections
*  @param geo     The geometry of the LAT. This is used to do the
*                 projections, defining where various boundaries are
*                 from the perspective of the TKR.
*  @param towerId The tower number. Used only for diagnostic messages.
*  @return        A mask of possible ACD X+ tile coincidences.
*
** ---------------------------------------------------------------------- */
static int prjAcd_XPS (const OnboardFilterTds::projections        *prj,
                       int prjstart,
					   int                   xCnt,
                       int                   yCnt,
                       const TFC_geometry    *geo)
{   yCnt=0;
    int acdMask    = 0;
    int toAcdFace  = geo->acd.xySides[1] - geo->tkr.twr.xy[0].offsets[2];
    int     deltaZ = geo->acd.zNominal;

    while (--xCnt >= 0)
    {
        int    mask;
        int  topHit;
        int  botHit;
        int deltaXY;

        topHit  = prj->prjs[prjstart].hits[0];
        botHit  = prj->prjs[prjstart].hits[2];
        deltaXY = topHit - botHit;

        mask = projectXYStoAcd (geo->tkr.twr.xy[0].z[prj->prjs[prjstart].max],
                                toAcdFace - topHit,
                                deltaZ,
                                deltaXY,
                                geo->acd.zSides);

        acdMask      |= mask;
        prjstart++;
    }

    return acdMask;

}
/**---------------------------------------------------------------------- */


/**---------------------------------------------------------------------
*
*  @fn   int prjAcd_YMS (const OnboardFilterTds::projections        *prj,
*                       int prjstart,
*						int                   xCnt,
*                       int                   yCnt,
*                        const TFC_geometry    *geo)
*  @brief Loops through the list of Y projections and checks for coincidences
*         with the Y- ACD face.
*  @param prj     The list of projections
*  @param xCnt    The number of X projections
*  @param yCnt    The number of Y projections
*  @param geo     The geometry of the LAT. This is used to do the
*                 projections, defining where various boundaries are
*                 from the perspective of the TKR.
*  @param towerId The tower number. Used only for diagnostic messages.
*  @return        A mask of possible ACD Y- tile coincidences.
*
** ---------------------------------------------------------------------- */
static int prjAcd_YMS  (const OnboardFilterTds::projections        *prj,
                        int prjstart,
						int                   xCnt,
                        int                   yCnt,
                        const TFC_geometry    *geo)
{
    int acdMask    =  0;
    int toAcdFace  =  geo->acd.xySides[2] - geo->tkr.twr.xy[1].offsets[1];
    int     deltaZ = -geo->acd.zNominal;

    prjstart += xCnt;
    while (--yCnt >= 0)
    {
        int    mask;
        int  topHit;
        int  botHit;
        int deltaXY;

        topHit  = prj->prjs[prjstart].hits[0];
        botHit  = prj->prjs[prjstart].hits[2];
        deltaXY = botHit - topHit;

        mask = projectXYStoAcd (geo->tkr.twr.xy[1].z[prj->prjs[prjstart].max],
                                toAcdFace - topHit,
                                deltaZ,
                                deltaXY,
                                geo->acd.zSides);

        acdMask      |= mask;
        prjstart++;
    }

    return acdMask;
}
/* ---------------------------------------------------------------------- */


/**----------------------------------------------------------------------
*
*  @fn   int prjAcd_YPS (const OnboardFilterTds::projections        *prj,
*                        int prjstart,
*						int                   xCnt,
*                        int                   yCnt,
*                        const TFC_geometry    *geo)
*  @brief Loops through the list of Y projections and checks for coincidences
*         with the Y+ ACD face.
*  @param prj     The list of projections
*  @param xCnt    The number of X projections
*  @param yCnt    The number of Y projections
*  @param geo     The geometry of the LAT. This is used to do the
*                 projections, defining where various boundaries are
*                 from the perspective of the TKR.
*  @param towerId The tower number. Used only for diagnostic messages.
*  @return        A mask of possible ACD Y+ tile coincidences.
*
** ---------------------------------------------------------------------- */
static int prjAcd_YPS  (const OnboardFilterTds::projections        *prj,
                        int prjstart,
						int                   xCnt,
                        int                   yCnt,
                        const TFC_geometry    *geo)
{
    int acdMask    = 0;
    int toAcdFace  = geo->acd.xySides[3] - geo->tkr.twr.xy[1].offsets[2];
    int     deltaZ = geo->acd.zNominal;


    prjstart += xCnt;
    while (--yCnt >= 0)
    {
        int mask;
        int  topHit;
        int  botHit;
        int deltaXY;

        topHit  = prj->prjs[prjstart].hits[0];
        botHit  = prj->prjs[prjstart].hits[2];
        deltaXY = topHit - botHit;

        mask = projectXYStoAcd (geo->tkr.twr.xy[1].z[prj->prjs[prjstart].max],
                                toAcdFace - topHit,
                                deltaZ,
                                deltaXY,
                                geo->acd.zSides);

        acdMask      |= mask;
        prjstart++;
    }

    return acdMask;
}
/**---------------------------------------------------------------------- */
