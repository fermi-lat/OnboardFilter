/* ---------------------------------------------------------------------- *//*!
   
  \file   TFC_geometryStd.c
  \brief  Defines the LAT geometry from the tracker's perspective
  \author JJRussell - russell@slac.stanford.edu

\verbatim
   CVS $Id: TFC_geometryStd.c,v 1.2 2004/07/12 17:37:44 golpa Exp $
\endverbatim 
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/TFC_geoIds.h"
#include "TFC_geometryDef.h"


#ifndef CMX_DOXYGEN


#define TFC_STD_CMT    TFC_TAGCMT(TFC_K_TAGTYPE_GLEAM, 0, 0, 10)
#define TFC_STD_CREATE TFC_TAGDATE  (1, 11, 2002)
#define TFC_STD_REVISE TFC_TAGDATE  (2, 12, 2003)
#define TFC_STD_TAG    TFC_TAG      ( TFC_K_GEO_ID_INITIAL, \
                                      TFC_STD_CMT,          \
                                      TFC_STD_CREATE,       \
                                      TFC_STD_REVISE)



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
#define TKR_K_STRIP_MM     0.228             /* Strip width              */

#define TKR_X_PITCH_MM   374.5               /* Center-to-center dist.   */
#define TKR_X_ACTIVE_MM (358.6-2*.974)       /* Active-to-active dist.   */
#define TKR_X_CENTER_MM    0.0               /* Middle of the detector   */
#define TKR_X_GAP_MM    (TKR_X_PITCH_MM - TKR_X_ACTIVE_MM)


#define TKR_X0_BEG_MM (TKR_X1_BEG_MM    - TKR_X_PITCH_MM)
#define TKR_X1_BEG_MM (TKR_X2_BEG_MM    - TKR_X_PITCH_MM)
#define TKR_X2_BEG_MM (TKR_X_GAP_MM/2.0 + TKR_X_CENTER_MM)
#define TKR_X3_BEG_MM (TKR_X2_BEG_MM    + TKR_X_PITCH_MM)

#define TKR_X0_END_MM (TKR_X0_BEG_MM    + TKR_X_ACTIVE_MM)
#define TKR_X1_END_MM (TKR_X1_BEG_MM    + TKR_X_ACTIVE_MM)
#define TKR_X2_END_MM (TKR_X2_BEG_MM    + TKR_X_ACTIVE_MM)
#define TKR_X3_END_MM (TKR_X3_BEG_MM    + TKR_X_ACTIVE_MM)


#define TKR_K_STRIP   (TKR_K_STRIP_MM * 1000 + 0.5)
#define TKR_X_ACTIVE   TFC_XY_SCALE(TKR_X_ACTIVE_MM,  TKR_K_STRIP_MM) 

#define TKR_X0_BEG     TFC_XY_SCALE(TKR_X0_BEG_MM,    TKR_K_STRIP_MM)
#define TKR_X1_BEG     TFC_XY_SCALE(TKR_X1_BEG_MM,    TKR_K_STRIP_MM)
#define TKR_X2_BEG     TFC_XY_SCALE(TKR_X2_BEG_MM,    TKR_K_STRIP_MM)
#define TKR_X3_BEG     TFC_XY_SCALE(TKR_X3_BEG_MM,    TKR_K_STRIP_MM)

#define TKR_X0_END     TFC_XY_SCALE(TKR_X0_END_MM,    TKR_K_STRIP_MM)
#define TKR_X1_END     TFC_XY_SCALE(TKR_X1_END_MM,    TKR_K_STRIP_MM)
#define TKR_X2_END     TFC_XY_SCALE(TKR_X2_END_MM,    TKR_K_STRIP_MM)
#define TKR_X3_END     TFC_XY_SCALE(TKR_X3_END_MM,    TKR_K_STRIP_MM)


#define TKR_Y_PITCH_MM   374.5               /* Center-to-center dist.   */
#define TKR_Y_ACTIVE_MM (358.6-2*.974)       /* Active-to-active dist.   */
#define TKR_Y_CENTER_MM    0.0               /* Middle of the detector   */
#define TKR_Y_GAP_MM    (TKR_X_PITCH_MM - TKR_X_ACTIVE_MM)

#define TKR_Y0_BEG_MM (TKR_Y1_BEG_MM     - TKR_Y_PITCH_MM)
#define TKR_Y1_BEG_MM (TKR_Y2_BEG_MM     - TKR_Y_PITCH_MM)
#define TKR_Y2_BEG_MM (TKR_Y_GAP_MM/2.0  + TKR_Y_CENTER_MM)
#define TKR_Y3_BEG_MM (TKR_Y2_BEG_MM     + TKR_Y_PITCH_MM)

#define TKR_Y0_END_MM (TKR_Y0_BEG_MM     + TKR_Y_ACTIVE_MM)
#define TKR_Y1_END_MM (TKR_Y1_BEG_MM     + TKR_Y_ACTIVE_MM)
#define TKR_Y2_END_MM (TKR_Y2_BEG_MM     + TKR_Y_ACTIVE_MM)
#define TKR_Y3_END_MM (TKR_Y3_BEG_MM     + TKR_Y_ACTIVE_MM)

#define TKR_Y_ACTIVE   TFC_XY_SCALE(TKR_Y_ACTIVE_MM,  TKR_K_STRIP_MM)

#define TKR_Y0_BEG     TFC_XY_SCALE(TKR_Y0_BEG_MM,    TKR_K_STRIP_MM)
#define TKR_Y1_BEG     TFC_XY_SCALE(TKR_Y1_BEG_MM,    TKR_K_STRIP_MM)
#define TKR_Y2_BEG     TFC_XY_SCALE(TKR_Y2_BEG_MM,    TKR_K_STRIP_MM)
#define TKR_Y3_BEG     TFC_XY_SCALE(TKR_Y3_BEG_MM,    TKR_K_STRIP_MM)

#define TKR_Y0_END     TFC_XY_SCALE(TKR_Y0_END_MM,    TKR_K_STRIP_MM)
#define TKR_Y1_END     TFC_XY_SCALE(TKR_Y1_END_MM,    TKR_K_STRIP_MM)
#define TKR_Y2_END     TFC_XY_SCALE(TKR_Y2_END_MM,    TKR_K_STRIP_MM)
#define TKR_Y3_END     TFC_XY_SCALE(TKR_Y3_END_MM,    TKR_K_STRIP_MM)





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
#define TKR_Z_NOMINALx2   TFC_Z_ABS_SCALE(TKR_Z_NOMINAL_MM * 2.0)



/* --------------------------------------------------------------------- *\
 |                                                                      |
 | Defines the absolute Z coordinate of any given X layer in mm.        |
 | and in units of TFC_Z_ABS_SCALE_FACTOR                               |
 |                                                                      |
\* -------------------------------------------------------------------- */ 
#define TKR_Z_XLAYER_MM(_layer) TFC_Z_XLAYER_MM(_layer, TKR_Z_NOMINAL_MM, \
                                                        TKR_Z_OFFSET_MM,  \
                                                        TKR_Z_DELTA_MM)   \


#define TKR_Z_XLAYER(_layer)    TFC_Z_ABS_SCALE(TKR_Z_XLAYER_MM(_layer))



/* --------------------------------------------------------------------- *\
 |                                                                       |
 | Defines the absolute Z coordinate of any given Y layer in mm.         |
 |                                                                       |
\* -------------------------------------------------------------------- */ 
#define TKR_Z_YLAYER_MM(_layer) TFC_Z_YLAYER_MM(_layer, TKR_Z_NOMINAL_MM,\
                                                        TKR_Z_OFFSET_MM, \
                                                        TKR_Z_DELTA_MM)


#define TKR_Z_YLAYER(_layer)    TFC_Z_ABS_SCALE(TKR_Z_YLAYER_MM(_layer))



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
#define ACD_TOP_Z_MM 575.248                    /* Z coordinate in mm    */
#define ACD_TOP_Z    TFC_Z_ABS_SCALE(ACD_TOP_Z_MM)   /* Z coordinate     */


/*
 | X TILES
 | -------
 | First define the width of the tiles. Tile 0 and tile 4 include the
 | the 50 mm corner piece. Each X tile overlaps its neighbor. We are
 | going to arbitrarily decide the each tile ends/begins at the halfway
 | point of the overlap. 
*/
#ifdef NEW_GEO
#define ACD_X_TOP_0_WIDTH_MM ( 50.0 + 300.0)
#define ACD_X_TOP_1_WIDTH_MM (338.0)
#define ACD_X_TOP_2_WIDTH_MM (334.0)
#define ACD_X_TOP_3_WIDTH_MM (338.0)
#define ACD_X_TOP_4_WIDTH_MM (300.0 +  50.0)


/*
 | For convenience the effective width of each tile is defined as
 | its nominal length - the overlap. This is an approximation to
 | reality, short-changing the overlap region where there are two
 | tiles which can register the hit. However, it is only in this
 | region that one has two chances, and at 99+% efficiency, this
 | should not be a problem
*/ 
#define ACD_X_TOP_OVERLAP_MM 10.0
#define ACD_X_TOP_OFFSET_MM +25.8

#define ACD_X_TOP_0_EWIDTH_MM (ACD_X_TOP_0_WIDTH_MM - ACD_X_TOP_OVERLAP_MM/2.0)
#define ACD_X_TOP_1_EWIDTH_MM (ACD_X_TOP_1_WIDTH_MM - ACD_X_TOP_OVERLAP_MM    )
#define ACD_X_TOP_2_EWIDTH_MM (ACD_X_TOP_2_WIDTH_MM - ACD_X_TOP_OVERLAP_MM    )
#define ACD_X_TOP_3_EWIDTH_MM (ACD_X_TOP_3_WIDTH_MM - ACD_X_TOP_OVERLAP_MM    )
#define ACD_X_TOP_4_EWIDTH_MM (ACD_X_TOP_4_WIDTH_MM - ACD_X_TOP_OVERLAP_MM/2.0)


#define ACD_X_TOP_0_BEG_MM  (ACD_X_TOP_1_BEG_MM   - ACD_X_TOP_0_EWIDTH_MM)
#define ACD_X_TOP_1_BEG_MM  (ACD_X_TOP_2_BEG_MM   - ACD_X_TOP_1_EWIDTH_MM)
#define ACD_X_TOP_2_BEG_MM  (ACD_X_TOP_OFFSET_MM  - ACD_X_TOP_2_EWIDTH_MM/2.0)
#define ACD_X_TOP_3_BEG_MM  (ACD_X_TOP_OFFSET_MM  + ACD_X_TOP_2_EWIDTH_MM/2.0)
#define ACD_X_TOP_4_BEG_MM  (ACD_X_TOP_3_BEG_MM   + ACD_X_TOP_3_EWIDTH_MM)
#define ACD_X_TOP_4_END_MM  (ACD_X_TOP_4_BEG_MM   + ACD_X_TOP_4_EWIDTH_MM)

#else

#define ACD_X_TOP_0_WIDTH_MM   327.126
#define ACD_X_TOP_1_WIDTH_MM   337.126
#define ACD_X_TOP_2_WIDTH_MM   337.126
#define ACD_X_TOP_3_WIDTH_MM   337.126
#define ACD_X_TOP_4_WIDTH_MM   347.126

#define ACD_X_TOP_OFFSET 23.0

#define ACD_X_TOP_0_CENTER_MM (-679.253 + ACD_X_TOP_OFFSET)
#define ACD_X_TOP_1_CENTER_MM (-347.126 + ACD_X_TOP_OFFSET)
#define ACD_X_TOP_2_CENTER_MM (  -1.000 + ACD_X_TOP_OFFSET)
#define ACD_X_TOP_3_CENTER_MM ( 327.126 + ACD_X_TOP_OFFSET)
#define ACD_X_TOP_4_CENTER_MM ( 669.253 + ACD_X_TOP_OFFSET)

#define ACD_X_TOP_0_BEG_MM   (ACD_X_TOP_0_CENTER_MM - ACD_X_TOP_0_WIDTH_MM/2.0)
#define ACD_X_TOP_1_BEG_MM   (ACD_X_TOP_1_CENTER_MM - ACD_X_TOP_1_WIDTH_MM/2.0)
#define ACD_X_TOP_2_BEG_MM   (ACD_X_TOP_2_CENTER_MM - ACD_X_TOP_2_WIDTH_MM/2.0)
#define ACD_X_TOP_3_BEG_MM   (ACD_X_TOP_3_CENTER_MM - ACD_X_TOP_3_WIDTH_MM/2.0)
#define ACD_X_TOP_4_BEG_MM   (ACD_X_TOP_4_CENTER_MM - ACD_X_TOP_4_WIDTH_MM/2.0)
#define ACD_X_TOP_4_END_MM   (ACD_X_TOP_4_CENTER_MM + ACD_X_TOP_4_WIDTH_MM/2.0)

#endif

#define ACD_X_TOP_0_BEG      TFC_XY_SCALE(ACD_X_TOP_0_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_TOP_1_BEG      TFC_XY_SCALE(ACD_X_TOP_1_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_TOP_2_BEG      TFC_XY_SCALE(ACD_X_TOP_2_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_TOP_3_BEG      TFC_XY_SCALE(ACD_X_TOP_3_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_TOP_4_BEG      TFC_XY_SCALE(ACD_X_TOP_4_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_TOP_4_END      TFC_XY_SCALE(ACD_X_TOP_4_END_MM, TKR_K_STRIP_MM)





/*
 | Y TILES
 | -------
 | First define the width of the tiles. 
*/
#ifdef NEW_GEO

#define ACD_Y_TOP_0_WIDTH_MM (334.0)
#define ACD_Y_TOP_1_WIDTH_MM (334.0)
#define ACD_Y_TOP_2_WIDTH_MM (318.0)
#define ACD_Y_TOP_3_WIDTH_MM (334.0)
#define ACD_Y_TOP_4_WIDTH_MM (334.0)

/*
 | For convenience the effective width of each tile is defined as its
 | nominal length + the overlap. This is an approximation to reality, 
 | occassionally causing the space in between the tiles to be associated
 | with one of its adjacent tiles. This slightly increases chances for a
 | false veto.
*/ 
#define ACD_Y_TOP_GAP_MM      3.0

#define ACD_Y_TOP_0_EWIDTH_MM (ACD_Y_TOP_0_WIDTH_MM + ACD_Y_TOP_GAP_MM/2.0)
#define ACD_Y_TOP_1_EWIDTH_MM (ACD_Y_TOP_1_WIDTH_MM + ACD_Y_TOP_GAP_MM    )
#define ACD_Y_TOP_2_EWIDTH_MM (ACD_Y_TOP_2_WIDTH_MM + ACD_Y_TOP_GAP_MM    )
#define ACD_Y_TOP_3_EWIDTH_MM (ACD_Y_TOP_3_WIDTH_MM + ACD_Y_TOP_GAP_MM    )
#define ACD_Y_TOP_4_EWIDTH_MM (ACD_Y_TOP_4_WIDTH_MM + ACD_Y_TOP_GAP_MM/2.0)


#define ACD_Y_TOP_OFFSET_MM   25.8


#define ACD_Y_TOP_0_BEG_MM  (ACD_Y_TOP_1_BEG_MM  - ACD_Y_TOP_0_EWIDTH_MM)
#define ACD_Y_TOP_1_BEG_MM  (ACD_Y_TOP_2_BEG_MM  - ACD_Y_TOP_1_EWIDTH_MM)
#define ACD_Y_TOP_2_BEG_MM  (ACD_Y_TOP_OFFSET_MM - ACD_Y_TOP_2_EWIDTH_MM/2.0)
#define ACD_Y_TOP_3_BEG_MM  (ACD_Y_TOP_OFFSET_MM + ACD_Y_TOP_2_EWIDTH_MM/2.0)
#define ACD_Y_TOP_4_BEG_MM  (ACD_Y_TOP_3_BEG_MM  + ACD_Y_TOP_3_EWIDTH_MM)
#define ACD_Y_TOP_4_END_MM  (ACD_Y_TOP_4_BEG_MM  + ACD_Y_TOP_4_EWIDTH_MM)

#else

#define ACD_Y_TOP_0_WIDTH_MM   327.126
#define ACD_Y_TOP_1_WIDTH_MM   337.126
#define ACD_Y_TOP_2_WIDTH_MM   337.126
#define ACD_Y_TOP_3_WIDTH_MM   337.126
#define ACD_Y_TOP_4_WIDTH_MM   347.126

#define ACD_Y_TOP_OFFSET       17.9

#define ACD_Y_TOP_0_CENTER_MM (-679.253 + ACD_Y_TOP_OFFSET)
#define ACD_Y_TOP_1_CENTER_MM (-347.126 + ACD_Y_TOP_OFFSET)
#define ACD_Y_TOP_2_CENTER_MM (  -1.000 + ACD_Y_TOP_OFFSET)
#define ACD_Y_TOP_3_CENTER_MM ( 327.126 + ACD_Y_TOP_OFFSET)
#define ACD_Y_TOP_4_CENTER_MM ( 669.253 + ACD_Y_TOP_OFFSET)

#define ACD_Y_TOP_0_BEG_MM   (ACD_Y_TOP_0_CENTER_MM - ACD_Y_TOP_0_WIDTH_MM/2.0)
#define ACD_Y_TOP_1_BEG_MM   (ACD_Y_TOP_1_CENTER_MM - ACD_Y_TOP_1_WIDTH_MM/2.0)
#define ACD_Y_TOP_2_BEG_MM   (ACD_Y_TOP_2_CENTER_MM - ACD_Y_TOP_2_WIDTH_MM/2.0)
#define ACD_Y_TOP_3_BEG_MM   (ACD_Y_TOP_3_CENTER_MM - ACD_Y_TOP_3_WIDTH_MM/2.0)
#define ACD_Y_TOP_4_BEG_MM   (ACD_Y_TOP_4_CENTER_MM - ACD_Y_TOP_4_WIDTH_MM/2.0)
#define ACD_Y_TOP_4_END_MM   (ACD_Y_TOP_4_CENTER_MM + ACD_Y_TOP_4_WIDTH_MM/2.0)

#endif

#define ACD_Y_TOP_0_BEG      TFC_XY_SCALE(ACD_Y_TOP_0_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_TOP_1_BEG      TFC_XY_SCALE(ACD_Y_TOP_1_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_TOP_2_BEG      TFC_XY_SCALE(ACD_Y_TOP_2_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_TOP_3_BEG      TFC_XY_SCALE(ACD_Y_TOP_3_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_TOP_4_BEG      TFC_XY_SCALE(ACD_Y_TOP_4_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_TOP_4_END      TFC_XY_SCALE(ACD_Y_TOP_4_END_MM, TKR_K_STRIP_MM)

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
#define ACD_X_Y0_WIDTH_MM 344.0
#define ACD_X_Y1_WIDTH_MM 334.0
#define ACD_X_Y2_WIDTH_MM 334.0
#define ACD_X_Y3_WIDTH_MM 334.0
#define ACD_X_Y4_WIDTH_MM 344.0

#define ACD_X_Y_OVERLAP_MM 10.0

#define ACD_X_Y0_EWIDTH_MM (ACD_X_Y0_WIDTH_MM - ACD_X_Y_OVERLAP_MM/2.0)
#define ACD_X_Y1_EWIDTH_MM (ACD_X_Y0_WIDTH_MM - ACD_X_Y_OVERLAP_MM    )
#define ACD_X_Y2_EWIDTH_MM (ACD_X_Y0_WIDTH_MM - ACD_X_Y_OVERLAP_MM    )
#define ACD_X_Y3_EWIDTH_MM (ACD_X_Y0_WIDTH_MM - ACD_X_Y_OVERLAP_MM    )
#define ACD_X_Y4_EWIDTH_MM (ACD_X_Y0_WIDTH_MM - ACD_X_Y_OVERLAP_MM/2.0)

#define ACD_X_Y_OFFSET_MM  0.0

#define ACD_X_Y0_BEG_MM  (ACD_X_Y1_BEG_MM   - ACD_X_Y0_EWIDTH_MM)
#define ACD_X_Y1_BEG_MM  (ACD_X_Y2_BEG_MM   - ACD_X_Y1_EWIDTH_MM)
#define ACD_X_Y2_BEG_MM  (ACD_X_Y_OFFSET_MM - ACD_X_Y2_EWIDTH_MM/2.0)
#define ACD_X_Y3_BEG_MM  (ACD_X_Y_OFFSET_MM + ACD_X_Y2_EWIDTH_MM/2.0)
#define ACD_X_Y4_BEG_MM  (ACD_X_Y3_BEG_MM   + ACD_X_Y3_EWIDTH_MM)
#define ACD_X_Y4_END_MM  (ACD_X_Y4_BEG_MM   + ACD_X_Y4_EWIDTH_MM)


#define ACD_X_Y0_BEG      TFC_XY_SCALE(ACD_X_Y0_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_Y1_BEG      TFC_XY_SCALE(ACD_X_Y1_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_Y2_BEG      TFC_XY_SCALE(ACD_X_Y2_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_Y3_BEG      TFC_XY_SCALE(ACD_X_Y3_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_Y4_BEG      TFC_XY_SCALE(ACD_X_Y4_BEG_MM, TKR_K_STRIP_MM)
#define ACD_X_Y4_END      TFC_XY_SCALE(ACD_X_Y4_END_MM, TKR_K_STRIP_MM)


#define ACD_XM_FACE_MM    -847.816
#define ACD_XP_FACE_MM     847.816

#define ACD_XM_FROM_TKR_MM (ACD_XM_FACE_MM - TKR_X0_BEG_MM)
#define ACD_XP_FROM_TKR_MM (ACD_XP_FACE_MM - TKR_X3_BEG_MM)

#define ACD_XM_FACE       TFC_XY_SCALE(ACD_XM_FACE_MM, TKR_K_STRIP_MM)
#define ACD_XP_FACE       TFC_XY_SCALE(ACD_XP_FACE_MM, TKR_K_STRIP_MM)

#if 0                                  
#define ACD_XM_FACE      -3731                  /* X coordinate, XM face */
#define ACD_XP_FACE       3732                  /* X coordinate, XP face */

#define ACD_XM_FROM_TKR  (ACD_XM_FACE - TKR_X0_BEG)    /* TKR to XM face */
#define ACD_XP_FROM_TKR  (ACD_XP_FACE - TKR_X3_BEG)    /* TKR to XP face */

#endif

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
#define ACD_Y_X0_WIDTH_MM 344.0
#define ACD_Y_X1_WIDTH_MM 334.0
#define ACD_Y_X2_WIDTH_MM 334.0
#define ACD_Y_X3_WIDTH_MM 334.0
#define ACD_Y_X4_WIDTH_MM 344.0

#define ACD_Y_X_OVERLAP_MM 10.0

#define ACD_Y_X0_EWIDTH_MM (ACD_Y_X0_WIDTH_MM - ACD_Y_X_OVERLAP_MM/2.0)
#define ACD_Y_X1_EWIDTH_MM (ACD_Y_X0_WIDTH_MM - ACD_Y_X_OVERLAP_MM    )
#define ACD_Y_X2_EWIDTH_MM (ACD_Y_X0_WIDTH_MM - ACD_Y_X_OVERLAP_MM    )
#define ACD_Y_X3_EWIDTH_MM (ACD_Y_X0_WIDTH_MM - ACD_Y_X_OVERLAP_MM    )
#define ACD_Y_X4_EWIDTH_MM (ACD_Y_X0_WIDTH_MM - ACD_Y_X_OVERLAP_MM/2.0)

#define ACD_Y_X_OFFSET_MM  0.0

#define ACD_Y_X0_BEG_MM  (ACD_Y_X1_BEG_MM   - ACD_Y_X0_EWIDTH_MM)
#define ACD_Y_X1_BEG_MM  (ACD_Y_X2_BEG_MM   - ACD_Y_X1_EWIDTH_MM)
#define ACD_Y_X2_BEG_MM  (ACD_Y_X_OFFSET_MM - ACD_Y_X2_EWIDTH_MM/2.0)
#define ACD_Y_X3_BEG_MM  (ACD_Y_X_OFFSET_MM + ACD_Y_X2_EWIDTH_MM/2.0)
#define ACD_Y_X4_BEG_MM  (ACD_Y_X3_BEG_MM   + ACD_Y_X3_EWIDTH_MM)
#define ACD_Y_X4_END_MM  (ACD_Y_X4_BEG_MM   + ACD_Y_X4_EWIDTH_MM)


#define ACD_Y_X0_BEG      TFC_XY_SCALE(ACD_Y_X0_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_X1_BEG      TFC_XY_SCALE(ACD_Y_X1_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_X2_BEG      TFC_XY_SCALE(ACD_Y_X2_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_X3_BEG      TFC_XY_SCALE(ACD_Y_X3_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_X4_BEG      TFC_XY_SCALE(ACD_Y_X4_BEG_MM, TKR_K_STRIP_MM)
#define ACD_Y_X4_END      TFC_XY_SCALE(ACD_Y_X4_END_MM, TKR_K_STRIP_MM)


#define ACD_YM_FACE_MM    -847.816
#define ACD_YP_FACE_MM     847.816

#define ACD_YM_FACE       TFC_XY_SCALE(ACD_YM_FACE_MM, TKR_K_STRIP_MM)
#define ACD_YP_FACE       TFC_XY_SCALE(ACD_YP_FACE_MM, TKR_K_STRIP_MM)

#define ACD_YM_FROM_TKR   TFC_XY_SCALE(ACD_YM_FROM_TKR_MM, TKR_K_STRIP_MM)
#define ACD_YP_FROM_TKR   TFC_XY_SCALE(ACD_YP_FROM_TKR_MM, TKR_K_STRIP_MM)

#if 0                                  

#define ACD_YM_FACE     -3687                   /* Y coordinate, YM face */
#define ACD_YP_FACE      3688                   /* Y coordinate, YP face */

#define ACD_YM_FROM_TKR (ACD_YM_FACE - TKR_Y0_BEG)    /* TKR to YM face  */
#define ACD_YP_FROM_TKR (ACD_YP_FACE - TKR_Y3_BEG)    /* TKR to YP face  */
#endif
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
#ifdef NEW_GEO
#define ACD_SIDE_Z0_WIDTH_MM 360.0
#define ACD_SIDE_Z1_WIDTH_MM 210.0
#define ACD_SIDE_Z2_WIDTH_MM 160.0
#define ACD_SIDE_Z3_WIDTH_MM 150.0

#define ACD_SIDE_Z_OVERLAP_MM 10.0

#define ACD_SIDE_Z0_EWIDTH_MM (ACD_SIDE_Z0_WIDTH_MM-ACD_SIDE_Z_OVERLAP_MM/2.0)
#define ACD_SIDE_Z1_EWIDTH_MM (ACD_SIDE_Z1_WIDTH_MM-ACD_SIDE_Z_OVERLAP_MM    )
#define ACD_SIDE_Z2_EWIDTH_MM (ACD_SIDE_Z2_WIDTH_MM-ACD_SIDE_Z_OVERLAP_MM    )
#define ACD_SIDE_Z3_EWIDTH_MM (ACD_SIDE_Z3_WIDTH_MM-ACD_SIDE_Z_OVERLAP_MM/2.0)

#define ACD_SIDE_Z0_TOP_MM  612.948
#define ACD_SIDE_Z1_TOP_MM (ACD_SIDE_Z0_TOP_MM - ACD_SIDE_Z0_EWIDTH_MM)
#define ACD_SIDE_Z2_TOP_MM (ACD_SIDE_Z1_TOP_MM - ACD_SIDE_Z1_EWIDTH_MM)
#define ACD_SIDE_Z3_TOP_MM (ACD_SIDE_Z2_TOP_MM - ACD_SIDE_Z2_EWIDTH_MM)
#define ACD_SIDE_Z3_BOT_MM (ACD_SIDE_Z3_TOP_MM - ACD_SIDE_Z3_EWIDTH_MM)

#else 0
#define ACD_SIDE_Z0_WIDTH_MM 267.5
#define ACD_SIDE_Z1_WIDTH_MM 200.0
#define ACD_SIDE_Z2_WIDTH_MM 150.0
#define ACD_SIDE_Z3_WIDTH_MM 150.0


#define ACD_SIDE_Z0_CENTER_MM 482.323         /* ACD side row 0, top Z mm */
#define ACD_SIDE_Z1_CENTER_MM 248.574         /* ACD side row 1, top Z mm */
#define ACD_SIDE_Z2_CENTER_MM  73.573         /* ACD side row 2, top Z mm */
#define ACD_SIDE_Z3_CENTER_MM -76.427         /* ACD side row 3, top Z mm */


#define ACD_SIDE_Z0_TOP_MM (ACD_SIDE_Z0_CENTER_MM + ACD_SIDE_Z0_WIDTH_MM/2.0)
#define ACD_SIDE_Z1_TOP_MM (ACD_SIDE_Z1_CENTER_MM + ACD_SIDE_Z1_WIDTH_MM/2.0)
#define ACD_SIDE_Z2_TOP_MM (ACD_SIDE_Z2_CENTER_MM + ACD_SIDE_Z2_WIDTH_MM/2.0)
#define ACD_SIDE_Z3_TOP_MM (ACD_SIDE_Z3_CENTER_MM + ACD_SIDE_Z3_WIDTH_MM/2.0)
#define ACD_SIDE_Z3_BOT_MM (ACD_SIDE_Z3_CENTER_MM - ACD_SIDE_Z3_WIDTH_MM/2.0)
#endif

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
#define ACD_TOP_XPROJECTION(_layer)                                      \
    TFC_ACD_TOP_PROJECTION(ACD_TOP_Z_MM,                                 \
                           TKR_Z_XLAYER_MM(_layer),                      \
                           TKR_Z_XLAYER_MM(_layer-1))


#define ACD_TOP_YPROJECTION(_layer)                                      \
    TFC_ACD_TOP_PROJECTION(ACD_TOP_Z_MM,                                 \
                           TKR_Z_YLAYER_MM(_layer),                      \
                           TKR_Z_YLAYER_MM(_layer-1))

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
#define SKIRT_Z_OFFSET_MM   0.0

#define SKIRT_Z_MM       (CAL_Z_OFFSET_MM - SKIRT_Z_OFFSET_MM)
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
 | Jan 11, 2004 - Added minus sign                                       |
 |                                                                       |
\* --------------------------------------------------------------------- */
#define SKIRT_XPROJECTION(_layer)                                        \
   -TFC_SKIRT_PROJECTION(SKIRT_Z_MM,                                     \
                         TKR_Z_XLAYER_MM(_layer),                        \
                         TKR_Z_XLAYER_MM(_layer-1))


#define SKIRT_YPROJECTION(_layer)                                        \
   -TFC_SKIRT_PROJECTION(SKIRT_Z_MM,                                     \
                         TKR_Z_YLAYER_MM(_layer),                        \
                         TKR_Z_YLAYER_MM(_layer-1))

/* --------------------------------------------------------------------- */


   


#define TKR_XY_WIDTHS                                               \
     {                                                              \
        TKR_X_ACTIVE,                                               \
        TKR_Y_ACTIVE                                                \
     }


#define TKR_X_BEGS                                                  \
     {                    /* X Offset to beginning of column n */   \
       TKR_X0_BEG,        /* X Offset to beginning of column 0 */   \
       TKR_X1_BEG,        /* X Offset to beginning of column 1 */   \
       TKR_X2_BEG,        /* X Offset to beginning of column 2 */   \
       TKR_X3_BEG         /* X Offset to beginning of column 3 */   \
     }

#define TKR_Z_XLAYERS                                               \
     {                    /* Z position of layer  Xn           */   \
       TKR_Z_XLAYER( 0),  /* Z position of layer  X0           */   \
       TKR_Z_XLAYER( 1),  /* Z position of layer  X1           */   \
       TKR_Z_XLAYER( 2),  /* Z position of layer  X2           */   \
       TKR_Z_XLAYER( 3),  /* Z position of layer  X3           */   \
       TKR_Z_XLAYER( 4),  /* Z position of layer  X4           */   \
       TKR_Z_XLAYER( 5),  /* Z position of layer  X5           */   \
       TKR_Z_XLAYER( 6),  /* Z position of layer  X6           */   \
       TKR_Z_XLAYER( 7),  /* Z position of layer  X7           */   \
       TKR_Z_XLAYER( 8),  /* Z position of layer  X8           */   \
       TKR_Z_XLAYER( 9),  /* Z position of layer  X9           */   \
       TKR_Z_XLAYER(10),  /* Z position of layer X10           */   \
       TKR_Z_XLAYER(11),  /* Z position of layer X11           */   \
       TKR_Z_XLAYER(12),  /* Z position of layer X12           */   \
       TKR_Z_XLAYER(13),  /* Z position of layer X13           */   \
       TKR_Z_XLAYER(14),  /* Z position of layer X14           */   \
       TKR_Z_XLAYER(15),  /* Z position of layer X15           */   \
       TKR_Z_XLAYER(16),  /* Z position of layer X16           */   \
       TKR_Z_XLAYER(17),  /* Z position of layer X17           */   \
     }


#define TKR_Y_BEGS                                                  \
     {                    /* Y Offset to beginning of row n    */   \
        TKR_Y0_BEG,       /* Y Offset to beginning of row 0    */   \
        TKR_Y1_BEG,       /* Y Offset to beginning of row 1    */   \
        TKR_Y2_BEG,       /* Y Offset to beginning of row 2    */   \
        TKR_Y3_BEG        /* Y Offset to beginning of row 3    */   \
     }


#define TKR_Z_YLAYERS                                               \
     {                    /* Z position of layer  Yn           */   \
       TKR_Z_YLAYER( 0),  /* Z position of layer  Y0           */   \
       TKR_Z_YLAYER( 1),  /* Z position of layer  Y1           */   \
       TKR_Z_YLAYER( 2),  /* Z position of layer  Y2           */   \
       TKR_Z_YLAYER( 3),  /* Z position of layer  Y3           */   \
       TKR_Z_YLAYER( 4),  /* Z position of layer  Y4           */   \
       TKR_Z_YLAYER( 5),  /* Z position of layer  Y5           */   \
       TKR_Z_YLAYER( 6),  /* Z position of layer  Y6           */   \
       TKR_Z_YLAYER( 7),  /* Z position of layer  Y7           */   \
       TKR_Z_YLAYER( 8),  /* Z position of layer  Y8           */   \
       TKR_Z_YLAYER( 9),  /* Z position of layer  Y9           */   \
       TKR_Z_YLAYER(10),  /* Z position of layer Y10           */   \
       TKR_Z_YLAYER(11),  /* Z position of layer Y11           */   \
       TKR_Z_YLAYER(12),  /* Z position of layer Y12           */   \
       TKR_Z_YLAYER(13),  /* Z position of layer Y13           */   \
       TKR_Z_YLAYER(14),  /* Z position of layer Y14           */   \
       TKR_Z_YLAYER(15),  /* Z position of layer Y15           */   \
       TKR_Z_YLAYER(16),  /* Z position of layer Y16           */   \
       TKR_Z_YLAYER(17),  /* Z position of layer Y17           */   \
     }


#define TKR_GEOMETRY                                           \
   {                                                           \
         TKR_K_STRIP,                                          \
        (TKR_Z_FIND_MAX  << 16) | TKR_Z_FIND_MIN,              \
        (TKR_Z_EXTEND_MAX<< 16) | TKR_Z_EXTEND_MIN,            \
         TKR_XY_WIDTHS,                                        \
      {  /* TKR.TWR Geometery          */                      \
         {                                                     \
            { TKR_X_BEGS, TKR_Z_XLAYERS },                     \
            { TKR_Y_BEGS, TKR_Z_YLAYERS }                      \
         }                                                     \
      }                                                        \
   }


#define ACD_X_TOP_EDGES                                             \
    {                                                               \
        ACD_X_TOP_0_BEG,                                            \
        ACD_X_TOP_1_BEG,                                            \
        ACD_X_TOP_2_BEG,                                            \
        ACD_X_TOP_3_BEG,                                            \
        ACD_X_TOP_4_BEG,                                            \
        ACD_X_TOP_4_END,                                            \
    }

#define ACD_Y_TOP_EDGES                                             \
    {                                                               \
        ACD_Y_TOP_0_BEG,                                            \
        ACD_Y_TOP_1_BEG,                                            \
        ACD_Y_TOP_2_BEG,                                            \
        ACD_Y_TOP_3_BEG,                                            \
        ACD_Y_TOP_4_BEG,                                            \
        ACD_Y_TOP_4_END,                                            \
    }


#define ACD_X_SIDE_YEDGES                                           \
    {                                                               \
       ACD_X_Y0_BEG,                                                \
       ACD_X_Y1_BEG,                                                \
       ACD_X_Y2_BEG,                                                \
       ACD_X_Y3_BEG,                                                \
       ACD_X_Y4_BEG,                                                \
       ACD_X_Y4_END                                                 \
    }


#define ACD_Y_SIDE_XEDGES                                           \
    {                                                               \
       ACD_Y_X0_BEG,                                                \
       ACD_Y_X1_BEG,                                                \
       ACD_Y_X2_BEG,                                                \
       ACD_Y_X3_BEG,                                                \
       ACD_Y_X4_BEG,                                                \
       ACD_Y_X4_END                                                 \
    }


#define ACD_Z_SIDE_ROWS                                             \
     {                                                              \
        ACD_SIDE_Z0_TOP,                                            \
        ACD_SIDE_Z1_TOP,                                            \
        ACD_SIDE_Z2_TOP,                                            \
        ACD_SIDE_Z3_TOP,                                            \
        ACD_SIDE_Z3_BOT,                                            \
        0                       /* Unused */                        \
     }


#define ACD_SIDE_FACES                                              \
     {                                                              \
        ACD_XM_FACE,                                                \
        ACD_XP_FACE,                                                \
        ACD_YM_FACE,                                                \
        ACD_YP_FACE,                                                \
     }


/*
 |  Gives the scale factor to project to the ACD. The index is the top layer
 |  of the tracker to project from. That is
 |
 |  X_at_acd =  X_top_layer
 |           + (X_top_layer - X_top_layer-1) * AcdXProjection[top_layer]
 |           / ACD_TOP_TKR_ZSCALE;
*/
#define ACD_TOP_XPROJECTIONS                                        \
    {                                                               \
       0,   /* -- Unused */                                         \
       0,   /* -- Unused */                                         \
       ACD_TOP_XPROJECTION( 2),   /* Extend from  2 to ACD */       \
       ACD_TOP_XPROJECTION( 3),   /* Extend from  3 to ACD */       \
       ACD_TOP_XPROJECTION( 4),   /* Extend from  4 to ACD */       \
       ACD_TOP_XPROJECTION( 5),   /* Extend from  5 to ACD */       \
       ACD_TOP_XPROJECTION( 6),   /* Extend from  6 to ACD */       \
       ACD_TOP_XPROJECTION( 7),   /* Extend from  7 to ACD */       \
       ACD_TOP_XPROJECTION( 8),   /* Extend from  8 to ACD */       \
       ACD_TOP_XPROJECTION( 9),   /* Extend from  9 to ACD */       \
       ACD_TOP_XPROJECTION(10),   /* Extend from 10 to ACD */       \
       ACD_TOP_XPROJECTION(11),   /* Extend from 11 to ACD */       \
       ACD_TOP_XPROJECTION(12),   /* Extend from 12 to ACD */       \
       ACD_TOP_XPROJECTION(13),   /* Extend from 13 to ACD */       \
       ACD_TOP_XPROJECTION(14),   /* Extend from 14 to ACD */       \
       ACD_TOP_XPROJECTION(15),   /* Extend from 15 to ACD */       \
       ACD_TOP_XPROJECTION(16),   /* Extend from 16 to ACD */       \
       ACD_TOP_XPROJECTION(17),   /* Extend from 17 to ACD */       \
    }



#define ACD_TOP_YPROJECTIONS                                        \
   {                             /* ACD TOP PROJECTION    */        \
       0,   /* -- Unused */                                         \
       0,   /* -- Unused */                                         \
       ACD_TOP_YPROJECTION( 2),   /* Extend from  2 to ACD */       \
       ACD_TOP_YPROJECTION( 3),   /* Extend from  3 to ACD */       \
       ACD_TOP_YPROJECTION( 4),   /* Extend from  4 to ACD */       \
       ACD_TOP_YPROJECTION( 5),   /* Extend from  5 to ACD */       \
       ACD_TOP_YPROJECTION( 6),   /* Extend from  6 to ACD */       \
       ACD_TOP_YPROJECTION( 7),   /* Extend from  7 to ACD */       \
       ACD_TOP_YPROJECTION( 8),   /* Extend from  8 to ACD */       \
       ACD_TOP_YPROJECTION( 9),   /* Extend from  9 to ACD */       \
       ACD_TOP_YPROJECTION(10),   /* Extend from 10 to ACD */       \
       ACD_TOP_YPROJECTION(11),   /* Extend from 11 to ACD */       \
       ACD_TOP_YPROJECTION(12),   /* Extend from 12 to ACD */       \
       ACD_TOP_YPROJECTION(13),   /* Extend from 13 to ACD */       \
       ACD_TOP_YPROJECTION(14),   /* Extend from 14 to ACD */       \
       ACD_TOP_YPROJECTION(15),   /* Extend from 15 to ACD */       \
       ACD_TOP_YPROJECTION(16),   /* Extend from 16 to ACD */       \
       ACD_TOP_YPROJECTION(17),   /* Extend from 17 to ACD */       \
    }
 
#define ACD_GEOMETRY                                              \
   {                                                              \
      TKR_Z_NOMINALx2,                                            \
      ACD_X_TOP_EDGES,                                            \
      ACD_Y_TOP_EDGES,                                            \
      ACD_X_SIDE_YEDGES,                                          \
      ACD_X_SIDE_YEDGES,                                          \
      ACD_Y_SIDE_XEDGES,                                          \
      ACD_Y_SIDE_XEDGES,                                          \
      ACD_Z_SIDE_ROWS,                                            \
      ACD_SIDE_FACES,                                             \
      ACD_TOP_XPROJECTIONS,                                       \
      ACD_TOP_YPROJECTIONS                                        \
   }



#define SKIRT_XPROJECTIONS                                        \
    {                                                             \
       SKIRT_XPROJECTION( 0),   /* Extend from  0 to SKIRT */     \
       SKIRT_XPROJECTION( 1),   /* Extend from  1 to SKIRT */     \
       SKIRT_XPROJECTION( 2),   /* Extend from  2 to SKIRT */     \
       SKIRT_XPROJECTION( 3),   /* Extend from  3 to SKIRT */     \
       SKIRT_XPROJECTION( 4),   /* Extend from  4 to SKIRT */     \
       SKIRT_XPROJECTION( 5),   /* Extend from  5 to SKIRT */     \
       SKIRT_XPROJECTION( 6),   /* Extend from  6 to SKIRT */     \
       SKIRT_XPROJECTION( 7),   /* Extend from  7 to SKIRT */     \
       SKIRT_XPROJECTION( 8),   /* Extend from  8 to SKIRT */     \
       SKIRT_XPROJECTION( 9),   /* Extend from  9 to SKIRT */     \
       SKIRT_XPROJECTION(10),   /* Extend from 10 to SKIRT */     \
       SKIRT_XPROJECTION(11),   /* Extend from 11 to SKIRT */     \
       SKIRT_XPROJECTION(12),   /* Extend from 12 to SKIRT */     \
       SKIRT_XPROJECTION(13),   /* Extend from 13 to SKIRT */     \
       SKIRT_XPROJECTION(14),   /* Extend from 14 to SKIRT */     \
       SKIRT_XPROJECTION(15),   /* Extend from 15 to SKIRT */     \
       0,                       /* -- Unused --          */       \
       0,                       /* -- Unused --          */       \
    }

#define SKIRT_YPROJECTIONS                                        \
    {                                                             \
       SKIRT_YPROJECTION( 0),   /* Extend from  0 to SKIRT */     \
       SKIRT_YPROJECTION( 1),   /* Extend from  1 to SKIRT */     \
       SKIRT_YPROJECTION( 2),   /* Extend from  2 to SKIRT */     \
       SKIRT_YPROJECTION( 3),   /* Extend from  3 to SKIRT */     \
       SKIRT_YPROJECTION( 4),   /* Extend from  4 to SKIRT */     \
       SKIRT_YPROJECTION( 5),   /* Extend from  5 to SKIRT */     \
       SKIRT_YPROJECTION( 6),   /* Extend from  6 to SKIRT */     \
       SKIRT_YPROJECTION( 7),   /* Extend from  7 to SKIRT */     \
       SKIRT_YPROJECTION( 8),   /* Extend from  8 to SKIRT */     \
       SKIRT_YPROJECTION( 9),   /* Extend from  9 to SKIRT */     \
       SKIRT_YPROJECTION(10),   /* Extend from 10 to SKIRT */     \
       SKIRT_YPROJECTION(11),   /* Extend from 11 to SKIRT */     \
       SKIRT_YPROJECTION(12),   /* Extend from 12 to SKIRT */     \
       SKIRT_YPROJECTION(13),   /* Extend from 13 to SKIRT */     \
       SKIRT_YPROJECTION(14),   /* Extend from 14 to SKIRT */     \
       SKIRT_YPROJECTION(15),   /* Extend from 15 to SKIRT */     \
       0,                       /* -- Unused --          */       \
       0,                       /* -- Unused --          */       \
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



/* ---------------------------------------------------------------------- *//*!

  \var   const struct _TFC_geometry TFC_GeometryStd
  \brief Defines the standard (nominal) geometry for the LAT in a form
         that the filtering code can readily digest. That is, this is
         not a full and general description of the LAT, but it is tailored
         for use by the filter program.
                                                                          */
/* ---------------------------------------------------------------------- */
const struct _TFC_geometry TFC_GeometryStd =
{
   TFC_STD_TAG,
   TKR_GEOMETRY,
   ACD_GEOMETRY,
   SKIRT_GEOMETRY
};
/* ---------------------------------------------------------------------- */



