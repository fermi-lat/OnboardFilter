#ifndef FILTER_RTO_H
#define FILTER_RTO_H


/* ---------------------------------------------------------------------- *//*!
   
   \file   filter_rto.h
   \brief  Utility routines to parse the command line parameters and 
           fill in the run time filter options, interface.
   \author JJRussell - russell@slac.stanford.edu
   
\verbatim

  CVS $Id
\endverbatim
    
   \par
    There is really no good reason why this code is not included directly
    in the filter.c code other than it was getting unwieldy large.
                                                                          */
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *\
 * 
 * HISTORY
 * -------
 *
 * DATE     WHO WHAT
 * -------- --- ---------------------------------------------------------
 * 03.13.05 jjr Changed name of EFC_RESULT_ENERGY to FLT_RESULT_ENERGY.
 *              This definition is private to the \e filter program
 * 03.10.05 jjr Renamed from EFC_rto.h on the principle that this is not
 *              an EFC system file.
 * 12.22.04 jjr Cloned from DFC/src/DFC_rto.h
 *
\* ---------------------------------------------------------------------- */




#include "EFC/EDM.h"
#include "EFC/TFC_geoIds.h"
#include "EDS/io/EBF_stream.h"


/* --------------------------------------------------------------------- *//*!

  \enum   _FLT_RESULT_ENERGY_K
  \brief   Indicates which energy is used to classified the event
									 *//*!
  \typedef FLT_RESULT_ENERGY_K
  \brief   Typedef for struct _FLT_RESULT_ENERGY_K

  \par
   The result vector records the energy of the event for classification
   purposes. This value determines the source of that energy. The first
   is just the energy as calculated from the CAL data. The last two
   are based on the Monte Carlo \e truth values. Naturally these last
   two are available only if these vaues are included in the input data
   sample. For example, they are not available in data sourced from the
   FES
									 */
/* --------------------------------------------------------------------- */
typedef enum _FLT_RESULT_ENERGY_K
{
  FLT_RESULT_ENERGY_K_CALCULATED = 0, 
  /*!< Use energy calculated from the CAL data                           */

  FLT_RESULT_ENERGY_K_GENERATED  = 1,
  /*!< Use the GLEAM Monte Carlo generated energy                        */

  FLT_RESULT_ENERGY_K_OBSERVED   = 2,
  /*!< Use the GLEAM Monte Carlo observed energy                         */
}
FLT_RESULT_ENERGY_K;
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \enum   _FLT_STATS_EBIN_V
  \brief   Determines how the energy binning of the statistics display,
           as bit offsets
									 *//*!

  \typedef FLT_STATS_EBIN_V
  \brief   Typedef for enum _FLT_STATS_EBIN_V 
									 */
/* --------------------------------------------------------------------- */
typedef enum _FLT_STATS_EBIN_V
{
  FLT_STATS_EBIN_V_DEF = 0, /*!< Use the default energy binning          */
  FLT_STATS_EBIN_V_ALT = 1, /*!< Use the alternate energy binning        */
}
FLT_STATS_EBIN_V;
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \enum   _FLT_STATS_EBIN_M
  \brief   Determines how the energy binning of the statistics display 
           as in-place bit masks.
									 *//*!

  \typedef FLT_STATS_EBIN_M
  \brief   Typedef for enum _FLT_STATS_EBIN_M
									 */
/* --------------------------------------------------------------------- */
typedef enum _FLT_STATS_EBIN_M
{
  FLT_STATS_EBIN_M_DEF = (1 << FLT_STATS_EBIN_V_DEF),
  /*!< Use the default energy binning                                    */

  FLT_STATS_EBIN_M_ALT = (1 << FLT_STATS_EBIN_V_ALT),
  /*!< Use the alternate energy binning                                  */
}
FLT_STATS_EBIN_M;
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \def    FLT_M_PRJS
  \brief  Print mask to request projection dump
									 *//*!
  \def    FLT_M_GEO
  \brief  Print mask to request geometry dum;
									 */
/* --------------------------------------------------------------------- */
#define   FLT_M_PRJS (1<<31)
#define   FLT_M_GEO  (1<<30)
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- */
#if EDM_USE
/* ---------------------------------------------------------------------- *//*!

  \struct _MessageObjLevels
  \brief   Establishes the message level for various objects
									  *//*!
  \typedef MessageObjLevels
  \brief   Typedef for struct \e _MessageObjLevels
									  */
/* ---------------------------------------------------------------------- */
typedef struct _MessageObjLevels
{
  unsigned int     filter:3;   /*!< Filter message level                  */
  unsigned int efc_filter:3;   /*!< Driver routine                        */
  unsigned int    tfc_ptf:3;   /*!< Projection tower finding level        */
  unsigned int    tfc_acd:3;   /*!< TFC_acd matching message level        */
  unsigned int  tfc_skirt:3;   /*!< TFC_skirt matching message level      */
}
MessageObjLevels;
/* ---------------------------------------------------------------------- */
#endif
/* ---------------------------------------------------------------------- */



    
/* ---------------------------------------------------------------------- *//*!

  \struct _FilterRto
  \brief   Collections all the runtime options together. These are
           essentially the interpretted command line options.
                                                                          *//*!
  \typedef FilterRto
  \brief   Typedef for struct _FilterRto
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _FilterRto
{
  EBF_streamType   type; /*!< The stream type                             */
  const char      *name; /*!< The name of the input file or the address
 			      of the data section, depends on 'type'      */
  const char     *ofile; /*!< The name of the output file (optional)      */
  int        to_process; /*!< The number of events to process.            */
  int           to_skip; /*!< The number of events to initially skip.     */
  int          to_print; /*!< The number of events to print.              */
  int       ss_to_print; /*!< The mask of which subsystems to print       */
  int             quiet; /*!< Minimal summary information                 */
  int              list; /*!< Print list of events not rejected           */
  unsigned int   vetoes; /*!< Veto mask                                   */
  int            energy; /*!< Which energy to use                         */
  int          esummary; /*!< Event results summary flags                 */
  int          geometry; /*!< Print the detector geometry                 */
  TFC_geoId      geo_id; /*!< The ID of the geometry to use               */
  EDM_CODE(
  MessageObjLevels
                 levels; /*!< Diagnostic message levels                   */)
}
FilterRto;
/* ---------------------------------------------------------------------- */



#ifdef __cplusplus
extern "C" {
#endif


/* ---------------------------------------------------------------------- */

#ifndef VXWORKS
extern int          filter_rtoFill        (FilterRto *rto,
					   int       argc,
					   char   *argv[]);

extern void         filter_rtoUsageReport (void);
#endif


extern void         filter_rtoPrint    (const FilterRto *rto);

/* ---------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif
