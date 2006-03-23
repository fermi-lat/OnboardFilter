#ifndef FILTERALG_PROJECTIONFIND_H
#define FILTERALG_PROJECTIONFIND_H


/* ---------------------------------------------------------------------- *//*!
   
   \file   TFC_projectionFind.h
   \brief  Defines the interface for finding projections in a single tower/
   \author JJRussell - russell@slac.stanford.edu

\verbatim

    CVS $Id: FilterAlg_projectionFind.h,v 1.1 2005/09/02 19:09:28 hughes Exp $
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
 * 12.22.04 jjr Cloned DFC/DFC/TFC_projectionTowerFind.h
 *
\* ---------------------------------------------------------------------- */

/* --------------------------------------------------------------------- */

  extern int FilterAlg_projectionsSizeof   (void);

  extern int FilterAlg_projectionTowerFind (TFC_projection          *prjs,
                                      EDR_tkrTower     *towerRecord,
				      unsigned int          xLayers,
                                      unsigned int          yLayers);

/* --------------------------------------------------------------------- */


    

#endif
