#ifndef AFC_SPLASH_H
#define AFC_SPLASH_H

/*------------------------------------------------------------------------
| CVS $Id$
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   AFC_splash.h
   \brief  Interface to the ACD splash veto evaluation
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \def AFC_M_VETO_SIDE_TILES
  \brief Primary AFC vetoing uses only the top 2 rows
                                                                          */
/* ---------------------------------------------------------------------- */
#  define AFC_M_VETO_SIDE_TILES  ((0x1f<<5) | (0x1f))
/* ---------------------------------------------------------------------- */



#ifdef __cplusplus
extern "C" {
#endif

struct _AFC_splashMap;
    
    
extern int AFC_splash  (const struct _AFC_splashMap *map,
                        int                      acd_top,
                        int                        acd_x,
                        int                        acd_y);


#ifdef __cplusplus
}
#endif


#endif




