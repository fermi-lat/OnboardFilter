#ifndef AFC_SPLASH_H
#define AFC_SPLASH_H


/* ---------------------------------------------------------------------- *//*!

   \file   AFC_splash.h
   \brief  Interface to the ACD splash veto evaluation
   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $Id
\endverbatim
                                                                          */
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \def   AFC_M_VETO_SIDE_TILES
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
                        int                        acd_x,
                        int                        acd_y,
                        int                        acd_z);


#ifdef __cplusplus
}
#endif


#endif




