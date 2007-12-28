#ifndef LatGeometry_h
#define LatGeometry_h

//#include "OnboardFilter/TFC_geometry_v1r13p0.c"
//#include "GGF_DB/GEO_DB_data.h"

#define TFC_X_CMT    TFC_TAGCMT(TFC_K_TAGTYPE_GLEAM, 1, 13, 0)
#define TFC_X_CREATE TFC_TAGDATE  (1,  9, 2004)
#define TFC_X_REVISE TFC_TAGDATE  (1,  9, 2004)
#define TFC_X_TAG    TFC_TAG      ( TFC_K_GEO_ID_V1_R13_P0, \
                                    TFC_X_CMT,              \
                                    TFC_X_CREATE,           \
                                    TFC_X_REVISE)



/* --------------------------------------------------------------------- *\
 |                                                                       |
 | TKR TOWER GEOMETRY                                                    |
 | ==================                                                    |
 |                                                                       | 
 | These give the offsets to the first and last active strips in each    |
 | tower. It has not been assumed that towers are symmetric in X and Y.  |
 |                                                                       | 
 | The units of the transverse dimensions are strip units (228um).       |
 |                                                                       | 
\* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *\
 |                                                                       |
 |  TKR Fundamental Constants                                            |
 |                                                                       |
\* --------------------------------------------------------------------- */
#define TKR_K_STRIP_MM            0.228      /* Strip width (mm)         */
#define TKR_XY_WIDTH_MM          360.60      /* Width of tracker towers  */

/* TKR X Fundamental constants                                           */
#define TKR_X0_MID_MM           -561.75      /* Center of COL = 0 towers */
#define TKR_X1_MID_MM           -187.25      /* Center of COL = 1 towers */
#define TKR_X2_MID_MM            187.25      /* Center of COL = 2 towers */
#define TKR_X3_MID_MM            561.75      /* Center of COL = 3 towers */


/* TKR Y Fundamental constants                                           */
#define TKR_Y0_MID_MM           -561.75      /* Center of ROW = 0 towers */
#define TKR_Y1_MID_MM           -187.25      /* Center of ROW = 1 towers */
#define TKR_Y2_MID_MM            187.25      /* Center of ROW = 2 towers */
#define TKR_Y3_MID_MM            561.75      /* Center of ROW = 3 towers */


/* TKR Z Fundamental constants                                           */
#define TKR_Z_NOMINAL_MM        29.775        /* Nominal layer spacing mm */
#define TKR_Z_DELTA_MM           2.127      /* Separation of XY layers  */

#define TKR_Z_YLAYER_00_MM      42.315
#define TKR_Z_XLAYER_00_MM      44.765

#define TKR_Z_XLAYER_01_MM      74.335
#define TKR_Z_YLAYER_01_MM      76.885

#define TKR_Z_YLAYER_02_MM     106.435
#define TKR_Z_XLAYER_02_MM     108.965

#define TKR_Z_XLAYER_03_MM     139.538
#define TKR_Z_YLAYER_03_MM     142.065

#define TKR_Z_YLAYER_04_MM     172.638
#define TKR_Z_XLAYER_04_MM     175.165

#define TKR_Z_XLAYER_05_MM     205.738
#define TKR_Z_YLAYER_05_MM     208.265

#define TKR_Z_YLAYER_06_MM     238.838
#define TKR_Z_XLAYER_06_MM     241.365

#define TKR_Z_XLAYER_07_MM     271.140
#define TKR_Z_YLAYER_07_MM     273.665

#define TKR_Z_YLAYER_08_MM     303.440
#define TKR_Z_XLAYER_08_MM     305.965

#define TKR_Z_XLAYER_09_MM     335.740
#define TKR_Z_YLAYER_09_MM     338.265

#define TKR_Z_YLAYER_10_MM     368.040
#define TKR_Z_XLAYER_10_MM     370.565

#define TKR_Z_XLAYER_11_MM     400.340
#define TKR_Z_YLAYER_11_MM     402.865

#define TKR_Z_YLAYER_12_MM     432.640
#define TKR_Z_XLAYER_12_MM     435.165

#define TKR_Z_XLAYER_13_MM     464.940
#define TKR_Z_YLAYER_13_MM     467.465

#define TKR_Z_YLAYER_14_MM     497.240
#define TKR_Z_XLAYER_14_MM     499.765

#define TKR_Z_XLAYER_15_MM     529.540
#define TKR_Z_YLAYER_15_MM     532.065

#define TKR_Z_YLAYER_16_MM     561.840
#define TKR_Z_XLAYER_16_MM     564.365

#define TKR_Z_XLAYER_17_MM     594.14
#define TKR_Z_YLAYER_17_MM     596.665



/* --------------------------------------------------------------------- *\
 |                                                                       |
 |  TKR Derived Constants                                                |
 |                                                                       |
\* --------------------------------------------------------------------- */

#define TKR_K_STRIP   (TKR_K_STRIP_MM * 1000 + 0.5)
#define TKR_XY_WIDTH   TFC_XY_SCALE(TKR_XY_WIDTH_MM, TKR_K_STRIP_MM)


/* TKR X Derived Constants                                               */
#define TKR_X0_BEG_MM (TKR_X0_MID_MM    - TKR_XY_WIDTH_MM/2.0)
#define TKR_X1_BEG_MM (TKR_X1_MID_MM    - TKR_XY_WIDTH_MM/2.0)
#define TKR_X2_BEG_MM (TKR_X2_MID_MM    - TKR_XY_WIDTH_MM/2.0)
#define TKR_X3_BEG_MM (TKR_X3_MID_MM    - TKR_XY_WIDTH_MM/2.0)

#define TKR_X0_END_MM (TKR_X0_MID_MM    + TKR_XY_WIDTH_MM/2.0)
#define TKR_X1_END_MM (TKR_X1_MID_MM    + TKR_XY_WIDTH_MM/2.0)
#define TKR_X2_END_MM (TKR_X2_MID_MM    + TKR_XY_WIDTH_MM/2.0)
#define TKR_X3_END_MM (TKR_X3_MID_MM    + TKR_XY_WIDTH_MM/2.0)


#define TKR_X0_BEG     TFC_XY_SCALE(TKR_X0_BEG_MM,    TKR_K_STRIP_MM)
#define TKR_X1_BEG     TFC_XY_SCALE(TKR_X1_BEG_MM,    TKR_K_STRIP_MM)
#define TKR_X2_BEG     TFC_XY_SCALE(TKR_X2_BEG_MM,    TKR_K_STRIP_MM)
#define TKR_X3_BEG     TFC_XY_SCALE(TKR_X3_BEG_MM,    TKR_K_STRIP_MM)

#define TKR_X0_END     TFC_XY_SCALE(TKR_X0_END_MM,    TKR_K_STRIP_MM)
#define TKR_X1_END     TFC_XY_SCALE(TKR_X1_END_MM,    TKR_K_STRIP_MM)
#define TKR_X2_END     TFC_XY_SCALE(TKR_X2_END_MM,    TKR_K_STRIP_MM)
#define TKR_X3_END     TFC_XY_SCALE(TKR_X3_END_MM,    TKR_K_STRIP_MM)


/* TKR Y Derived Constants                                               */
#define TKR_Y0_BEG_MM (TKR_X0_MID_MM    - TKR_XY_WIDTH_MM/2.0)
#define TKR_Y1_BEG_MM (TKR_X1_MID_MM    - TKR_XY_WIDTH_MM/2.0)
#define TKR_Y2_BEG_MM (TKR_X2_MID_MM    - TKR_XY_WIDTH_MM/2.0)
#define TKR_Y3_BEG_MM (TKR_X3_MID_MM    - TKR_XY_WIDTH_MM/2.0)

#define TKR_Y0_END_MM (TKR_X0_MID_MM    + TKR_XY_WIDTH_MM/2.0)
#define TKR_Y1_END_MM (TKR_X1_MID_MM    + TKR_XY_WIDTH_MM/2.0)
#define TKR_Y2_END_MM (TKR_X2_MID_MM    + TKR_XY_WIDTH_MM/2.0)
#define TKR_Y3_END_MM (TKR_X3_MID_MM    + TKR_XY_WIDTH_MM/2.0)



#define TKR_Y0_BEG     TFC_XY_SCALE(TKR_Y0_BEG_MM,    TKR_K_STRIP_MM)
#define TKR_Y1_BEG     TFC_XY_SCALE(TKR_Y1_BEG_MM,    TKR_K_STRIP_MM)
#define TKR_Y2_BEG     TFC_XY_SCALE(TKR_Y2_BEG_MM,    TKR_K_STRIP_MM)
#define TKR_Y3_BEG     TFC_XY_SCALE(TKR_Y3_BEG_MM,    TKR_K_STRIP_MM)

#define TKR_Y0_END     TFC_XY_SCALE(TKR_Y0_END_MM,    TKR_K_STRIP_MM)
#define TKR_Y1_END     TFC_XY_SCALE(TKR_Y1_END_MM,    TKR_K_STRIP_MM)
#define TKR_Y2_END     TFC_XY_SCALE(TKR_Y2_END_MM,    TKR_K_STRIP_MM)
#define TKR_Y3_END     TFC_XY_SCALE(TKR_Y3_END_MM,    TKR_K_STRIP_MM)


/* TKR Z Derived Constants                                               */
#define TKR_Z_XLAYER_00       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_00_MM)
#define TKR_Z_XLAYER_01       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_01_MM)
#define TKR_Z_XLAYER_02       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_02_MM)
#define TKR_Z_XLAYER_03       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_03_MM)
#define TKR_Z_XLAYER_04       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_04_MM)
#define TKR_Z_XLAYER_05       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_05_MM)
#define TKR_Z_XLAYER_06       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_06_MM)
#define TKR_Z_XLAYER_07       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_07_MM)
#define TKR_Z_XLAYER_08       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_08_MM)
#define TKR_Z_XLAYER_09       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_09_MM)
#define TKR_Z_XLAYER_10       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_10_MM)
#define TKR_Z_XLAYER_11       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_11_MM)
#define TKR_Z_XLAYER_12       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_12_MM)
#define TKR_Z_XLAYER_13       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_13_MM)
#define TKR_Z_XLAYER_14       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_14_MM)
#define TKR_Z_XLAYER_15       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_15_MM)
#define TKR_Z_XLAYER_16       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_16_MM)
#define TKR_Z_XLAYER_17       TFC_Z_ABS_SCALE(TKR_Z_XLAYER_17_MM)

#define TKR_Z_YLAYER_00       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_00_MM)
#define TKR_Z_YLAYER_01       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_01_MM)
#define TKR_Z_YLAYER_02       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_02_MM)
#define TKR_Z_YLAYER_03       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_03_MM)
#define TKR_Z_YLAYER_04       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_04_MM)
#define TKR_Z_YLAYER_05       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_05_MM)
#define TKR_Z_YLAYER_06       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_06_MM)
#define TKR_Z_YLAYER_07       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_07_MM)
#define TKR_Z_YLAYER_08       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_08_MM)
#define TKR_Z_YLAYER_09       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_09_MM)
#define TKR_Z_YLAYER_10       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_10_MM)
#define TKR_Z_YLAYER_11       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_11_MM)
#define TKR_Z_YLAYER_12       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_12_MM)
#define TKR_Z_YLAYER_13       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_13_MM)
#define TKR_Z_YLAYER_14       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_14_MM)
#define TKR_Z_YLAYER_15       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_15_MM)
#define TKR_Z_YLAYER_16       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_16_MM)
#define TKR_Z_YLAYER_17       TFC_Z_ABS_SCALE(TKR_Z_YLAYER_17_MM)




#define TKR_Z_NOMINAL     TFC_Z_ABS_SCALE(TKR_Z_NOMINAL_MM)
#define TKR_Z_NOMINALx2   TFC_Z_ABS_SCALE(TKR_Z_NOMINAL_MM * 2.0)




/* --------------------------------------------------------------------- *\
 |                                                                      |
 | Defines the absolute Z coordinate of any given X layer in mm.        |
 | and in units of TFC_Z_ABS_SCALE_FACTOR                               |
 |                                                                      |
\* -------------------------------------------------------------------- */ 
#define TKR_Z_XLAYER_MM(_layer)   TKR_Z_XLAYER_ ## _layer ## _MM
#define TKR_Z_XLAYER(_layer)      TFC_Z_ABS_SCALE(TKR_Z_XLAYER_MM(_layer))



/* --------------------------------------------------------------------- *\
 |                                                                       |
 | Defines the absolute Z coordinate of any given Y layer in mm.         |
 |                                                                       |
\* -------------------------------------------------------------------- */ 
#define TKR_Z_YLAYER_MM(_layer)  TKR_Z_YLAYER_ ## _layer ## _MM
#define TKR_Z_YLAYER(_layer)     TFC_Z_ABS_SCALE(TKR_Z_YLAYER_MM(_layer))



/* --------------------------------------------------------------------- */

#define TKR_Z_FIND_MAX   TFC_Z_FIND_MAX(TKR_Z_NOMINAL_MM, TKR_Z_DELTA_MM)
#define TKR_Z_FIND_MIN   TFC_Z_FIND_MIN(TKR_Z_NOMINAL_MM, TKR_Z_DELTA_MM)

#define TKR_Z_EXTEND_MAX TFC_Z_EXTEND_MAX(TKR_Z_NOMINAL_MM, TKR_Z_DELTA_MM)
#define TKR_Z_EXTEND_MIN TFC_Z_EXTEND_MIN(TKR_Z_NOMINAL_MM, TKR_Z_DELTA_MM)




/* --------------------------------------------------------------------- *\
 |                                                                       |  
 | TOP ACD TILE GEOMETRY                                                 |
 | =====================                                                 |   
 | The following numbers define the limits of X,Y for the top ACD tiles. |
 |                                                                       | 
\* --------------------------------------------------------------------- */

/* Middle of tile 2 z */
#define ACD_TOP_Z_MM 754.6

/*
 | X TILE - Simplification
 | -----------------------
 | Tile 0 and tile 4 include the 50 mm corner piece. Each X tile overlaps
 | its neighbor. We are going to arbitrarily decide that each tile ends/begins
 | at the halfway point of the overlap. This assumption leads to not giving
 | two chances to paricles that pass in the overlap region, but a 99+%
 | efficiency, this should be good enough for the filter.
 |
 | Y TILE - Simplification
 | -----------------------
 | There is a gap between the Y tiles. We are going to arbitrarily decide
 | that each tile ends/begins at the halfway point in the gap. This 
 | simplification could be a little more dangerous, allowing particles
 | that point into the gap to be capriously tested for a tile hit. The
 | degree of damage depends on the noise rate. That is, if the tiles are
 | quiet, the test will simply fail.
*/
                                     /* X tile widths                  */ 
#define ACD_X_TOP_0_WIDTH_MM 355.00  /* Col 0 width                    */
#define ACD_X_TOP_1_WIDTH_MM 344.00  /* Col 1 width                    */
#define ACD_X_TOP_2_WIDTH_MM 344.00  /* Col 2 width                    */
#define ACD_X_TOP_3_WIDTH_MM 344.00  /* Col 3 width                    */
#define ACD_X_TOP_4_WIDTH_MM 355.00  /* Col 4 width                    */

                                      /* Y tile widths                  */
#define ACD_Y_TOP_0_WIDTH_MM 329.67   /* Row 0 width                    */
#define ACD_Y_TOP_1_WIDTH_MM 332.00   /* Row 1 width                    */
#define ACD_Y_TOP_2_WIDTH_MM 332.00   /* Row 2 width                    */
#define ACD_Y_TOP_3_WIDTH_MM 332.00   /* Row 3 width                    */
#define ACD_Y_TOP_4_WIDTH_MM 329.67   /* Row 4 width                    */


#define ACD_X_TOP_OVERLAP_MM  20.00   /* Columns overlap                */
#define ACD_Y_TOP_GAP_MM       2.00   /* Row gap,covered by the ribbons */
#define ACD_X_TOP_OFFSET_MM    0.00   /* Middle of X tile 3             */
#define ACD_Y_TOP_OFFSET_MM    0.00   /* Middle of Y tile 3             */ 


/* --------------------------------------------------------------------- */


 


/* --------------------------------------------------------------------  *\
 |                                                                       |  
 | SIDE ACD TILE GEOMETRY                                                |
 | ======================                                                |   
 |                                                                       |  
 | The following numbers define the limits of Y coordinates for the X+   |
 | and X- ACD side tiles. In addition the distance between the nearest   |
 | tower face to the X+ or X- ACD face is defined. This allows one to    |
 | extrapolate from the X position in the tower to the ACD face.         |
 | It is assumed that the tiles are infinitely close, ie the end of one  |
 | tile is the same as the begining of the next tile.                    |
 |                                                                       |
 | The ACD faces are no longer symmetric. The picture (looking down from |
 | the top (i.e. eyesight along the -z axis                              |
 |                                                                       |
 |                                                                       |
 |                           FACE 4                  X+                  |
 |      +-----------+-------+-------+-------+-------+--+                 |
 |  Y+  |     0     |   1   |   2   |   3   |   4   |  |                 |
 |      +--+--------+-------+-------+-------+-------+  +                 |
 |      |  |                                        |  |                 |
 |      | 4|                                        | 4|                 |
 |      |__|                                        |__|                 |
 |      |  |                    ^                   |  |                 |
 |      | 3|                    |                   | 3|                 |
 |  F   |__|                 Y  |                   |__|  F              |
 |  A   |  |                    |                   |  |  A              |
 |  C   | 2|                    +------>            | 2|  C              |
 |  E   |__|                       X                |__|  E              |
 |      |  |                                        |  |                 |
 |  1   | 1|                                        | 1|  3              |
 |      |__|                                        |__|                 |
 |      |  |                                        |  |                 |
 |      | 0|                                        | 0|                 |
 |      |  |                                        |  |                 |
 |      |  +--------+-------+-------+-------+----------+                 |
 |      |  |    0   |   1   |   2   |   3   |    4     |  Y-             |
 |      +--+--------+-------+-------+-------+----------+                 |
 |       X-                  FACE 2                                      |
 |                                                                       |
 | From this diagram, one sees that the tile 0 in the X- plane (FACE 1)  |
 | the Y+ plane (FACE 4) are longer than the tile 0 in the X+ plane      |
 | (FACE 3) and the Y- planes (FACE 4). Although a symmetry may exist    |
 | between the faces, it is ignored; all faces are entered as though     |
 | they have nothing to do with each other other                         |
 |                                                                       | 
\* --------------------------------------------------------------------- */


                                       /* X and Y common definitions     */
#define ACD_VERTICAL_OVERLAP_MM   0.0  /* Rows are shingled              */
#define ACD_HORIZONTAL_GAP_MM     2.0  /* Cols have gaps filled by rib's */

                                       /* Z widths                       */
#define ACD_SIDE_Z0_WIDTH_MM   366.00  /* Row 0 width                    */
#define ACD_SIDE_Z1_WIDTH_MM   199.46  /* Row 1 width                    */
#define ACD_SIDE_Z2_WIDTH_MM   150.00  /* Row 2 width                    */  
#define ACD_SIDE_Z3_WIDTH_MM   135.35  /* Row 3 width                    */

#define ACD_SIDE_Z_OVERLAP_MM    0.00  /* All rows overlap the same, none*/
#define ACD_SIDE_Z0_TOP_MM     802.60  /* Top of the top side tile       */


                                       /* X- tile widths                 */
#define ACD_XM_Y0_WIDTH_MM     342.97  /* X- Tile 0 width                */
#define ACD_XM_Y1_WIDTH_MM     332.00  /* X- Tile 1 width                */
#define ACD_XM_Y2_WIDTH_MM     332.00  /* X- Tile 2 width                */
#define ACD_XM_Y3_WIDTH_MM     332.00  /* X- Tile 3 width                */
#define ACD_XM_Y4_WIDTH_MM     329.67  /* X- Tile 4 width                */

                                       /* X+ tile widths                 */
#define ACD_XP_Y0_WIDTH_MM     329.67  /* X+ Tile 0 width                */
#define ACD_XP_Y1_WIDTH_MM     332.00  /* X+ Tile 1 width                */
#define ACD_XP_Y2_WIDTH_MM     332.00  /* X+ Tile 2 width                */
#define ACD_XP_Y3_WIDTH_MM     332.00  /* X+ Tile 3 width                */
#define ACD_XP_Y4_WIDTH_MM     342.97  /* X+ Tile 4 width                */

                                       /* Y- tile widths                 */
#define ACD_YM_X0_WIDTH_MM     329.67  /* Y- Tile 0 width                */
#define ACD_YM_X1_WIDTH_MM     332.00  /* Y- Tile 1 width                */
#define ACD_YM_X2_WIDTH_MM     332.00  /* Y- Tile 2 width                */
#define ACD_YM_X3_WIDTH_MM     332.00  /* Y- Tile 3 width                */
#define ACD_YM_X4_WIDTH_MM     342.97  /* Y- Tile 4 width                */

                                       /* Y+ tile widths                 */
#define ACD_YP_X0_WIDTH_MM     342.97  /* Y+ Tile 0 width                */
#define ACD_YP_X1_WIDTH_MM     332.00  /* Y+ Tile 1 width                */
#define ACD_YP_X2_WIDTH_MM     332.00  /* Y+ Tile 2 width                */
#define ACD_YP_X3_WIDTH_MM     332.00  /* Y+ Tile 3 width                */
#define ACD_YP_X4_WIDTH_MM     329.67  /* Y+ Tile 4 width                */


                                       /* Offsets to center of tile 2    */
#define ACD_XM_Y_OFFSET_MM       0.65  /* X- tile 2 center               */ 
#define ACD_XP_Y_OFFSET_MM      -0.65  /* X+ tile 2 center               */
#define ACD_YM_X_OFFSET_MM      -0.65  /* Y- tile 2 center               */
#define ACD_YP_X_OFFSET_MM       0.65  /* Y+ tile 2 center               */

                                       /* Central position of the planes */
#define ACD_XM_FACE_MM        -840.14  /* X- central X position          */
#define ACD_XP_FACE_MM         840.14  /* X+ central X position          */
#define ACD_YM_FACE_MM        -840.14  /* Y- central Y position          */
#define ACD_YP_FACE_MM         840.14  /* Y+ central Y position          */


/* --------------------------------------------------------------------- *\
 |                                                                       |
 |                These are all derived quantities                       | 
 |                                                                       |
\* --------------------------------------------------------------------- */


/* Effective width of TOP X tiles                                            */
#define ACD_X_TOP_0_EWIDTH_MM (ACD_X_TOP_0_WIDTH_MM - ACD_X_TOP_OVERLAP_MM/2.0)
#define ACD_X_TOP_1_EWIDTH_MM (ACD_X_TOP_1_WIDTH_MM - ACD_X_TOP_OVERLAP_MM    )
#define ACD_X_TOP_2_EWIDTH_MM (ACD_X_TOP_2_WIDTH_MM - ACD_X_TOP_OVERLAP_MM    )
#define ACD_X_TOP_3_EWIDTH_MM (ACD_X_TOP_3_WIDTH_MM - ACD_X_TOP_OVERLAP_MM    )
#define ACD_X_TOP_4_EWIDTH_MM (ACD_X_TOP_4_WIDTH_MM - ACD_X_TOP_OVERLAP_MM/2.0)


/* Edges of TOP X tiles in MM                                               */
#define ACD_X_TOP_0_BEG_MM  (ACD_X_TOP_1_BEG_MM   - ACD_X_TOP_0_EWIDTH_MM    )
#define ACD_X_TOP_1_BEG_MM  (ACD_X_TOP_2_BEG_MM   - ACD_X_TOP_1_EWIDTH_MM    )
#define ACD_X_TOP_2_BEG_MM  (ACD_X_TOP_OFFSET_MM  - ACD_X_TOP_2_EWIDTH_MM/2.0)
#define ACD_X_TOP_3_BEG_MM  (ACD_X_TOP_OFFSET_MM  + ACD_X_TOP_2_EWIDTH_MM/2.0)
#define ACD_X_TOP_4_BEG_MM  (ACD_X_TOP_3_BEG_MM   + ACD_X_TOP_3_EWIDTH_MM    )
#define ACD_X_TOP_4_END_MM  (ACD_X_TOP_4_BEG_MM   + ACD_X_TOP_4_EWIDTH_MM    )


/* Edges of TOP X tiles in Strip units                                      */
#define ACD_X_TOP_0_BEG      TFC_XY_SCALE(ACD_X_TOP_0_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_TOP_1_BEG      TFC_XY_SCALE(ACD_X_TOP_1_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_TOP_2_BEG      TFC_XY_SCALE(ACD_X_TOP_2_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_TOP_3_BEG      TFC_XY_SCALE(ACD_X_TOP_3_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_TOP_4_BEG      TFC_XY_SCALE(ACD_X_TOP_4_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_TOP_4_END      TFC_XY_SCALE(ACD_X_TOP_4_END_MM, TKR_K_STRIP_MM)



/* Effective width of the ACD Y top tiles in MM                          */
#define ACD_Y_TOP_0_EWIDTH_MM (ACD_Y_TOP_0_WIDTH_MM + ACD_Y_TOP_GAP_MM/2.0)
#define ACD_Y_TOP_1_EWIDTH_MM (ACD_Y_TOP_1_WIDTH_MM + ACD_Y_TOP_GAP_MM    )
#define ACD_Y_TOP_2_EWIDTH_MM (ACD_Y_TOP_2_WIDTH_MM + ACD_Y_TOP_GAP_MM    )
#define ACD_Y_TOP_3_EWIDTH_MM (ACD_Y_TOP_3_WIDTH_MM + ACD_Y_TOP_GAP_MM    )
#define ACD_Y_TOP_4_EWIDTH_MM (ACD_Y_TOP_4_WIDTH_MM + ACD_Y_TOP_GAP_MM/2.0)


/* Edges of the ACD Y top tiles in MM                                      */
#define ACD_Y_TOP_0_BEG_MM  (ACD_Y_TOP_1_BEG_MM  - ACD_Y_TOP_0_EWIDTH_MM    )
#define ACD_Y_TOP_1_BEG_MM  (ACD_Y_TOP_2_BEG_MM  - ACD_Y_TOP_1_EWIDTH_MM    )
#define ACD_Y_TOP_2_BEG_MM  (ACD_Y_TOP_OFFSET_MM - ACD_Y_TOP_2_EWIDTH_MM/2.0)
#define ACD_Y_TOP_3_BEG_MM  (ACD_Y_TOP_OFFSET_MM + ACD_Y_TOP_2_EWIDTH_MM/2.0)
#define ACD_Y_TOP_4_BEG_MM  (ACD_Y_TOP_3_BEG_MM  + ACD_Y_TOP_3_EWIDTH_MM    )
#define ACD_Y_TOP_4_END_MM  (ACD_Y_TOP_4_BEG_MM  + ACD_Y_TOP_4_EWIDTH_MM    )


/* Edges of the ACD Y top tiles in strip units                             */
#define ACD_Y_TOP_0_BEG      TFC_XY_SCALE(ACD_Y_TOP_0_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_TOP_1_BEG      TFC_XY_SCALE(ACD_Y_TOP_1_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_TOP_2_BEG      TFC_XY_SCALE(ACD_Y_TOP_2_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_TOP_3_BEG      TFC_XY_SCALE(ACD_Y_TOP_3_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_TOP_4_BEG      TFC_XY_SCALE(ACD_Y_TOP_4_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_TOP_4_END      TFC_XY_SCALE(ACD_Y_TOP_4_END_MM, TKR_K_STRIP_MM)

/* Top of the ACD top tiles (really the middle of tile 2                 */
#define ACD_TOP_Z            TFC_Z_ABS_SCALE(ACD_TOP_Z_MM)


/* ------------------------------------------------------------------------- *\
 |                       TOP ACD DERIVED QUANTITIES                          |
\* ------------------------------------------------------------------------- */

/* These are the effective widths of the X- tiles in MM                    */
#define ACD_XM_Y0_EWIDTH_MM (ACD_XM_Y0_WIDTH_MM + ACD_HORIZONTAL_GAP_MM/2.0)
#define ACD_XM_Y1_EWIDTH_MM (ACD_XM_Y1_WIDTH_MM - ACD_HORIZONTAL_GAP_MM    )
#define ACD_XM_Y2_EWIDTH_MM (ACD_XM_Y2_WIDTH_MM - ACD_HORIZONTAL_GAP_MM    )
#define ACD_XM_Y3_EWIDTH_MM (ACD_XM_Y3_WIDTH_MM - ACD_HORIZONTAL_GAP_MM    )
#define ACD_XM_Y4_EWIDTH_MM (ACD_XM_Y4_WIDTH_MM - ACD_HORIZONTAL_GAP_MM/2.0)


/* These are the effective widths of the X+ tiles in MM                    */
#define ACD_XP_Y0_EWIDTH_MM (ACD_XP_Y0_WIDTH_MM + ACD_HORIZONTAL_GAP_MM/2.0)
#define ACD_XP_Y1_EWIDTH_MM (ACD_XP_Y1_WIDTH_MM - ACD_HORIZONTAL_GAP_MM    )
#define ACD_XP_Y2_EWIDTH_MM (ACD_XP_Y2_WIDTH_MM - ACD_HORIZONTAL_GAP_MM    )
#define ACD_XP_Y3_EWIDTH_MM (ACD_XP_Y3_WIDTH_MM - ACD_HORIZONTAL_GAP_MM    )
#define ACD_XP_Y4_EWIDTH_MM (ACD_XP_Y4_WIDTH_MM - ACD_HORIZONTAL_GAP_MM/2.0)


/* These are the effective widths of the Y- tiles in MM                   */
#define ACD_YM_X0_EWIDTH_MM (ACD_YM_X0_WIDTH_MM - ACD_HORIZONTAL_GAP_MM/2.0)
#define ACD_YM_X1_EWIDTH_MM (ACD_YM_X0_WIDTH_MM - ACD_HORIZONTAL_GAP_MM    )
#define ACD_YM_X2_EWIDTH_MM (ACD_YM_X0_WIDTH_MM - ACD_HORIZONTAL_GAP_MM    )
#define ACD_YM_X3_EWIDTH_MM (ACD_YM_X0_WIDTH_MM - ACD_HORIZONTAL_GAP_MM    )
#define ACD_YM_X4_EWIDTH_MM (ACD_YM_X0_WIDTH_MM - ACD_HORIZONTAL_GAP_MM/2.0)


/* These are the effective widths of the Y+ tiles in MM                     */
#define ACD_YP_X0_EWIDTH_MM (ACD_YP_X0_WIDTH_MM - ACD_HORIZONTAL_GAP_MM/2.0)
#define ACD_YP_X1_EWIDTH_MM (ACD_YP_X0_WIDTH_MM - ACD_HORIZONTAL_GAP_MM    )
#define ACD_YP_X2_EWIDTH_MM (ACD_YP_X0_WIDTH_MM - ACD_HORIZONTAL_GAP_MM    )
#define ACD_YP_X3_EWIDTH_MM (ACD_YP_X0_WIDTH_MM - ACD_HORIZONTAL_GAP_MM    )
#define ACD_YP_X4_EWIDTH_MM (ACD_YP_X0_WIDTH_MM - ACD_HORIZONTAL_GAP_MM/2.0)



/* These are the edges of the X- tiles in MM                                */
#define ACD_XM_Y0_BEG_MM  (ACD_XM_Y1_BEG_MM   - ACD_XM_Y0_EWIDTH_MM    )
#define ACD_XM_Y1_BEG_MM  (ACD_XM_Y2_BEG_MM   - ACD_XM_Y1_EWIDTH_MM    )
#define ACD_XM_Y2_BEG_MM  (ACD_XM_Y_OFFSET_MM - ACD_XM_Y2_EWIDTH_MM/2.0)
#define ACD_XM_Y3_BEG_MM  (ACD_XM_Y_OFFSET_MM + ACD_XM_Y2_EWIDTH_MM/2.0)
#define ACD_XM_Y4_BEG_MM  (ACD_XM_Y3_BEG_MM   + ACD_XM_Y3_EWIDTH_MM    )
#define ACD_XM_Y4_END_MM  (ACD_XM_Y4_BEG_MM   + ACD_XM_Y4_EWIDTH_MM    )



/* These are the edges of the X+ tiles in MM                                */
#define ACD_XP_Y0_BEG_MM  (ACD_XP_Y1_BEG_MM   - ACD_XP_Y0_EWIDTH_MM    )
#define ACD_XP_Y1_BEG_MM  (ACD_XP_Y2_BEG_MM   - ACD_XP_Y1_EWIDTH_MM    )
#define ACD_XP_Y2_BEG_MM  (ACD_XP_Y_OFFSET_MM - ACD_XP_Y2_EWIDTH_MM/2.0)
#define ACD_XP_Y3_BEG_MM  (ACD_XP_Y_OFFSET_MM + ACD_XP_Y2_EWIDTH_MM/2.0)
#define ACD_XP_Y4_BEG_MM  (ACD_XP_Y3_BEG_MM   + ACD_XP_Y3_EWIDTH_MM    )
#define ACD_XP_Y4_END_MM  (ACD_XP_Y4_BEG_MM   + ACD_XP_Y4_EWIDTH_MM    )


/* These are edges of the Y- tiles in MM                          */
#define ACD_YM_X0_BEG_MM  (ACD_YM_X1_BEG_MM   - ACD_YM_X0_EWIDTH_MM)
#define ACD_YM_X1_BEG_MM  (ACD_YM_X2_BEG_MM   - ACD_YM_X1_EWIDTH_MM)
#define ACD_YM_X2_BEG_MM  (ACD_YM_X_OFFSET_MM - ACD_YM_X2_EWIDTH_MM/2.0)
#define ACD_YM_X3_BEG_MM  (ACD_YM_X_OFFSET_MM + ACD_YM_X2_EWIDTH_MM/2.0)
#define ACD_YM_X4_BEG_MM  (ACD_YM_X3_BEG_MM   + ACD_YM_X3_EWIDTH_MM)
#define ACD_YM_X4_END_MM  (ACD_YM_X4_BEG_MM   + ACD_YM_X4_EWIDTH_MM)


/* These are edges of the Y+ tiles in MM                          */
#define ACD_YP_X0_BEG_MM  (ACD_YP_X1_BEG_MM   - ACD_YP_X0_EWIDTH_MM)
#define ACD_YP_X1_BEG_MM  (ACD_YP_X2_BEG_MM   - ACD_YP_X1_EWIDTH_MM)
#define ACD_YP_X2_BEG_MM  (ACD_YP_X_OFFSET_MM - ACD_YP_X2_EWIDTH_MM/2.0)
#define ACD_YP_X3_BEG_MM  (ACD_YP_X_OFFSET_MM + ACD_YP_X2_EWIDTH_MM/2.0)
#define ACD_YP_X4_BEG_MM  (ACD_YP_X3_BEG_MM   + ACD_YP_X3_EWIDTH_MM)
#define ACD_YP_X4_END_MM  (ACD_YP_X4_BEG_MM   + ACD_YP_X4_EWIDTH_MM)




/* These are the edges of the X- tiles in strip units                      */
#define ACD_XM_Y0_BEG      TFC_XY_SCALE(ACD_XM_Y0_BEG_MM, TKR_K_STRIP_MM)
#define ACD_XM_Y1_BEG      TFC_XY_SCALE(ACD_XM_Y1_BEG_MM, TKR_K_STRIP_MM)
#define ACD_XM_Y2_BEG      TFC_XY_SCALE(ACD_XM_Y2_BEG_MM, TKR_K_STRIP_MM)
#define ACD_XM_Y3_BEG      TFC_XY_SCALE(ACD_XM_Y3_BEG_MM, TKR_K_STRIP_MM)
#define ACD_XM_Y4_BEG      TFC_XY_SCALE(ACD_XM_Y4_BEG_MM, TKR_K_STRIP_MM)
#define ACD_XM_Y4_END      TFC_XY_SCALE(ACD_XM_Y4_END_MM, TKR_K_STRIP_MM)


/* These are the edges of the X+ tiles in strip units                      */
#define ACD_XP_Y0_BEG      TFC_XY_SCALE(ACD_XP_Y0_BEG_MM, TKR_K_STRIP_MM)
#define ACD_XP_Y1_BEG      TFC_XY_SCALE(ACD_XP_Y1_BEG_MM, TKR_K_STRIP_MM)
#define ACD_XP_Y2_BEG      TFC_XY_SCALE(ACD_XP_Y2_BEG_MM, TKR_K_STRIP_MM)
#define ACD_XP_Y3_BEG      TFC_XY_SCALE(ACD_XP_Y3_BEG_MM, TKR_K_STRIP_MM)
#define ACD_XP_Y4_BEG      TFC_XY_SCALE(ACD_XP_Y4_BEG_MM, TKR_K_STRIP_MM)
#define ACD_XP_Y4_END      TFC_XY_SCALE(ACD_XP_Y4_END_MM, TKR_K_STRIP_MM)


/* These are the edges of the Y- tiles in strip units               */
#define ACD_YM_X0_BEG      TFC_XY_SCALE(ACD_YM_X0_BEG_MM, TKR_K_STRIP_MM)
#define ACD_YM_X1_BEG      TFC_XY_SCALE(ACD_YM_X1_BEG_MM, TKR_K_STRIP_MM)
#define ACD_YM_X2_BEG      TFC_XY_SCALE(ACD_YM_X2_BEG_MM, TKR_K_STRIP_MM)
#define ACD_YM_X3_BEG      TFC_XY_SCALE(ACD_YM_X3_BEG_MM, TKR_K_STRIP_MM)
#define ACD_YM_X4_BEG      TFC_XY_SCALE(ACD_YM_X4_BEG_MM, TKR_K_STRIP_MM)
#define ACD_YM_X4_END      TFC_XY_SCALE(ACD_YM_X4_END_MM, TKR_K_STRIP_MM)


/* These are the edges of the Y+ tiles in strip units               */
#define ACD_YP_X0_BEG      TFC_XY_SCALE(ACD_YP_X0_BEG_MM, TKR_K_STRIP_MM)
#define ACD_YP_X1_BEG      TFC_XY_SCALE(ACD_YP_X1_BEG_MM, TKR_K_STRIP_MM)
#define ACD_YP_X2_BEG      TFC_XY_SCALE(ACD_YP_X2_BEG_MM, TKR_K_STRIP_MM)
#define ACD_YP_X3_BEG      TFC_XY_SCALE(ACD_YP_X3_BEG_MM, TKR_K_STRIP_MM)
#define ACD_YP_X4_BEG      TFC_XY_SCALE(ACD_YP_X4_BEG_MM, TKR_K_STRIP_MM)
#define ACD_YP_X4_END      TFC_XY_SCALE(ACD_YP_X4_END_MM, TKR_K_STRIP_MM)


/* Defines distance between ACD and TKR in MM            */
#define ACD_XM_FROM_TKR_MM (ACD_XM_FACE_MM - TKR_X0_BEG_MM)
#define ACD_XP_FROM_TKR_MM (ACD_XP_FACE_MM - TKR_X3_BEG_MM)
#define ACD_YM_FROM_TKR_MM (ACD_YM_FACE_MM - TKR_Y0_BEG_MM)
#define ACD_YP_FROM_TKR_MM (ACD_YP_FACE_MM - TKR_Y3_BEG_MM)


/* Defines distance between ACD and TKR in strip units   */
#define ACD_XM_FACE       TFC_XY_SCALE(ACD_XM_FACE_MM, TKR_K_STRIP_MM)
#define ACD_XP_FACE       TFC_XY_SCALE(ACD_XP_FACE_MM, TKR_K_STRIP_MM)
#define ACD_YM_FACE       TFC_XY_SCALE(ACD_YM_FACE_MM, TKR_K_STRIP_MM)
#define ACD_YP_FACE       TFC_XY_SCALE(ACD_YP_FACE_MM, TKR_K_STRIP_MM)

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



#define ACD_SIDE_Z0_EWIDTH_MM (ACD_SIDE_Z0_WIDTH_MM-ACD_SIDE_Z_OVERLAP_MM/2.0)
#define ACD_SIDE_Z1_EWIDTH_MM (ACD_SIDE_Z1_WIDTH_MM-ACD_SIDE_Z_OVERLAP_MM    )
#define ACD_SIDE_Z2_EWIDTH_MM (ACD_SIDE_Z2_WIDTH_MM-ACD_SIDE_Z_OVERLAP_MM    )
#define ACD_SIDE_Z3_EWIDTH_MM (ACD_SIDE_Z3_WIDTH_MM-ACD_SIDE_Z_OVERLAP_MM/2.0)


#define ACD_SIDE_Z1_TOP_MM (ACD_SIDE_Z0_TOP_MM - ACD_SIDE_Z0_EWIDTH_MM)
#define ACD_SIDE_Z2_TOP_MM (ACD_SIDE_Z1_TOP_MM - ACD_SIDE_Z1_EWIDTH_MM)
#define ACD_SIDE_Z3_TOP_MM (ACD_SIDE_Z2_TOP_MM - ACD_SIDE_Z2_EWIDTH_MM)
#define ACD_SIDE_Z3_BOT_MM (ACD_SIDE_Z3_TOP_MM - ACD_SIDE_Z3_EWIDTH_MM)


#define ACD_SIDE_Z0_TOP TFC_Z_ABS_SCALE(ACD_SIDE_Z0_TOP_MM) 
#define ACD_SIDE_Z1_TOP TFC_Z_ABS_SCALE(ACD_SIDE_Z1_TOP_MM) 
#define ACD_SIDE_Z2_TOP TFC_Z_ABS_SCALE(ACD_SIDE_Z2_TOP_MM)
#define ACD_SIDE_Z3_TOP TFC_Z_ABS_SCALE(ACD_SIDE_Z3_TOP_MM)
#define ACD_SIDE_Z3_BOT TFC_Z_ABS_SCALE(ACD_SIDE_Z3_BOT_MM)
/* --------------------------------------------------------------------- */ 





/* --------------------------------------------------------------------  *\
 |                                                                       |  
 | PROJECTIONS FROM TKR to TOP ACD TILE FACE                             |
 | =========================================                             |   
 |                                                                       |  
 | This defines how to project from any TKR layer to the ACD TOP face.   |
 |                                                                       |
\* --------------------------------------------------------------------- */
#define ACD_TOP_XPROJECTION(_layer, _layer_m1)                           \
    TFC_ACD_TOP_PROJECTION(ACD_TOP_Z_MM,                                 \
                           TKR_Z_XLAYER_MM(_layer),                      \
                           TKR_Z_XLAYER_MM(_layer_m1))


#define ACD_TOP_YPROJECTION(_layer, _layer_m1)                           \
    TFC_ACD_TOP_PROJECTION(ACD_TOP_Z_MM,                                 \
                           TKR_Z_YLAYER_MM(_layer),                      \
                           TKR_Z_YLAYER_MM(_layer_m1))

/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------  *\
 |                                                                       |  
 | PROJECTIONS FROM TKR TO SKIRT REGION                                  |
 | ====================================                                  |
 |                                                                       |
 | This defines the XY dimensions and Z position of the skirt region.    |
 | The skirt region is that region between the ACD and CAL which is      |
 | unprotected by the ACD. Charged tracks can sneak up this area,        |
 | missing the ACD and thus potentially faking photons in the LAT. The   |
 | only recourse is to project tracks to this plane and see if they fall |
 | into this uprotected region. If so, they are rejected. This causes    |
 | a loss in efficiency, but there is no other alternative.              |
 |                                                                       |
\* --------------------------------------------------------------------  */

#define CAL_Z_OFFSET_MM  -149.446
#define CAL_X_EDGE_BEG_MM  -728.5
#define CAL_X_EDGE_END_MM   728.5
#define CAL_Y_EDGE_BEG_MM  -728.5
#define CAL_Y_EDGE_END_MM   728.5

#define CAL_X_EDGE_BEG      TFC_XY_SCALE(CAL_X_EDGE_BEG_MM, TKR_K_STRIP_MM)
#define CAL_X_EDGE_END      TFC_XY_SCALE(CAL_X_EDGE_END_MM, TKR_K_STRIP_MM)
#define CAL_Y_EDGE_BEG      TFC_XY_SCALE(CAL_Y_EDGE_BEG_MM, TKR_K_STRIP_MM)
#define CAL_Y_EDGE_END      TFC_XY_SCALE(CAL_Y_EDGE_END_MM, TKR_K_STRIP_MM)

/* Arbritarily define the Z plane to at the CAL boundary */
#define SKIRT_Z_MM        ACD_SIDE_Z3_BOT_MM
#define SKIRT_Z           TFC_Z_ABS_SCALE(SKIRT_Z_MM)

/* The XY limits of the region will be the limits of the ACD/CAL */
#define SKIRT_X_LEFT_BEG  ACD_XM_FACE
#define SKIRT_X_LEFT_END  CAL_X_EDGE_BEG
#define SKIRT_X_RIGHT_BEG CAL_X_EDGE_END
#define SKIRT_X_RIGHT_END ACD_XP_FACE


#define SKIRT_Y_LEFT_BEG  ACD_YM_FACE
#define SKIRT_Y_LEFT_END  CAL_Y_EDGE_BEG
#define SKIRT_Y_RIGHT_BEG CAL_Y_EDGE_END
#define SKIRT_Y_RIGHT_END ACD_YP_FACE


/* --------------------------------------------------------------------  *\
 |                                                                       |  
 | PROJECTIONS FROM TKR to SKIRT PLANE                                   |
 | ===================================                                   |   
 |                                                                       |  
 | This defines how to project from any TKR layer to the ACD TOP face.   |
 |                                                                       |
\* --------------------------------------------------------------------- */
#define SKIRT_XPROJECTION(_layer, _layer_p1)                             \
    TFC_SKIRT_PROJECTION(SKIRT_Z_MM,                                     \
                         TKR_Z_XLAYER_MM(_layer),                        \
                         TKR_Z_XLAYER_MM(_layer_p1))


#define SKIRT_YPROJECTION(_layer, _layer_p1)                             \
    TFC_SKIRT_PROJECTION(SKIRT_Z_MM,                                     \
                         TKR_Z_YLAYER_MM(_layer),                        \
                         TKR_Z_YLAYER_MM(_layer_p1))

/* --------------------------------------------------------------------- */


   


#define TKR_XY_WIDTHS                                                    \
     {                                                                   \
        TKR_XY_WIDTH,                                                    \
        TKR_XY_WIDTH                                                     \
     }


#define TKR_X_BEGS                                                       \
     {                    /* X Offset to beginning of column n */        \
       TKR_X0_BEG,        /* X Offset to beginning of column 0 */        \
       TKR_X1_BEG,        /* X Offset to beginning of column 1 */        \
       TKR_X2_BEG,        /* X Offset to beginning of column 2 */        \
       TKR_X3_BEG         /* X Offset to beginning of column 3 */        \
     }

#define TKR_Z_XLAYERS                                                    \
     {                    /* Z position of layer  Xn           */        \
       TKR_Z_XLAYER_00,   /* Z position of layer  X0           */        \
       TKR_Z_XLAYER_01,   /* Z position of layer  X1           */        \
       TKR_Z_XLAYER_02,   /* Z position of layer  X2           */        \
       TKR_Z_XLAYER_03,   /* Z position of layer  X3           */        \
       TKR_Z_XLAYER_04,   /* Z position of layer  X4           */        \
       TKR_Z_XLAYER_05,   /* Z position of layer  X5           */        \
       TKR_Z_XLAYER_06,   /* Z position of layer  X6           */        \
       TKR_Z_XLAYER_07,   /* Z position of layer  X7           */        \
       TKR_Z_XLAYER_08,   /* Z position of layer  X8           */        \
       TKR_Z_XLAYER_09,   /* Z position of layer  X9           */        \
       TKR_Z_XLAYER_10,   /* Z position of layer X10           */        \
       TKR_Z_XLAYER_11,   /* Z position of layer X11           */        \
       TKR_Z_XLAYER_12,   /* Z position of layer X12           */        \
       TKR_Z_XLAYER_13,   /* Z position of layer X13           */        \
       TKR_Z_XLAYER_14,   /* Z position of layer X14           */        \
       TKR_Z_XLAYER_15,   /* Z position of layer X15           */        \
       TKR_Z_XLAYER_16,   /* Z position of layer X16           */        \
       TKR_Z_XLAYER_17,   /* Z position of layer X17           */        \
     }


#define TKR_Y_BEGS                                                       \
     {                    /* Y Offset to beginning of row n    */        \
        TKR_Y0_BEG,       /* Y Offset to beginning of row 0    */        \
        TKR_Y1_BEG,       /* Y Offset to beginning of row 1    */        \
        TKR_Y2_BEG,       /* Y Offset to beginning of row 2    */        \
        TKR_Y3_BEG        /* Y Offset to beginning of row 3    */        \
     }


#define TKR_Z_YLAYERS                                                    \
     {                    /* Z position of layer  Yn           */        \
       TKR_Z_YLAYER_00,   /* Z position of layer  Y0           */        \
       TKR_Z_YLAYER_01,   /* Z position of layer  Y1           */        \
       TKR_Z_YLAYER_02,   /* Z position of layer  Y2           */        \
       TKR_Z_YLAYER_03,   /* Z position of layer  Y3           */        \
       TKR_Z_YLAYER_04,   /* Z position of layer  Y4           */        \
       TKR_Z_YLAYER_05,   /* Z position of layer  Y5           */        \
       TKR_Z_YLAYER_06,   /* Z position of layer  Y6           */        \
       TKR_Z_YLAYER_07,   /* Z position of layer  Y7           */        \
       TKR_Z_YLAYER_08,   /* Z position of layer  Y8           */        \
       TKR_Z_YLAYER_09,   /* Z position of layer  Y9           */        \
       TKR_Z_YLAYER_10,   /* Z position of layer Y10           */        \
       TKR_Z_YLAYER_11,   /* Z position of layer Y11           */        \
       TKR_Z_YLAYER_12,   /* Z position of layer Y12           */        \
       TKR_Z_YLAYER_13,   /* Z position of layer Y13           */        \
       TKR_Z_YLAYER_14,   /* Z position of layer Y14           */        \
       TKR_Z_YLAYER_15,   /* Z position of layer Y15           */        \
       TKR_Z_YLAYER_16,   /* Z position of layer Y16           */        \
       TKR_Z_YLAYER_17,   /* Z position of layer Y17           */        \
     }


#define TKR_GEOMETRY                                                     \
   {                                                                     \
        TKR_K_STRIP,                                                     \
        (TKR_Z_FIND_MAX  << 16) | TKR_Z_FIND_MIN,                        \
        (TKR_Z_EXTEND_MAX<< 16) | TKR_Z_EXTEND_MIN,                      \
         TKR_XY_WIDTHS,                                                  \
      {  /* TKR.TWR Geometery          */                                \
         {                                                               \
            { TKR_X_BEGS, TKR_Z_XLAYERS },                               \
            { TKR_Y_BEGS, TKR_Z_YLAYERS }                                \
         }                                                               \
      }                                                                  \
   }


#define ACD_X_TOP_EDGES                                                  \
    {                                                                    \
        ACD_X_TOP_0_BEG,                                                 \
        ACD_X_TOP_1_BEG,                                                 \
        ACD_X_TOP_2_BEG,                                                 \
        ACD_X_TOP_3_BEG,                                                 \
        ACD_X_TOP_4_BEG,                                                 \
        ACD_X_TOP_4_END,                                                 \
    }

#define ACD_Y_TOP_EDGES                                                  \
    {                                                                    \
        ACD_Y_TOP_0_BEG,                                                 \
        ACD_Y_TOP_1_BEG,                                                 \
        ACD_Y_TOP_2_BEG,                                                 \
        ACD_Y_TOP_3_BEG,                                                 \
        ACD_Y_TOP_4_BEG,                                                 \
        ACD_Y_TOP_4_END,                                                 \
    }


#define ACD_XM_SIDE_YEDGES                                               \
    {                                                                    \
       ACD_XM_Y0_BEG,                                                    \
       ACD_XM_Y1_BEG,                                                    \
       ACD_XM_Y2_BEG,                                                    \
       ACD_XM_Y3_BEG,                                                    \
       ACD_XM_Y4_BEG,                                                    \
       ACD_XM_Y4_END                                                     \
    }

#define ACD_XP_SIDE_YEDGES                                               \
    {                                                                    \
       ACD_XP_Y0_BEG,                                                    \
       ACD_XP_Y1_BEG,                                                    \
       ACD_XP_Y2_BEG,                                                    \
       ACD_XP_Y3_BEG,                                                    \
       ACD_XP_Y4_BEG,                                                    \
       ACD_XP_Y4_END                                                     \
    }


#define ACD_YM_SIDE_XEDGES                                               \
    {                                                                    \
       ACD_YM_X0_BEG,                                                    \
       ACD_YM_X1_BEG,                                                    \
       ACD_YM_X2_BEG,                                                    \
       ACD_YM_X3_BEG,                                                    \
       ACD_YM_X4_BEG,                                                    \
       ACD_YM_X4_END                                                     \
    }


#define ACD_YP_SIDE_XEDGES                                               \
    {                                                                    \
       ACD_YP_X0_BEG,                                                    \
       ACD_YP_X1_BEG,                                                    \
       ACD_YP_X2_BEG,                                                    \
       ACD_YP_X3_BEG,                                                    \
       ACD_YP_X4_BEG,                                                    \
       ACD_YP_X4_END                                                     \
    }


#define ACD_Z_SIDE_ROWS                                                  \
     {                                                                   \
        ACD_SIDE_Z0_TOP,                                                 \
        ACD_SIDE_Z1_TOP,                                                 \
        ACD_SIDE_Z2_TOP,                                                 \
        ACD_SIDE_Z3_TOP,                                                 \
        ACD_SIDE_Z3_BOT,                                                 \
        0                       /* Unused */                             \
     }


#define ACD_SIDE_FACES                                                   \
     {                                                                   \
        ACD_XM_FACE,                                                     \
        ACD_XP_FACE,                                                     \
        ACD_YM_FACE,                                                     \
        ACD_YP_FACE,                                                     \
     }


/*
 |  Gives the scale factor to project to the ACD. The index is the top layer
 |  of the tracker to project from. That is
 |
 |  X_at_acd =  X_top_layer
 |           + (X_top_layer - X_top_layer-1) * AcdXProjection[top_layer]
 |           / ACD_TOP_TKR_ZSCALE;
*/
#define ACD_TOP_XPROJECTIONS                                             \
    {                                                                    \
       0,   /* -- Unused */                                              \
       0,   /* -- Unused */                                              \
       ACD_TOP_XPROJECTION(02, 01), /* Extend from  2 to ACD */          \
       ACD_TOP_XPROJECTION(03, 02), /* Extend from  3 to ACD */          \
       ACD_TOP_XPROJECTION(04, 03), /* Extend from  4 to ACD */          \
       ACD_TOP_XPROJECTION(05, 04), /* Extend from  5 to ACD */          \
       ACD_TOP_XPROJECTION(06, 05), /* Extend from  6 to ACD */          \
       ACD_TOP_XPROJECTION(07, 06), /* Extend from  7 to ACD */          \
       ACD_TOP_XPROJECTION(08, 07), /* Extend from  8 to ACD */          \
       ACD_TOP_XPROJECTION(09, 08), /* Extend from  9 to ACD */          \
       ACD_TOP_XPROJECTION(10, 09), /* Extend from 10 to ACD */          \
       ACD_TOP_XPROJECTION(11, 10), /* Extend from 11 to ACD */          \
       ACD_TOP_XPROJECTION(12, 11), /* Extend from 12 to ACD */          \
       ACD_TOP_XPROJECTION(13, 12), /* Extend from 13 to ACD */          \
       ACD_TOP_XPROJECTION(14, 13), /* Extend from 14 to ACD */          \
       ACD_TOP_XPROJECTION(15, 14), /* Extend from 15 to ACD */          \
       ACD_TOP_XPROJECTION(16, 15), /* Extend from 16 to ACD */          \
       ACD_TOP_XPROJECTION(17, 16), /* Extend from 17 to ACD */          \
    }



#define ACD_TOP_YPROJECTIONS                                            \
   {                             /* ACD TOP PROJECTION    */            \
       0,   /* -- Unused */                                             \
       0,   /* -- Unused */                                             \
       ACD_TOP_YPROJECTION(02, 01), /* Extend from  2 to ACD */         \
       ACD_TOP_YPROJECTION(03, 02), /* Extend from  3 to ACD */         \
       ACD_TOP_YPROJECTION(04, 03), /* Extend from  4 to ACD */         \
       ACD_TOP_YPROJECTION(05, 04), /* Extend from  5 to ACD */         \
       ACD_TOP_YPROJECTION(06, 05), /* Extend from  6 to ACD */         \
       ACD_TOP_YPROJECTION(07, 06), /* Extend from  7 to ACD */         \
       ACD_TOP_YPROJECTION(08, 07), /* Extend from  8 to ACD */         \
       ACD_TOP_YPROJECTION(09, 08), /* Extend from  9 to ACD */         \
       ACD_TOP_YPROJECTION(10, 09), /* Extend from 10 to ACD */         \
       ACD_TOP_YPROJECTION(11, 10), /* Extend from 11 to ACD */         \
       ACD_TOP_YPROJECTION(12, 11), /* Extend from 12 to ACD */         \
       ACD_TOP_YPROJECTION(13, 12), /* Extend from 13 to ACD */         \
       ACD_TOP_YPROJECTION(14, 13), /* Extend from 14 to ACD */         \
       ACD_TOP_YPROJECTION(15, 14), /* Extend from 15 to ACD */         \
       ACD_TOP_YPROJECTION(16, 15), /* Extend from 16 to ACD */         \
       ACD_TOP_YPROJECTION(17, 16), /* Extend from 17 to ACD */         \
    }
 
#define ACD_GEOMETRY                                                    \
   {                                                                    \
      TKR_Z_NOMINALx2,                                                  \
      ACD_X_TOP_EDGES,                                                  \
      ACD_Y_TOP_EDGES,                                                  \
      ACD_XM_SIDE_YEDGES,                                               \
      ACD_XP_SIDE_YEDGES,                                               \
      ACD_YM_SIDE_XEDGES,                                               \
      ACD_YP_SIDE_XEDGES,                                               \
      ACD_Z_SIDE_ROWS,                                                  \
      ACD_SIDE_FACES,                                                   \
      ACD_TOP_XPROJECTIONS,                                             \
      ACD_TOP_YPROJECTIONS                                              \
   }



#define SKIRT_XPROJECTIONS                                              \
    {                                                                   \
       SKIRT_XPROJECTION(00, 01), /* Extend from  0 to SKIRT */         \
       SKIRT_XPROJECTION(01, 02), /* Extend from  1 to SKIRT */         \
       SKIRT_XPROJECTION(02, 03), /* Extend from  2 to SKIRT */         \
       SKIRT_XPROJECTION(03, 04), /* Extend from  3 to SKIRT */         \
       SKIRT_XPROJECTION(04, 05), /* Extend from  4 to SKIRT */         \
       SKIRT_XPROJECTION(05, 06), /* Extend from  5 to SKIRT */         \
       SKIRT_XPROJECTION(06, 07), /* Extend from  6 to SKIRT */         \
       SKIRT_XPROJECTION(07, 08), /* Extend from  7 to SKIRT */         \
       SKIRT_XPROJECTION(08, 09), /* Extend from  8 to SKIRT */         \
       SKIRT_XPROJECTION(09, 10), /* Extend from  9 to SKIRT */         \
       SKIRT_XPROJECTION(10, 11), /* Extend from 10 to SKIRT */         \
       SKIRT_XPROJECTION(11, 12), /* Extend from 11 to SKIRT */         \
       SKIRT_XPROJECTION(12, 13), /* Extend from 12 to SKIRT */         \
       SKIRT_XPROJECTION(13, 14), /* Extend from 13 to SKIRT */         \
       SKIRT_XPROJECTION(14, 15), /* Extend from 14 to SKIRT */         \
       SKIRT_XPROJECTION(15, 16), /* Extend from 15 to SKIRT */         \
       0,                         /* -- Unused --            */         \
       0,                         /* -- Unused --            */         \
    }

#define SKIRT_YPROJECTIONS                                              \
    {                                                                   \
       SKIRT_YPROJECTION(00, 01), /* Extend from  0 to SKIRT */         \
       SKIRT_YPROJECTION(01, 02), /* Extend from  1 to SKIRT */         \
       SKIRT_YPROJECTION(02, 03), /* Extend from  2 to SKIRT */         \
       SKIRT_YPROJECTION(03, 04), /* Extend from  3 to SKIRT */         \
       SKIRT_YPROJECTION(04, 05), /* Extend from  4 to SKIRT */         \
       SKIRT_YPROJECTION(05, 06), /* Extend from  5 to SKIRT */         \
       SKIRT_YPROJECTION(06, 07), /* Extend from  6 to SKIRT */         \
       SKIRT_YPROJECTION(07, 08), /* Extend from  7 to SKIRT */         \
       SKIRT_YPROJECTION(08, 09), /* Extend from  8 to SKIRT */         \
       SKIRT_YPROJECTION(09, 10), /* Extend from  9 to SKIRT */         \
       SKIRT_YPROJECTION(10, 11), /* Extend from 10 to SKIRT */         \
       SKIRT_YPROJECTION(11, 12), /* Extend from 11 to SKIRT */         \
       SKIRT_YPROJECTION(12, 13), /* Extend from 12 to SKIRT */         \
       SKIRT_YPROJECTION(13, 14), /* Extend from 13 to SKIRT */         \
       SKIRT_YPROJECTION(14, 15), /* Extend from 14 to SKIRT */         \
       SKIRT_YPROJECTION(15, 16), /* Extend from 15 to SKIRT */         \
       0,                         /* -- Unused --          */           \
       0,                         /* -- Unused --          */           \
    }


#define SKIRT_GEOMETRY                                                        \
{                                                                             \
  SKIRT_Z,                                                                    \
  {                                                                           \
   {SKIRT_X_LEFT_BEG, SKIRT_X_LEFT_END, SKIRT_X_RIGHT_BEG, SKIRT_X_RIGHT_END},\
   {SKIRT_Y_LEFT_BEG, SKIRT_Y_LEFT_END, SKIRT_Y_RIGHT_BEG, SKIRT_Y_RIGHT_END},\
  },                                                                          \
  SKIRT_XPROJECTIONS,                                                         \
  SKIRT_YPROJECTIONS                                                          \
}

#endif

