#ifndef TFC_SKIRT_H
#define TFC_SKIRT_H


/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   TFC_skirt.h
   \brief  Interface to TKR skirt projection routines
   \author JJRussell - russell@slac.stanford.edu

                                                                         */
/* --------------------------------------------------------------------- */



/*
  
*/

#ifdef __cplusplus
extern "C" {
#endif    

struct _TFC_projection;
struct _TFC_geometry;
    
/* --------------------------------------------------------------------- *//*!

  \enum _TFC_skirtRegion
  \brief The enumeration of the 9 skirt regions, see diagram below

  The skirt region is the defined as the area between the towers and
  the ACD at plane of the TKR/CAL boundary. The region is numbered as
  follows

  \verbatim
      +------------------------+
      |   |                |   |
      | 5 |        6       | 7 |      
      |   |                |   |      
      |---+----------------+---|
      |   |                |   |
      |   |                |   |
      |   |                |   |
      |   |                |   |
      | 3 |                | 4 |
      |   |                |   |
      |   |                |   |
      |   |                |   |
      |   |                |   |
      |   |                |   |      
      |---+----------------+---|
      |   |                |   |
      | 0 |       1        | 2 |
      |   |                |   |      
      +------------------------+

  \endverbatim
                                                                         *//*!
  \typedef TFC_skirtRegion
  \brief   Typedef for enum \e _TFC_skirtRegion
                                                                         */
/* --------------------------------------------------------------------- */  
typedef enum _TFC_skirtRegion
{
   TFC_SKIRT_K_REGION_YM_XM = 0, /*!< Lower  left   region               */
   TFC_SKIRT_K_REGION_YM_XC = 1, /*!< Lower  center region               */
   TFC_SKIRT_K_REGION_YM_XP = 2, /*!< Lower  right  region               */
   TFC_SKIRT_K_REGION_YC_XM = 3, /*!< Center left   region               */
   TFC_SKIRT_K_REGION_YC_XP = 4, /*!< Center right  region               */
   TFC_SKIRT_K_REGION_YP_XM = 5, /*!< Upper  left   region               */
   TFC_SKIRT_K_REGION_YP_XC = 6, /*!< Upper  center region               */
   TFC_SKIRT_K_REGION_YP_XP = 7  /*!< Upper  right  region               */
}
TFC_skirtRegion;
/* --------------------------------------------------------------------- */
    
    
/* --------------------------------------------------------------------- *//*!

  \enum _TFC_skirtRegion_m
  \brief Enumeration of the skirt regions as a bit mask
                                                                         *//*!
  \typedef TFC_skirtRegion_m
  \brief   Typedef for enum \e _TFC_skirtRegion_m
                                                                         */
/* --------------------------------------------------------------------- */    
typedef enum _TFC_skirtRegion_m
{
   TFC_SKIRT_M_REGION_YM_XM  = (1 << TFC_SKIRT_K_REGION_YM_XM),
   /*!< Lower  left   region */
   
   TFC_SKIRT_M_REGION_YM_XC  = (1 << TFC_SKIRT_K_REGION_YM_XC),
   /*!< Lower  middle region */
   
   TFC_SKIRT_M_REGION_YM_XP  = (1 << TFC_SKIRT_K_REGION_YM_XP),
   /*!< Lower  right  region */
   
   TFC_SKIRT_M_REGION_YC_XM  = (1 << TFC_SKIRT_K_REGION_YC_XM),
   /*!< Middle left   region */
   
   TFC_SKIRT_M_REGION_YC_XP  = (1 << TFC_SKIRT_K_REGION_YC_XP),
   /*!< Middle right  region */
   
   TFC_SKIRT_M_REGION_YP_XM  = (1 << TFC_SKIRT_K_REGION_YP_XM),
   /*!< Upper  left   region */
   
   TFC_SKIRT_M_REGION_YP_XC  = (1 << TFC_SKIRT_K_REGION_YP_XC),
   
   /*!< Upper  middle region */
   TFC_SKIRT_M_REGION_YP_XP  = (1 << TFC_SKIRT_K_REGION_YP_XP)
   /*!< Upper  right  region */
}
TFC_skirtRegion_m;
/* --------------------------------------------------------------------- */


/* --------------------------------------------------------------------- *//*!

  \enum _TFC_skirtRegions
  \brief Enumeration of the bit masks representing commonly used
         collections of the primitive skirt regions.
                                                                         *//*!
  \typedef TFC_skirtRegions
  \brief   Typedef for enum \e TFC_skirtRegions
                                                                         */
/* --------------------------------------------------------------------- */
typedef enum _TFC_skirtRegions
{
   TFC_SKIRT_REGIONS_XM    = (TFC_SKIRT_M_REGION_YM_XM  |
                              TFC_SKIRT_M_REGION_YC_XM  |
                              TFC_SKIRT_M_REGION_YP_XM),
   /*!< Skirt template for XM region */

   
   TFC_SKIRT_REGIONS_XC    = (TFC_SKIRT_M_REGION_YM_XC  |
                              TFC_SKIRT_M_REGION_YP_XC),
   /*!< Skirt template for X middle region */
   
   
   TFC_SKIRT_REGIONS_XP    = (TFC_SKIRT_M_REGION_YM_XP  |
                              TFC_SKIRT_M_REGION_YC_XP  |
                              TFC_SKIRT_M_REGION_YP_XP),
   /*!< Skirt template for XP region */


   
   TFC_SKIRT_REGIONS_YM    = (TFC_SKIRT_M_REGION_YM_XM  |
                              TFC_SKIRT_M_REGION_YM_XC  |
                              TFC_SKIRT_M_REGION_YM_XP),
   /*!< Skirt template for YM region */

   
   TFC_SKIRT_REGIONS_YC    = (TFC_SKIRT_M_REGION_YC_XM  |
                              TFC_SKIRT_M_REGION_YC_XP),
   /*!< Skirt template for Y middle region */

   
   TFC_SKIRT_REGIONS_YP    = (TFC_SKIRT_M_REGION_YP_XM  |
                              TFC_SKIRT_M_REGION_YP_XC  |
                              TFC_SKIRT_M_REGION_YP_XP),
   /*!< Skirt template for YP region */
}
TFC_skirtRegions;
/* --------------------------------------------------------------------- */

   
extern int TFC_skirtProject (struct _TFC_projection        *prj,
                             int                           xCnt,
                             int                           yCnt,
                             const struct _TFC_geometry    *geo,
                             int                        towerId);
    
#ifdef __cplusplus
}
#endif    



#endif
