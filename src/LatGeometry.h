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

#include "GGF_DB/src/GEO_DB_data.h"
#endif

