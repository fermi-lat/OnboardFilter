#ifndef TFC_ACD_H
#define TFC_ACD_H



/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!

   \file   TFC_acd.h
   \brief  Interface to TKR->ACD projection and matching routines
   \author JJRussell - russell@slac.stanford.edu

                                                                         */
/* --------------------------------------------------------------------- */


#ifdef __cplusplus
extern "C" {
#endif

struct _TFC_projection;
struct _TFC_geometry;



extern int TFC_acdProject   (struct _TFC_projection        *prj,
                             int                           xCnt,
                             int                           yCnt,
                             const struct _TFC_geometry    *geo,
                             int                        towerId,
                             int                       dispatch,
                             int                          acd_x,
                             int                          acd_y,
                             int                          acd_z);

extern int TFC_acdProjectTemplate (int             acd_x,
                                   int             acd_y,
                                   int             acd_z);

#ifdef __cplusplus
}
#endif



#endif
