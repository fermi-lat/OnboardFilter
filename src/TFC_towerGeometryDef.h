#ifndef TFC_TOWERGEOMETRYDEF_H
#define TFC_TOWERGEOMETRYDEF_H



/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   TFC_latRecord.h
   \brief  Defines the tower geometry used in track finding
   \author JJRussell - russell@slac.stanford.edu

                                                                         */
/* --------------------------------------------------------------------- */
typedef struct _TFC_towerGeometryXY
{
    TFC_towerOffset offsets[4]; /* Offset to beginning of row or col n */
    TFC_towerZ           z[18]; /* Z position of layer n               */
}
TFC_towerGeometryXY;


typedef struct _TFC_towerGeometry
{
    TFC_towerGeometryXY x;    /* Geometry of the X layers             */
    TFC_towerGeometryXY y;    /* Geometry of the Y layers             */
}
TFC_towerGeometry;



#ifdef __cplusplus
}
#endif    



#endif
