#ifndef TFC_GEOMETRYDEF_H
#define TFC_GEOMETRYDEF_H



/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   TFC_geometryDef.h
   \brief  Defines the tower geometry used in track finding
   \author JJRussell - russell@slac.stanford.edu

                                                                         */
/* --------------------------------------------------------------------- */


#ifdef __cplusplus
extern "C" {
#endif    

/* --------------------------------------------------------------------- *//*!

  \typedef TFC_geometryTowerZ
  \brief   Hides the implementation size of a Z coordinate expressed
           in the TFC Z units
                                                                         */
/* --------------------------------------------------------------------- */  
typedef signed short TFC_geometryTowerZ;
/* --------------------------------------------------------------------- */  



    
    
/* --------------------------------------------------------------------- *//*!

  \typedef TFC_geometryTowerOffset
  \brief   Hides the implementation size of a XY offset expressed
           in the TFC units (typically an integer fraction of a strip)
                                                                         */
/* --------------------------------------------------------------------- */  
typedef signed short TFC_geometryTowerOffset;
/* --------------------------------------------------------------------- */

    

    
/* --------------------------------------------------------------------- *//*!

  \struct _TFC_geometryTowerXY
  \brief  Describes the TKR geometry of either an X or Y stack of towers.
                                                                         *//*!
  \typedef TFC_geometryTowerXY
  \brief   Typedef for struct \e _TFC_geometryTowerXY
                                                                         */
/* --------------------------------------------------------------------- */
typedef struct _TFC_geometryTowerXY
{
  TFC_geometryTowerOffset offsets[4]; /*!< Offset, beginning of row or
                                           column N                      */
  TFC_geometryTowerZ           z[18]; /*!< Z position of layer n         */
}
TFC_geometryTowerXY;
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!

  \struct _TFC_geometryTower
  \brief  Describes the geometry of all TKR towers.
                                                                         *//*!
  \typedef TFC_geometryTower
  \brief   Typedef for struct \e _TFC_geometryTower
                                                                         */
/* --------------------------------------------------------------------- */
typedef struct _TFC_geometryTower
{
    TFC_geometryTowerXY xy[2];   /*!< Geometry of the X/Y layers         */
}
TFC_geometryTower;
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \struct _TFC_geometryTkr
  \brief  Describes the geometry of a TKR tower. The XY units are in
          integer multiples of strips and the Z units are in integer
          multiples of mm. The exact units are defined in this file,
          which provides macros to convert from physical units to
          internal filtering units. This structure just adds couple
          of values that apply to the TKR tower as a whole.
                                                                         *//*!
  \typedef TFC_geometryTkr
  \brief   Typedef for struct \e _TFC_geometryTkr
                                                                         */
/* --------------------------------------------------------------------- */
typedef struct _TFC_geometryTkr
{
    int          zfindMaxMin;  /*!< Geometric factor used when initially
                                    finding projections. This is a
                                    packed number giving two numbers
                                    for the two configurations of the
                                    stagger, ie projecting when the
                                    middle layer is closer/further from
                                    the top layer                        */
    int        zextendMaxMin;  /*!< Geometric factor used when extending
                                    a projection to the next layer. This
                                    is a packed number giving two numbers
                                    for the two configuration of the
                                    stagger, ie projecting when the seed
                                    pair have maximum/minimum separation */
    short int    xyWidths[2];  /*!< Width of tower in strips X/Y         */
    TFC_geometryTower    twr;  /*!< The XY and Z edges of the TKR        */
}
TFC_geometryTkr;
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \struct _TFC_geometryAcd
  \brief   Describes the geometry of the ACD. The XY units are in integer
           multiples of strips and the Z units are in integer multiples
           of mm. The exact units are defined in this file, which 
           which provides macros to convert from physical units to
           internal filtering units.
                                                                         *//*!
  \typedef TFC_geometryAcd
  \brief   Typedef for struct \e _TFC_geometry
                                                                         */
/* --------------------------------------------------------------------- */  
typedef struct _TFC_geometryAcd
{
    int                        zNominal; /*!< TKR spacing in abs units   */
    short int              xTopEdges[6]; /*!< X edges of the top tiles   */
    short int              yTopEdges[6]; /*!< Y edges of the top tiles   */
    short int            xSideYedges[6]; /*!< Y edges of X side tiles    */
    short int            ySideXedges[6]; /*!< X edges of Y side tiles    */
    short int                 zSides[6]; /*!< Side row Z boundaries      */
    int                      xySides[4]; /*!< X-/+, Y-/+                 */
    unsigned short int xProjections[18]; /*!< From layer X -> ACD top    */
    unsigned short int yProjections[18]; /*!< From layer Y -> ACD top    */
}
TFC_geometryAcd;
/* --------------------------------------------------------------------- */  




/* --------------------------------------------------------------------- *//*!

  \struct _TFC_geometrySkirt
  \brief  Describes the geometry at the plane of the skirt (the TKR/CAL
          boundary. This is used to project a track to the plane of
          skirt and then to ascertain which region(s) of the skirt the
          projection passes through.
                                                                         *//*!
  \typedef TFC_geometrySkirt
  \brief   Typedef for struct \e _TFC_geometrySkirt
                                                                         */
/* --------------------------------------------------------------------- */
typedef struct _TFC_geometrySkirt
{
    short int z;                         /*!< Zposition of the skirt     */
    short int xy[2][4];                  /*!< Two dimensional boundary of
                                             the region. Nominally this
                                             will be the edges
                                             -ACD, -TOWER, +TOWER +ACD   */
    unsigned short int xProjections[18]; /*!< From X layer -> skirt      */
    unsigned short int yProjections[18]; /*!< From Y layer -> skirt      */
}
TFC_geometrySkirt;
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \struct _TFC_geometry
  \brief   Collects all the structures defining the LAT geometry into one
           grand structure.
                                                                         *//*!
  \typedef TFC_geometry
  \brief   Typedfe for struct \e _TFC_geometry
                                                                         */
/* --------------------------------------------------------------------- */
typedef struct _TFC_geometry
{
    TFC_geometryTkr     tkr;  /*!< The Tracker geometry                  */
    TFC_geometryAcd     acd;  /*!< The ACD     geometry                  */
    TFC_geometrySkirt skirt;  /*!< The Skirt   geometry                  */
}
TFC_geometry;
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def    TFC_XY_SCALE(_xy, _stripWidth)
  \brief  Macro to scale a transverse direction by the strip width. The
          definition is such as to guarantee +/- symmetry. This may or may
          not be desirable.
  \param _xy         The x or y dimension to scale to strip units
  \param _stripWidth The width of strip.

   Note that \a _xy and \a _stripWidth \e must be in the same units, eg
   cm or mm.
                                                                         */
/* --------------------------------------------------------------------- */
#define TFC_XY_SCALE(_xy, _stripWidth)                       \
    ((_xy > 0) ?  ((int)(((+(_xy))/_stripWidth + 0.5)))      \
               : -((int)(((-(_xy))/_stripWidth + 0.5))))
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def    TFC_Z_ABS_SCALE_FACTOR
  \brief  Sets the Z scale for the filter. This scale factor is applied
          to physical Z dimensions expressed in mm.
          
   This scaling will be applied to all absolute Z dimensions.
   The largest Z dimension is approximately 1000mm, so this number
   should be chosen so that the arithmetic will not overflow
                                                                          */
/* --------------------------------------------------------------------- */
#define TFC_Z_ABS_SCALE_FACTOR  10
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def   TFC_Z_ABS_SCALE(_z)
  \brief Macro to scale the Z dimensions such that 1mm = ZSCALE_FACTOR
  \param _z The Z dimension, expressed in mm, to scale
  \return   The scaled Z dimension
                                                                          
                                                                         */
/* --------------------------------------------------------------------- */
#define TFC_Z_ABS_SCALE(_z)     \
       ((TFC_geometryTowerZ)(((_z) * TFC_Z_ABS_SCALE_FACTOR) +.5))
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------  *\
 |                                                                       |  
 | PROJECTIONS FROM TKR to a middle TKR LAYER                            |
 | ==========================================                            |   
 |                                                                       |  
 | This defines how to project from any pair of jacketing TKR layers to  |
 | jacketed (middle) TKR layer.                                          |
 |                                                                       |
\* --------------------------------------------------------------------- */


/* --------------------------------------------------------------------- *//*!

  \def   TFC_Z_FIND_SCALE_FACTOR
  \brief Used when finding a seed projection.

  This exact scale factor is selected by the following criteria
  \verbatim
    1. The maximum projection error due to using integer arthimetic is
       less than one strip.
    2. The calculation never overflows 32-bit integer arthimetic
    3. The value is a multiple of two, so that removing the scale factor
       by division is avoided, using a shift operation instead.
  \endverbatim

  The pattern recognition uses this value when finding a seed projection.
  A seed projection is found by picking 3 adjacent TKR layers. One hit  
  from each of the outer 2 layers is selected and the predicted hit in
  the middle layer is calculated. To perform this calculation the
  fractional distance of the middle layer needs to be calculated. Since
  all calculation is done using integer arthimetic this fraction (which
  would be 1/2 if the middle plane was halfway between the two outer
  planes) is scaled by this number.
                                                                         */
/* --------------------------------------------------------------------- */
#define TFC_Z_FIND_SCALE_FACTOR   2048       /* Find scaling factor      */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def   TFC_Z_FIND_SCALE(_zratio)
  \brief Macro to scale a Z ratio when finding a seed projection.
  \param  _zratio The _z ratio to scale
  \return         The scaled Z ratio
                                                                         *//*!
                                                                             
  \def   TFC_Z_FIND(_z, _nominal)
  \brief Macro to scale the specified Z separtion  to a percentage
         of the nominal plane separation. The fractional units are \e
         TFC_Z_FIND_SCALE.
         
  \param _z       The Z separation (in mm) to scale
  \param _nominal The nominal Z separation (ie, no stagger) (in mm)
  \return         The ratio of the specified Z separtion to the nominal
                  separation in units of TFC_Z_FIND_SCALE
                                                                         *//*!
                                                                             
  \def   TFC_Z_FIND_MAX(_nominal, _delta)
  \brief Ratio of the Z separation to the nominal separation when the
         stagger is a maximum.
         
  \param _nominal The nominal separation (in mm).
  \param _delta   The full stagger (in mm).
  \return         The ratio of the Z distance to nominal separation
                  of two planes. (Produces TFC_Z_FIND_SCALE when
                  _delta = 0.)
                                                                         *//*!
                                                                             
  \def   TFC_Z_FIND_MIN(_nominal, _delta)
  \brief Ratio of the Z separation to the nominal separation when the
         stagger is a minimum.
         
  \param _nominal The nominal separation in units of TFC_Z_FIND_SCALE
  \param _delta   The full stagger (in mm).
  \return         The ratio of the Z distance to nominal separation
                  of two planes. (Produces TFC_Z_FIND_SCALE when
                  _delta = 0.)

                                                                        
  TFC_Z_FIND_MAX/MIN is used during the find stage to predict the  hit  
  in the middle layers. The MAX is used when the middle layer is further
  from the top layer, and MIN is used when the middle layer is closer to
  the top layer.
                                                                         */
/* --------------------------------------------------------------------- */
#define TFC_Z_FIND_SCALE(_zratio) (((_zratio) * TFC_Z_FIND_SCALE_FACTOR))

#define TFC_Z_FIND(_z, _nominal)                         \
        ((int)((TFC_Z_FIND_SCALE(_z))/(_nominal) +.5))

#define TFC_Z_FIND_MAX(_nominal, _delta)                 \
        TFC_Z_FIND(((_nominal) + (_delta))/2.0, _nominal)

#define TFC_Z_FIND_MIN(_nominal, _delta)                 \
        TFC_Z_FIND(((_nominal) - (_delta))/2.0, _nominal)
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------  *\
 |                                                                       |  
 | PROJECTIONS FROM TKR to another TKR LAYER                             |
 | =========================================                             |   
 |                                                                       |  
 | This defines how to project from any TKR pair of layers to another    |
 | TKR layer.                                                             |
 |                                                                       |
\* --------------------------------------------------------------------- */


/* --------------------------------------------------------------------- *//*!

  \def   TFC_Z_EXTEND_SCALE_FACTOR
  \brief Used when extending a projection to an adjacent TKR layer.

  This exact scale factor is selected by the following criteria
  \verbatim
    1. The maximum projection error due to using integer arthimetic is
       less than one strip.
    2. The calculation never overflows 32-bit integer arthimetic
    3. The value is a multiple of two, so that removing the scale factor
       by division is avoided, using a shift operation instead.
  \endverbatim

  The pattern recognition uses this value when extending a projection.
  The hits in the two planes adjacent to the layer the projection is
  being extended two are selected. To predict where the projection hits
  in the next layer, the ratio of the Z distance between the two hit
  layers and the Z distance between the last hit layer the extension
  layer needs to be calculated. Since the calculation is done using
  integer arthimetic, this ratio is scaled by this number.
                                                                         */
/* --------------------------------------------------------------------- */
#define TFC_Z_EXTEND_SCALE_FACTOR  2048      /* Scales ratio of two Z    */
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!

  \def   TFC_Z_EXTEND_SCALE(_zratio)
  \brief Macro to scale a Z ratio when extendig a seed projection.
  \param  _zratio The _z ratio to scale
  \return         The scaled Z ratio
                                                                         *//*!
                                                                             
  \def   TFC_Z_EXTEND_MAX(_nominal, _delta)
  \brief Calculates the ratio of the Z separation when the layers are
         maximally separated to the Z separation when the layers are
         minimally separated.
         
  \param _nominal The nominal separation (in mm)
  \param _delta   The full stagger (in mm).
  \return         Tthe ratio of the Z separation when the layers are
                  maximally separated to the Z separation when the layers
                  are minimally separated. Produces TFC_Z_EXTEND_SCALE
                  when \a _delta = 0. When \a _delta is non-zero, this
                  ratio will be greater than TFC_Z_EXTEND_SCALE.
                                                                         *//*!
                                                                             
  \def   TFC_Z_EXTEND_MIN(_nominal, _delta)
  \brief Calculates the ratio of the Z separation when the layers are
         maximally separated to the Z separation when the layers are
         minimally separated.
         
  \param _nominal The nominal separation (in mm)
  \param _delta   The full stagger (in mm).
  \return         The ratio of the Z separation when the layers are
                  minimally separated to the Z separation when the layers
                  are maximally separated. Produces TFC_Z_EXTEND_SCALE
                  when \a _delta = 0. When \a _delta is non-zero, this
                  ratio will be less than TFC_Z_EXTEND_SCALE.

                                                                        
  TFC_Z_EXTEND_MAX/MIN is used when extending a seed projection from
  two adjacent layers to a third consequetive layer. The MAX is used when
  the extension layer is furtherest from the last layer, and MIN is used
  when the extension layer is closer to the last layer.
  \e TKR_Z_EXTEND_SCALE is purely an internal helper macro.
                                                                         */
/* --------------------------------------------------------------------- */
#define TFC_Z_EXTEND_SCALE(_z) (((_z) * TFC_Z_EXTEND_SCALE_FACTOR))

#define TFC_Z_EXTEND_MAX(_nominal, _delta)                                \
        ((int)(TFC_Z_EXTEND_SCALE(((_nominal) + (_delta))  /              \
                                  ((_nominal) - (_delta)) ) + 0.5))

#define TFC_Z_EXTEND_MIN(_nominal, _delta)                                \
        ((int)(TFC_Z_EXTEND_SCALE((((_nominal) - (_delta)) /              \
                                   ((_nominal) + (_delta)) ) ) + 0.5))
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def   TFC_Z_XLAYER_MM(_layer, _nominal, _offset, _delta)
  \brief Defines the absolute Z position of an X layer.

  \param _layer   The layer number (0-17, 0 closest to the CAL)
  \param _nominal The nominal layer spacing in mm.
  \param _offset  The Z position of layer 0 (middle of X & Y)
  \param _delta   The full stagger in mm.
  \return         The absolute Z position of the specified X layer
                                                                         *//*!
  \def   TFC_Z_YLAYER_MM(_layer, _nominal, _offset, _delta)
  \brief Defines the absolute Z position of an Y layer.

  \param _layer   The layer number (0-17, 0 closest to the CAL)
  \param _nominal The nominal layer spacing in mm.
  \param _offset  The Z position of layer 0 (middle of X & Y)
  \param _delta   The full stagger in mm.
  \return         The absolute Z position of the specified Y layer
                                                                         */
/* --------------------------------------------------------------------- */  
#define TFC_Z_XLAYER_MM(_layer, _nominal, _offset, _delta)                \
         ((_nominal) * (_layer) + (_offset) + (((_layer)&1)               \
                                            ? (-(_delta)/2.0)             \
                                            : (+(_delta)/2.0)))

#define TFC_Z_YLAYER_MM(_layer, _nominal, _offset, _delta)                \
          ((_nominal) * (_layer) + (_offset) + (((_layer)&1)              \
                                             ? (+(_delta)/2.0)            \
                                             : (-(_delta)/2.0)))
/* --------------------------------------------------------------------- */







/* --------------------------------------------------------------------  *\
 |                                                                       |  
 | PROJECTIONS FROM TKR to TOP ACD TILE FACE                             |
 | =========================================                             |   
 |                                                                       |  
 | This defines how to project from any TKR pair of layers to the ACD    |
 | TOP face.                                                             |
 |                                                                       |
\* --------------------------------------------------------------------- */


/* --------------------------------------------------------------------- *//*!

  \def   TFC_Z_ACD_TOP_TO_TKR_SCALE_FACTOR
  \brief Used when extending a projection to the plane of TOP ACD tiles.
  
  This exact scale factor is selected by the following criteria
  \verbatim
    1. The maximum projection error due to using integer arthimetic is
       less than one strip.
    2. The calculation never overflows 32-bit integer arthimetic
    3. The value is a multiple of two, so that removing the scale factor
       by division is avoided, using a shift operation instead.
  \endverbatim

  To extend a projection to the TOP ACD plane, a ratio of the Z distance
  from the TKR layer nearest the  TOP ACD plane and the TOP ACD plane
  to the Z separation of the nearest two TKR layers is needed. Since the
  calculation is done using integer arthimetic, this ratio needs to be
  scaled to preserve the accuracy.
                                                                         */
/* --------------------------------------------------------------------- */
#define TFC_Z_ACD_TOP_TO_TKR_SCALE_FACTOR 2048
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def    TFC_Z_ACD_TOP_TO_TKR_SCALE(_zratio)
  \brief  Scales the specified Z ration
  
  \param  _zratio The Z ratio to scale
  \return         The scaled Z ratio.
                                                                         *//*!
  \def    TFC_ACD_TOP_PROJECTION(_acd_top_mm, _tkr_top0_mm, _tkr_top1_mm)
  \brief  Produces the scaled ratio between the ACD TOP from TKR TOP distance
          and the TKR TOP to TKR TOP - 1 distance.

  \param _acd_top_mm    The absolute Z position of the ACD TOP plane in mm.
  \param _tkr_top0_mm   The absolute Z position of the nearest TKR plane in mm.
  \param _tkr_top1_mm   The absolute Z position of the next TKR plane in mm.
  \return               The scaled ratio
                                                                         */
/* --------------------------------------------------------------------- */  
#define TFC_Z_ACD_TOP_TO_TKR_SCALE(_zratio)                               \
        ((int)(((_zratio) * TFC_Z_ACD_TOP_TO_TKR_SCALE_FACTOR) + 0.5))


#define TFC_ACD_TOP_PROJECTION(_acd_top_mm, _tkr_top0_mm, _tkr_top1_mm)   \
        TFC_Z_ACD_TOP_TO_TKR_SCALE (((_acd_top_mm ) - (_tkr_top0_mm)) /   \
                                    ((_tkr_top0_mm) - (_tkr_top1_mm)))
/* --------------------------------------------------------------------- */






/* --------------------------------------------------------------------  *\
 |                                                                       |  
 | PROJECTIONS FROM TKR to SKIRT PLANE                                   |
 | =========================================                             |   
 |                                                                       |  
 | This defines how to project from any TKR pair of layers to the plane  |
 | of the skirt                                                          |
 |                                                                       |
\* --------------------------------------------------------------------- */


/* --------------------------------------------------------------------- *//*!

  \def   TFC_Z_TKR_TO_SKIRT_SCALE_FACTOR
  \brief Used when extending a projection to the plane of skirt (TKR/CAL
         boundary).
  
  This exact scale factor is selected by the following criteria
  \verbatim
    1. The maximum projection error due to using integer arthimetic is
       less than one strip.
    2. The calculation never overflows 32-bit integer arthimetic
    3. The value is a multiple of two, so that removing the scale factor
       by division is avoided, using a shift operation instead.
  \endverbatim

  To extend a projection to the skirt plane, a ratio of the Z distance
  from the TKR layer nearest the skirt plane and the skirt plane
  to the Z separation of the nearest two TKR layers is needed. Since the
  calculation is done using integer arthimetic, this ratio needs to be
  scaled to preserve the accuracy.
                                                                         */
/* --------------------------------------------------------------------- */
#define TFC_Z_TKR_TO_SKIRT_SCALE_FACTOR 2048
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def    TFC_Z_TKR_TO_SKIRT_SCALE(_zratio)
  \brief  Scales the specified Z ration
  
  \param  _zratio The Z ratio to scale
  \return         The scaled Z ratio.
                                                                         *//*!
  \def    TFC_SKIRT_PROJECTION(_skirt_mm, _tkr_top0_mm, _tkr_top1_mm)
  \brief  Produces the scaled ratio between the skirt plane from TKR TOP
          distance and the TKR TOP to TKR TOP - 1 distance.

  \param _skirt_mm      The absolute Z position of the skirt plane in mm.
  \param _tkr_top0_mm   The absolute Z position of the nearest TKR plane in mm.
  \param _tkr_top1_mm   The absolute Z position of the next TKR plane in mm.
  \return               The scaled ratio
                                                                         */
/* --------------------------------------------------------------------- */
#define TFC_Z_TKR_TO_SKIRT_SCALE(_z)                                      \
        ((int)(((_z) * TFC_Z_TKR_TO_SKIRT_SCALE_FACTOR) + 0.5))

#define TFC_SKIRT_PROJECTION(_skirt_mm, _tkr_top0_mm, _tkr_top1_mm)   \
        TFC_Z_TKR_TO_SKIRT_SCALE (((_tkr_top0_mm) - (_skirt_mm)) /     \
                                  ((_tkr_top0_mm) - (_tkr_top1_mm)))


#ifdef __cplusplus
}
#endif    



#endif
