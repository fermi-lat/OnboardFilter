#include "TFC_geometryDef.h"




/* --------------------------------------------------------------------- *\
 |                                                                       |
 | TKR XY TOWER GEOMETRY                                                 |
 | =====================                                                 |
 |                                                                       | 
 | These give the offsets to the first and last active strips in each    |
 | tower. It has not been assumed that towers are symmetric in X and Y.  |
 |                                                                       | 
 | The units of the transverse dimensions are strip units (228um).       |
 |                                                                       | 
\* --------------------------------------------------------------------- */
#define TKR_X_WIDTH 1563                     /* Effective width of tower */

#define TKR_X0_BEG -3235                     /* 1rst active Xstrip TKR 0 */
#define TKR_X1_BEG -1599                     /* 1rst active Xstrip TKR 1 */
#define TKR_X2_BEG    35                     /* 1rst active Xstrip TKR 2 */
#define TKR_X3_BEG  1671                     /* 1rst active Xstrip TKR 3 */

#define TKR_X0_END (TKR_X0_BEG + TKR_X_WIDTH) /*Last active Xstrip TKR 0 */
#define TKR_X1_END (TKR_X1_BEG + TKR_X_WIDTH) /*Last active Xstrip TKR 1 */
#define TKR_X2_END (TKR_X2_BEG + TKR_X_WIDTH) /*Last active Xstrip TKR 2 */
#define TKR_X3_END (TKR_X3_BEG + TKR_X_WIDTH) /*Last active Xstrip TKR 3 */


#define TKR_Y_WIDTH 1563                     /* Effective width of tower */

#define TKR_Y0_BEG -3235                     /* 1rst active Ystrip TKR 0 */
#define TKR_Y1_BEG -1599                     /* 1rst active Ystrip TKR 1 */
#define TKR_Y2_BEG    35                     /* 1rst active Ystrip TKR 2 */
#define TKR_Y3_BEG  1671                     /* 1rst active Ystrip TKR 3 */

#define TKR_Y0_END (TKR_Y0_BEG + TKR_Y_WIDTH) /*Last active Ystrip TKR 0 */
#define TKR_Y1_END (TKR_Y1_BEG + TKR_Y_WIDTH) /*Last active Ystrip TKR 1 */
#define TKR_Y2_END (TKR_Y2_BEG + TKR_Y_WIDTH) /*Last active Ystrip TKR 2 */
#define TKR_Y3_END (TKR_Y3_BEG + TKR_Y_WIDTH) /*Last active Ystrip TKR 3 */



/* --------------------------------------------------------------------- *\
 |                                                                       |
 | TKR Z TOWER GEOMETRY                                                  |
 | ====================                                                  |
 |                                                                       | 
 | This defines the Z geometry of the TKR.                               |
 |                                                                       |
 | The input units are mm,but the working units are appropriately scaled.|
 | The scaling varies by usage to keep the arthimetic under control.     |
\* --------------------------------------------------------------------- */
#define TKR_Z_OFFSET_MM  -65.804             /* Layer 0 Z coordinate     */ 
#define TKR_Z_NOMINAL_MM  32.402             /* Nominal layer spacing mm */
#define TKR_Z_DELTA_MM     2.400             /* Separation of XY layers  */
#define TKR_Z_NOMINAL     TFC_Z_ABS_SCALE(TKR_Z_NOMINAL_MM)



/* --------------------------------------------------------------------- *\
 |                                                                       |
 | Defines the absolute Z coordinate of any given X layer in mm.         |
 | and in units of TFC_Z_ABS_SCALE_FACTOR.                               |
 |                                                                       |
\* --------------------------------------------------------------------- */ 
#define TKR_Z_XLAYER_MM(_layer) TFC_Z_XLAYER_MM(_layer, TKR_Z_NOMINAL_MM, \
                                                        TKR_Z_OFFSET_MM,  \
                                                        TKR_Z_DELTA_MM)   \


#define TKR_Z_XLAYER(_layer)    TFC_Z_ABS_SCALE(TKR_Z_XLAYER_MM(_layer))




       /* -------------------------------------------------------------- *\
        |                                                                |
        | Defines the absolute Z coordinate of any given Y layer in mm.  |
        |                                                                |
       \* -------------------------------------------------------------- */ 
#define TKR_Z_YLAYER_MM(_layer) TFC_Z_YLAYER_MM(_layer, TKR_Z_NOMINAL_MM,\
                                                        TKR_Z_OFFSET_MM, \
                                                        TKR_Z_DELTA_MM)


#define TKR_Z_YLAYER(_layer)    TFC_Z_ABS_SCALE(TKR_Z_YLAYER_MM(_layer))



/* --------------------------------------------------------------------- */







/* --------------------------------------------------------------------- *\
 |                                                                       |  
 | TOP ACD TILE GEOMETRY                                                 |
 | =====================                                                 |   
 | The following numbers define the limits of X,Y for the top ACD tiles. |
 | The setup is symmetric so the numbers  apply to both the X and Y.     |
 | The units are the strip pitch (.228mm)                                |
 |                                                                       | 
\* --------------------------------------------------------------------- */
#define ACD_TOP_0      -3687                    /* Tile 0 -3687 to -2256 */
#define ACD_TOP_1      -2256                    /* Tile 1 -2256 to  -780 */
#define ACD_TOP_2       -780                    /* Tile 2  -780 to   694 */
#define ACD_TOP_3        694                    /* Tile 3   694 to  2169 */
#define ACD_TOP_4       2169                    /* Tile 4  2169 to  3688 */
#define ACD_TOP_5       3688                    /* Tile 4  extreme edge  */
#define ACD_TOP_Z_MM 575.248                    /* Z coordinate in mm    */
#define ACD_TOP_Z    TFC_Z_ABS_SCALE(ACD_TOP_Z_MM)   /* Z coordinate     */
/* --------------------------------------------------------------------- */


 


/* --------------------------------------------------------------------  *\
 |                                                                       |  
 | X SIDE ACD TILE GEOMETRY                                              |
 | ========================                                              |   
 |                                                                       |  
 | The following numbers define the limits of Y coordinates for the X+   |
 | and X- ACD side tiles. In addition the distance between the nearest   |
 | tower face to the X+ or X- ACD face is defined. This allows one to    |
 | extrapolate from the X position in the tower to the ACD face.         |
 | It is assumed that the tiles are infinitely close, ie the end of one  |
 | tile is the same as the begining of the next tile.                    |
 | The units are the strip pitch (.228mm).                               |
 |                                                                       | 
\* --------------------------------------------------------------------- */ 
#define ACD_X_Y0         -3687                  /* Tile 0 -3687 to -2212 */
#define ACD_X_Y1         -2212                  /* Tile 1 -2212 to  -737 */
#define ACD_X_Y2          -737                  /* Tile 2  -737 to   738 */
#define ACD_X_Y3           738                  /* Tile 3   738 to  2213 */
#define ACD_X_Y4          2213                  /* Tile 4  2213 to  3688 */
#define ACD_X_Y5          3688                  /* Tile 4 extreme edge   */

#define ACD_XM_FACE      -3731                  /* X coordinate, XM face */
#define ACD_XP_FACE       3732                  /* X coordinate, XP face */

#define ACD_XM_FROM_TKR  (ACD_XM_FACE - TKR_X0_BEG)    /* TKR to XM face */
#define ACD_XP_FROM_TKR  (ACD_XP_FACE - TKR_X3_END)    /* TKR to XP face */
/* --------------------------------------------------------------------- */ 







/* --------------------------------------------------------------------  *\
 |                                                                       |  
 | Y SIDE ACD TILE GEOMETRY                                              |
 | ========================                                              |   
 |                                                                       |  
 | The following numbers define the limits of X coordinates for the Y+   |
 | and Y- ACD side tiles. In addition the distance between the nearest   |
 | tower face to the Y+ or Y- ACD face is defined. This allows one to    |
 | extrapolate from the Y position in the tower to the ACD face.         |
 | It is assumed that the tiles are infinitely close, ie the end of one  |
 | tile is the same as the begining of the next tile.                    |
 | The units are the strip pitch (.228mm).                               |
\* --------------------------------------------------------------------- */ 
#define ACD_Y_X0        -3731                   /* Tile 0 -3731 to -2238 */
#define ACD_Y_X1        -2238                   /* Tile 1 -2238 to  -745 */
#define ACD_Y_X2         -745                   /* Tile 2  -745 to   746 */
#define ACD_Y_X3          746                   /* Tile 3   746 to  2239 */
#define ACD_Y_X4         2239                   /* Tile 4  2239 to  3732 */
#define ACD_Y_X5         3732                   /* Tile 4 extreme edge   */

#define ACD_YM_FACE     -3687                   /* Y coordinate, YM face */
#define ACD_YP_FACE      3688                   /* Y coordinate, YP face */

#define ACD_YM_FROM_TKR (ACD_YM_FACE - TKR_Y0_BEG)    /* TKR to YM face  */
#define ACD_YP_FROM_TKR (ACD_YP_FACE - TKR_Y3_END)    /* TKR to YP face  */
/* --------------------------------------------------------------------- */ 





/* --------------------------------------------------------------------  *\
 |                                                                       |  
 | X,Y SIDE ACD TILE GEOMETRY, Z COORDINATES                             |
 | =========================================                             |   
 |                                                                       |  
 | The following numbers define the limits of the Z coordinates.         |
 | for the ACD side tiles. These apply to all for ACD side faces.        |
 | the units 1mm = ZSCALE_FACTOR.                                        |
 |                                                                       |
\* --------------------------------------------------------------------- */
#define ACD_SIDE_Z0_MM  612.948              /* ACD side row 0, top Z mm */
#define ACD_SIDE_Z1_MM  345.448              /* ACD side row 1, top Z mm */
#define ACD_SIDE_Z2_MM  145.448              /* ACD side row 2, top Z mm */
#define ACD_SIDE_Z3_MM   -4.552              /* ACD side row 3, top Z mm */
#define ACD_SIDE_Z4_MM -154.552              /* ACD side row 3, bot Z mm */
#define ACD_SIDE_Z0 TFC_Z_ABS_SCALE(ACD_SIDE_Z0_MM) /* Side row 0 top Z  */
#define ACD_SIDE_Z1 TFC_Z_ABS_SCALE(ACD_SIDE_Z1_MM) /* Side row 1 top Z  */
#define ACD_SIDE_Z2 TFC_Z_ABS_SCALE(ACD_SIDE_Z2_MM) /* Side row 2 top Z  */
#define ACD_SIDE_Z3 TFC_Z_ABS_SCALE(ACD_SIDE_Z3_MM) /* Side row 3 top Z  */
#define ACD_SIDE_Z4 TFC_Z_ABS_SCALE(ACD_SIDE_Z4_MM) /* Side row 3 bot Z  */
/* --------------------------------------------------------------------- */ 






/* --------------------------------------------------------------------  *\
 |                                                                       |  
 | PROJECTIONS FROM TKR to TOP ACD TILE FACE                             |
 | =========================================                             |   
 |                                                                       |  
 | This defines how to project from any TKR layer to the ACD TOP face.   |
 |                                                                       |
\* --------------------------------------------------------------------- */
#define ACD_TOP_TKR_ZSCALE 2048           /* Used to scale Z ratio      */

#define ACD_TOP_XPROJECTION(_layer)                                      \
    TFC_ACD_TOP_PROJECTION(ACD_TOP_Z_MM,                                 \
                           TKR_Z_XLAYER_MM(_layer),                      \
                           TKR_Z_XLAYER_MM(_layer-1))
    
#define ACD_TOP_YPROJECTION(_layer)                                      \
    TFC_ACD_TOP_PROJECTION(ACD_TOP_Z_MM,                                 \
                           TKR_Z_YLAYER_MM(_layer),                      \
                           TKR_Z_YLAYER_MM(_layer-1))


/* --------------------------------------------------------------------- */



/*
 |  Gives the scale factor to project to the ACD. The index is the top layer
 |  of the tracker to project from. That is
 |
 |  X_at_acd =  X_top_layer
 |           + (X_top_layer - X_top_layer-1) * AcdXProjection[top_layer]
 |           / ACD_TOP_TKR_ZSCALE;
*/


static const TFC_geometry TFC_Geometry =
{
  {                       /* TKR Geometry                      */
    {                     /* X Layer Geometry                  */
     {                    /* X Offset to beginning of column n */ 
       TKR_X0_BEG,        /* X Offset to beginning of column 0 */
       TKR_X1_BEG,        /* X Offset to beginning of column 1 */
       TKR_X2_BEG,        /* X Offset to beginning of column 2 */
       TKR_X3_BEG         /* X Offset to beginning of column 3 */
     },
     {                    /* Z position of layer  Xn           */
       TKR_Z_XLAYER( 0),  /* Z position of layer  X0           */
       TKR_Z_XLAYER( 1),  /* Z position of layer  X1           */
       TKR_Z_XLAYER( 2),  /* Z position of layer  X2           */
       TKR_Z_XLAYER( 3),  /* Z position of layer  X3           */
       TKR_Z_XLAYER( 4),  /* Z position of layer  X4           */
       TKR_Z_XLAYER( 5),  /* Z position of layer  X5           */
       TKR_Z_XLAYER( 6),  /* Z position of layer  X6           */
       TKR_Z_XLAYER( 7),  /* Z position of layer  X7           */
       TKR_Z_XLAYER( 8),  /* Z position of layer  X8           */
       TKR_Z_XLAYER( 9),  /* Z position of layer  X9           */
       TKR_Z_XLAYER(10),  /* Z position of layer X10           */       
       TKR_Z_XLAYER(11),  /* Z position of layer X11           */
       TKR_Z_XLAYER(12),  /* Z position of layer X12           */
       TKR_Z_XLAYER(13),  /* Z position of layer X13           */
       TKR_Z_XLAYER(14),  /* Z position of layer X14           */
       TKR_Z_XLAYER(15),  /* Z position of layer X15           */
       TKR_Z_XLAYER(16),  /* Z position of layer X16           */
       TKR_Z_XLAYER(17),  /* Z position of layer X17           */
     }
   },
  
   {                      /* Y Layer geometry                  */
     {                    /* Y Offset to beginning of row n    */
        TKR_Y0_BEG,       /* Y Offset to beginning of row 0    */
        TKR_Y1_BEG,       /* Y Offset to beginning of row 1    */
        TKR_Y2_BEG,       /* Y Offset to beginning of row 2    */
        TKR_Y3_BEG        /* Y Offset to beginning of row 3    */
     },
     {                    /* Z position of layer  Yn           */
       TKR_Z_YLAYER( 0),  /* Z position of layer  Y0           */
       TKR_Z_YLAYER( 1),  /* Z position of layer  Y1           */
       TKR_Z_YLAYER( 2),  /* Z position of layer  Y2           */
       TKR_Z_YLAYER( 3),  /* Z position of layer  Y3           */
       TKR_Z_YLAYER( 4),  /* Z position of layer  Y4           */
       TKR_Z_YLAYER( 5),  /* Z position of layer  Y5           */
       TKR_Z_YLAYER( 6),  /* Z position of layer  Y6           */
       TKR_Z_YLAYER( 7),  /* Z position of layer  Y7           */
       TKR_Z_YLAYER( 8),  /* Z position of layer  Y8           */
       TKR_Z_YLAYER( 9),  /* Z position of layer  Y9           */
       TKR_Z_YLAYER(10),  /* Z position of layer Y10           */       
       TKR_Z_YLAYER(11),  /* Z position of layer Y11           */
       TKR_Z_YLAYER(12),  /* Z position of layer Y12           */
       TKR_Z_YLAYER(13),  /* Z position of layer Y13           */
       TKR_Z_YLAYER(14),  /* Z position of layer Y14           */
       TKR_Z_YLAYER(15),  /* Z position of layer Y15           */
       TKR_Z_YLAYER(16),  /* Z position of layer Y16           */
       TKR_Z_YLAYER(17),  /* Z position of layer Y17           */
     }
  },
  {                       /* ACD Geometry                      */
    {

       0,   /* -- Unused */
       0,   /* -- Unused */
       ACD_TOP_XPROJECTION( 2),   /* Extend from  2 to ACD */
       ACD_TOP_XPROJECTION( 3),   /* Extend from  3 to ACD */
       ACD_TOP_XPROJECTION( 4),   /* Extend from  4 to ACD */
       ACD_TOP_XPROJECTION( 5),   /* Extend from  5 to ACD */
       ACD_TOP_XPROJECTION( 6),   /* Extend from  6 to ACD */
       ACD_TOP_XPROJECTION( 7),   /* Extend from  7 to ACD */
       ACD_TOP_XPROJECTION( 8),   /* Extend from  8 to ACD */
       ACD_TOP_XPROJECTION( 9),   /* Extend from  9 to ACD */
       ACD_TOP_XPROJECTION(10),   /* Extend from 10 to ACD */
       ACD_TOP_XPROJECTION(11),   /* Extend from 11 to ACD */
       ACD_TOP_XPROJECTION(12),   /* Extend from 12 to ACD */
       ACD_TOP_XPROJECTION(13),   /* Extend from 13 to ACD */
       ACD_TOP_XPROJECTION(14),   /* Extend from 14 to ACD */
       ACD_TOP_XPROJECTION(15),   /* Extend from 15 to ACD */
       ACD_TOP_XPROJECTION(16),   /* Extend from 16 to ACD */     
       ACD_TOP_XPROJECTION(17),   /* Extend from 17 to ACD */
    },
    {                             /* ACD TOP PROJECTION    */ 
       0,   /* -- Unused */
       0,   /* -- Unused */
       ACD_TOP_YPROJECTION( 2),   /* Extend from  2 to ACD */
       ACD_TOP_YPROJECTION( 3),   /* Extend from  3 to ACD */
       ACD_TOP_YPROJECTION( 4),   /* Extend from  4 to ACD */
       ACD_TOP_YPROJECTION( 5),   /* Extend from  5 to ACD */
       ACD_TOP_YPROJECTION( 6),   /* Extend from  6 to ACD */
       ACD_TOP_YPROJECTION( 7),   /* Extend from  7 to ACD */
       ACD_TOP_YPROJECTION( 8),   /* Extend from  8 to ACD */
       ACD_TOP_YPROJECTION( 9),   /* Extend from  9 to ACD */
       ACD_TOP_YPROJECTION(10),   /* Extend from 10 to ACD */
       ACD_TOP_YPROJECTION(11),   /* Extend from 11 to ACD */
       ACD_TOP_YPROJECTION(12),   /* Extend from 12 to ACD */
       ACD_TOP_YPROJECTION(13),   /* Extend from 13 to ACD */
       ACD_TOP_YPROJECTION(14),   /* Extend from 14 to ACD */
       ACD_TOP_YPROJECTION(15),   /* Extend from 15 to ACD */
       ACD_TOP_YPROJECTION(16),   /* Extend from 16 to ACD */     
       ACD_TOP_YPROJECTION(17),   /* Extend from 17 to ACD */
    }
  }
};
