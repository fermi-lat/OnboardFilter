#ifndef DFC_RTO_H
#define DFC_RTO_H



/* ---------------------------------------------------------------------- */
/*!
   
   \file   DFC_rto.h
   \brief  Utility routines to parse the command line parameters and 
           fill in the run time filter options, interface.
   \author JJRussell - russell@slac.stanford.edu
   
\verbatim

  CVS $Id
\endverbatim
    
    There is really no good reason why this code is not included directly
    in the filter.c code other than it was getting unwieldy large.
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/EDM.h"
#include "DFC/TFC_geoIds.h"


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
  unsigned int dfc_filter:3;   /*!< Driver routine                        */
  unsigned int    tfc_ptf:3;   /*!< Projection tower finding level        */
  unsigned int    tfc_acd:3;   /*!< TFC_acd matching message level        */
  unsigned int  tfc_skirt:3;   /*!< TFC_skirt matching message level      */
}
MessageObjLevels;
/* ---------------------------------------------------------------------- */
#endif
/* ---------------------------------------------------------------------- */



    
/* ---------------------------------------------------------------------- *//*!
  \struct _DFC_rto
  \brief   Collections all the runtime options together. These are
           essentially the interpretted command line options.
                                                                          *//*!
  \typedef DFC_rto
  \brief   Typedef for struct _DFC_rto
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _DFC_rto
{
  const char     *ifile; /*!< The name of the input file (Evt Bld  format)*/
  const char     *ofile; /*!< The name of the output file (optional)      */
  unsigned int    *data; /*!< Pointer to a EBD data structure             */
  int        to_process; /*!< The number of events to process.            */
  int           to_skip; /*!< The number of events to initially skip.     */
  int          to_print; /*!< The number of events to print.              */
  int       ss_to_print; /*!< The mask of which subsystems to print       */
  int             quiet; /*!< Minimal summary information                 */
  int              list; /*!< Print list of events not rejected           */
  unsigned int   vetoes; /*!< Veto mask                                   */
  int          esummary; /*!< Event results summary flags                 */
  int          geometry; /*!< Print the detector geometry                 */
  TFC_geoId      geo_id; /*!< The ID of the geometry to use               */
  EDM_CODE(
  MessageObjLevels
                 levels; /*!< Diagnostic message levels                   */)
}
DFC_rto;
/* ---------------------------------------------------------------------- */


#ifdef __cplusplus
extern "C" {
#endif


#ifndef VXWORKS
extern int          DFC_rtoFill        (DFC_rto *rto, int argc, char *argv[]);
extern void         DFC_rtoUsageReport (void);
#endif


extern void         DFC_rtoPrint       (const DFC_rto *rto);


#ifdef __cplusplus
}
#endif

#endif
