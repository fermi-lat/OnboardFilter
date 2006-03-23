/* ---------------------------------------------------------------------- *//*!
   
   \file   TFC_skirt.c
   \brief  Routines to project XY projections to the skirt region
   \author JJRussell - russell@slac.stanford.edu

\verbatim

    CVS $Id: FilterAlg_skirt.cxx,v 1.2 2005/09/15 17:46:11 hughes Exp $
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
 * 12.22.04 jjr Cloned from DFC/src/TFC_skirt.c
 *
\* ---------------------------------------------------------------------- */




#include "EFC/EDM.h"
#include "TFC_geometryDef.h"
#include "EFC/TFC_projectionDef.h"
#include "TFC_projectionFind.h"
#include "TFC_geometryDef.h"
#include "EDS/EDR_tkr.h"
#include "EFC/TFC_geos.h"
#include "EFC/TFC_geoIds.h"

#include "FilterAlg_skirt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------- */
#ifndef CMX_DOXYGEN
/* ---------------------------------------------------------------------- */

EDM_CODE (EDM_level TFC_Skirt_edm = EDM_K_DEBUG);
EDM_CODE (static int printSkirtTitle (void));

/* ---------------------------------------------------------------------- */
#endif
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
#ifndef CMX_DOXYGEN
/* ---------------------------------------------------------------------- */
static __inline int getRegionMask (int             position,
				   const short int edges[4],
				   int         leftTemplate,
				   int          midTemplate,
				   int        rightTemplate);



static __inline int projectXYtoSkirt (TFC_projection             *prj,
				      int                     towerId,
				      int                      offset,
				      const unsigned short int  z[18],
				      const short int        edges[4],
				      int                leftTemplate,
				      int                 midTemplate,
				      int               rightTemplate);
#ifdef __cplusplus
}
#endif

    
static __inline int getRegionMask (int             position,
				   const short int edges[4],
				   int         leftTemplate,
				   int          midTemplate,
				   int        rightTemplate)
{
 
   /*  Is it outside the left edge */
   if (position > edges[1])
   {
       /* Is it in the middle region */
       if      (position < edges[2]) return midTemplate;
       else if (position < edges[3]) return rightTemplate;
       else                          return 0;
   }
   else if (position > edges[0])  return leftTemplate;

   return 0;
}



static __inline int projectXYtoSkirt (TFC_projection             *prj,
				      int                     towerId,
				      int                      offset,
				      const unsigned short int  z[18],
				      const short int        edges[4],
				      int                leftTemplate,
				      int                 midTemplate,
				      int               rightTemplate)
{
   int botLayerNum;
   
   botLayerNum = prj->min;
   
   EDM_DEBUGPRINTF ((TFC_Skirt_edm, " %3d %3d", towerId, botLayerNum));

   /*
    | !!! KLUDGE !!!
    | ==============
    | Should verify that there are not too many missed layers on
    | the way to the skirt. Cheating for now.
   */
   {
       int            deltaZ;
       int          position;
       int             slope;
       int            botHit;
       int           regions;
       int               min;
       const short int *hits;

       min      =  prj->min;
       hits     = &prj->hits[min];
       deltaZ   = z[botLayerNum];
       botHit   = hits[0];
       slope    = botHit - hits[1];
       position = botHit
                + offset + (deltaZ * slope) / TFC_Z_TKR_TO_SKIRT_SCALE_FACTOR;

       regions  = getRegionMask (position,
                                 edges,
                                 leftTemplate,
                                 midTemplate,
                                 rightTemplate);
       
       EDM_DEBUGPRINTF ((TFC_Skirt_edm,
			 " %9d + %6d + %10d * %5d =  %7d     %2.2x\n",
			 botHit,
			 offset,
			 deltaZ,
			 slope,
			 position,
			 regions));

       return regions;
   }
   
   return 0;
}
/* ---------------------------------------------------------------------- */
#endif
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn  int TFC_skirtProject (TFC_projection     *prj,
                             int                xCnt,
                             int                yCnt,
                             const TFC_geometry *geo,
                             int             towerId)
  \brief Checks the specified list of projections, looking for regions
         of skirt that these projections pass through.

  \param prj     The list of projections, assumed X projections first
                 followed by the Y projections
  \param xCnt    The number of X projections
  \param yCnt    The number of Y projections
  \param geo     The LAT geometry specification
  \param towerId The tower Id of the projections
  \return        The XY regions that the projections pass through
                                                                          */
/* ---------------------------------------------------------------------- */
int FilterAlg_skirtProject (TFC_projection     *prj,
                      int                xCnt,
                      int                yCnt,
                      const TFC_geometry *geo,
                      int             towerId)
{
   int    row;
   int    col;
   int offset;
   int  xMask;
   int  yMask;
   EDM_CODE (int printTitle = 1);
   
   /*
    | Project the X track projections to the TKR/CAL boundary.
    | A bit mask of candidate skirt regions is formed. This will
    | be a column of Y regions.
   */
   col    = (towerId & 0x3);
   offset = geo->tkr.twr.xy[0].offsets[col];
   xMask  = 0;
   while (--xCnt >= 0)
   {
       int mask;

       EDM_CODE (if (printTitle) printTitle = printSkirtTitle ());
       
       mask = projectXYtoSkirt (prj,
                                towerId,
                                offset,
                                geo->skirt.xProjections,
                                geo->skirt.xy[0],
                                TFC_SKIRT_REGIONS_XM,
                                TFC_SKIRT_REGIONS_XC,  
                                TFC_SKIRT_REGIONS_XP);
       xMask         |= mask;
       prj->skirtMask = mask;
       prj++;
   }

   
   /*
    | Project the Y track projections to the TKR/CAL boundary.
    | A bit mask of candidate skirt regions. This will be a row
    | of X regions.
   */
   row      = (towerId >> 2) & 0x3;
   offset   = geo->tkr.twr.xy[1].offsets[row];
   yMask    = 0;                         
   while (--yCnt >= 0)
   {
       int mask;
       
       EDM_CODE (if (printTitle) printTitle = printSkirtTitle ());
     
       mask = projectXYtoSkirt (prj,
                                towerId,
                                offset,
                                geo->skirt.yProjections,
                                geo->skirt.xy[1],
                                TFC_SKIRT_REGIONS_YM,
                                TFC_SKIRT_REGIONS_YC,  
                                TFC_SKIRT_REGIONS_YP);

       yMask         |= mask;
       prj->skirtMask = mask;
       prj++;
   }

   EDM_DEBUGPRINTF ((TFC_Skirt_edm,
		     "Coincidence = %2.2x & %2.2x = %2.2x %s\n",
		     xMask,
		     yMask,
		     xMask & yMask,
		     (xMask & yMask) ? "SKIRT REJECT\n" : ""));
   
   return xMask & yMask;
}
/* ---------------------------------------------------------------------- */






#ifdef EDM_USE
/* ---------------------------------------------------------------------- *//*!

  \fn     int prjAcd_TopTitlePrint (void)
  \brief  Diagnostic routine to print the title line when projecting to
          ACD TOP face.
  \retval 0          
                                                                          */
/* ---------------------------------------------------------------------- */
static int printSkirtTitle (void)
{
  EDM_DEBUGPRINTF ((TFC_Skirt_edm,
   "\n"
   " Project to SKIRT\n"
   " Twr Lyr Intercept + Offset + projection * slope = At Skirt Region\n"
   " --- --- ---------   ------   ----------   -----   -------- ------\n"));

    return 0;
}
/* ---------------------------------------------------------------------- */
#endif

