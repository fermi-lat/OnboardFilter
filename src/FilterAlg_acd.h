#ifndef TFC_ACD_H
#define TFC_ACD_H

/* ---------------------------------------------------------------------- *//*!
   
   \file   TFC_acd.h
   \brief  Interface to TKR->ACD projection and matching routines
   \author JJRussell - russell@slac.stanford.edu

\verbatim

    CVS $Id: TFC_acd.h,v 1.1.1.1 2005/05/10 14:44:25 russell Exp $
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
 * 12.22.04 jjr Cloned from DFC/src/TFC_acd.h
 *
\* ---------------------------------------------------------------------- */



#ifdef __cplusplus
extern "C" {
#endif    


/* --------------------------------------------------------------------- */
#ifndef CMX_DOXYGEN

#ifndef TFC_PROJECTION_TD
#define TFC_PROJECTION_TD
typedef struct _TFC_projection TFC_projection;
#endif

#ifndef TFC_GEOMETRY_TD
#define TFC_GEOMETRY_TD
typedef struct _TFC_geometry TFC_geometry;
#endif

#endif
/* --------------------------------------------------------------------- */
    
    

/* --------------------------------------------------------------------- */
  extern int TFC_acdProject         (TFC_projection        *prj,
				     int                   xCnt,
				     int                   yCnt,
				     const TFC_geometry    *geo,
				     int                towerId,
				     int               dispatch,
				     int                  acd_x,
				     int                  acd_y,
				     int                  acd_z);
  
  extern int TFC_acdProjectTemplate (int                  acd_x,
				     int                  acd_y,
				     int                  acd_z);
/* --------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif    

#endif
