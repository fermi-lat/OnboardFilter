/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   ATF_shadowedTowers2x2.c
   \brief  Initializes the data structure which translates a list of
           struct ACD tiles into a list of shadowed towers. The
           overlap is a 2x2 grid of ACD tiles for each tower
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */


#include "ATF_shadowedTowersCtl.h"

/* ---------------------------------------------------------------------- *//*!

  \def   TWR(_n)
  \brief Macro to set the bit in a tower mask. This macro is UNDEF'd,
         but DOXYGEN gives one no alternative except to document.
                                                                          */
/* ---------------------------------------------------------------------- */
#define TWR(_n)  (0x8000 >> 0x ## _n)

#ifdef __cplusplus
extern "C" {
#endif


    
/* ---------------------------------------------------------------------- *//*!

  \var   struct _ATF_shadowedTowersCtl ATF_ShadowedTowers2x2
  \brief Controls which tiles shadow which towers. 

   This is a readonly data structure. It consists of 3 arrays indexed
   by tile number. The first array controls the top tiles and the
   other two array controls the X & Y side tiles. Symmetry allows one
   to use the X/Y maps for both the ends. The maps are packed in MSB
   order, ie tower 0 is the MSB bit. This is consistent with the FFS
   routine.
                                                                          */
/* ---------------------------------------------------------------------- */
    const struct _ATF_shadowedTowersCtl ATF_ShadowedTowers2x2 =    
{
  
   /*  Candidate towers by top ACD tile             */
   {
      TWR(F),                            /* Tile 24 */
      TWR(E) | TWR(F),                   /* Tile 23 */
      TWR(D) | TWR(E),                   /* Tile 22 */
      TWR(C) | TWR(D),                   /* Tile 21 */
      TWR(C),                            /* Tile 20 */
          
      TWR(B) | TWR(F),                   /* Tile 19 */
      TWR(A) | TWR(B) | TWR(E) | TWR(F), /* Tile 18 */
      TWR(9) | TWR(A) | TWR(D) | TWR(E), /* Tile 17 */
      TWR(8) | TWR(9) | TWR(C) | TWR(D), /* Tile 16 */
      TWR(8) | TWR(C),                   /* Tile 15 */
          
      TWR(7) | TWR(B),                   /* Tile 14 */
      TWR(6) | TWR(7) | TWR(A) | TWR(B), /* Tile 13 */
      TWR(5) | TWR(6) | TWR(9) | TWR(A), /* Tile 12 */
      TWR(4) | TWR(5) | TWR(8) | TWR(9), /* Tile 11 */
      TWR(4) | TWR(8),                   /* Tile 10 */
      
      TWR(3) | TWR(7),                   /* Tile  9 */
      TWR(2) | TWR(3) | TWR(6) | TWR(7), /* Tile  8 */
      TWR(1) | TWR(2) | TWR(5) | TWR(6), /* Tile  7 */
      TWR(0) | TWR(1) | TWR(4) | TWR(5), /* Tile  6 */
      TWR(0) | TWR(4),                   /* Tile  5 */
      
      TWR(3),                            /* Tile  4 */
      TWR(2) | TWR(3),                   /* Tile  3 */
      TWR(1) | TWR(2),                   /* Tile  2 */
      TWR(0) | TWR(1),                   /* Tile  1 */
      TWR(0)                             /* Tile  0 */
   },

   /* Candidate towers by X ACD tile                */
   {
      TWR(C),                            /* Tile 14 */
      TWR(8) | TWR(C),                   /* Tile 13 */
      TWR(4) | TWR(8),                   /* Tile 12 */
      TWR(0) | TWR(4),                   /* Tile 11 */
      TWR(0),                            /* Tile 10 */
      
      TWR(C),                            /* Tile  9 */
      TWR(8) | TWR(C),                   /* Tile  8 */
      TWR(4) | TWR(8),                   /* Tile  7 */
      TWR(0) | TWR(4),                   /* Tile  6 */
      TWR(0),                            /* Tile  5 */
      
      TWR(C),                            /* Tile  4 */
      TWR(8) | TWR(C),                   /* Tile  3 */
      TWR(4) | TWR(8),                   /* Tile  2 */
      TWR(0) | TWR(4),                   /* Tile  1 */
      TWR(0),                            /* Tile  0 */
   },

   /* Candidate towers by Y ACD tile                */
   {
      TWR(3),                            /* Tile 14 */
      TWR(2) | TWR(3),                   /* Tile 13 */
      TWR(1) | TWR(2),                   /* Tile 12 */
      TWR(0) | TWR(1),                   /* Tile 11 */
      TWR(0),                            /* Tile 10 */

      TWR(3),                            /* Tile  9 */
      TWR(2) | TWR(3),                   /* Tile  8 */
      TWR(1) | TWR(2),                   /* Tile  7 */
      TWR(0) | TWR(1),                   /* Tile  6 */
      TWR(0),                            /* Tile  5 */
      
      TWR(3),                            /* Tile  4 */
      TWR(2) | TWR(3),                   /* Tile  3 */
      TWR(1) | TWR(2),                   /* Tile  2 */
      TWR(0) | TWR(1),                   /* Tile  1 */
      TWR(0),                            /* Tile  0 */
   }
  
};

    
    
#undef TWR
    
#ifdef __cplusplus
}
#endif






