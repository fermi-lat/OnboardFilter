/* ---------------------------------------------------------------------- *//*!
   
   \file   TFC_geos.c
   \brief  Organizes all the known geometries.
   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $Id
\endverbatim
									  */
/* ---------------------------------------------------------------------- */


#include "DFC/TFC_geos.h"
#include "DFC/TFC_geoIds.h"
#include "TFC_geometryDef.h"


extern const struct _TFC_geometry TFC_GeometryStd;
extern const struct _TFC_geometry TFC_Geo_v1r13p0;


/* ---------------------------------------------------------------------- */
/*!  \def TFC_K_GEO_ID_CURRENT  \brief < The current default Id           */
/* ---------------------------------------------------------------------- */  
#define TFC_K_GEO_ID_CURRENT TFC_K_GEO_ID_V1_R13_P0
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

 \var   struct _TFC_geometry *TFC_Geos[];
 \brief A NULL terminated list of the known geometries
									  */
/* ---------------------------------------------------------------------- */
const struct _TFC_geometry *TFC_Geos[] = { &TFC_GeometryStd,
                                           &TFC_Geo_v1r13p0,
                                            0 };
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn const struct _TFC_geometry *TFC_geosLocate (
                                    const struct _TFC_geometry *geos[], 
                                    int                          ngeos,
		                    int                             id)
  \brief  Locates the geometry with the specified \a id
  \return A pointer to the specified geometry or NULL if it is not in the
          list.

  \param  geos   The begining of the list of geometries to search
  \param  ngeos  The number of geometries to search. If specified
                 as < 0, the entire list, until the terminating NULL
                 will be searched.
  \param  id     The id of the geometry to locate. If this is specified
                 as 0, the current default geometry will be located.
									  */
/* ---------------------------------------------------------------------- */
const struct _TFC_geometry *TFC_geosLocate (const struct _TFC_geometry *geos[], 
			   	            int                          ngeos,
					    int                             id)
{
  unsigned int cnt = ngeos;

  /* Is the request for the default geometry? */
  if (id == TFC_K_GEO_ID_DEFAULT) id = TFC_K_GEO_ID_CURRENT;

  while (--cnt >= 0)
  {
      const struct _TFC_geometry *geo = *geos++;
      if ( (geo == 0) || (geo->tag.id == id) ) return geo;
  }

  return (struct _TFC_geometry *)0;
}
/* ---------------------------------------------------------------------- */




