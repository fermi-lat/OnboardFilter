/* ---------------------------------------------------------------------- *//*!

   \file  filter.c
   \brief Driver program to test filtering code
   \author JJRussell - russell@slac.stanford.edu

\verbatim

  CVS $Id: OnboardFilter.cxx,v 1.48 2006/03/14 03:12:52 hughes Exp $
\endverbatim

                                                                          */
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *\
 *
 * HISTORY
 * -------
 *
 * DATE     WHO WHAT
 * -------- --- ---------------------------------------------------------
 * 04.03.05 jjr Change the default configuration parameter setup to
 *              reflect the changes in the CAL configuration block.
 * 03.30.05 jjr Lowered default TWO TRACK activation cut to 5 MeV
 * 02.26.05 jjr Removed EFC_filterComplete. It cannot be supported in a
 *              streaming environment and most of the functionality has
 *              been moved somewhere else.
 * 02.09.05 jjr Corrected use of CACHE_invalidate, one must first do a
 *              flush to ensure everything is in memory before
 *              invalidating the cache.
 *
\* ---------------------------------------------------------------------- */



#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "src/filter_rto.h"

#include "EFC_DB/EFC_DB_schema.h"
#include "GFC_DB/GFC_DB_schema.h"
#include "GFC_DB/GFC_DB_instance.h"
#include "RFC_DB/RFC_DB_schema.h"
#include "RFC_DB/RFC_DB_instance.h"

//#include "EFC/EDM.h"
#include "EFC/EFC.h"
#include "EFC/EFC_edsFw.h"
//#include "EFC/GFC_resultsPrint.h"
//#include "EFC/RFC_resultsPrint.h"
//#include "EFC/EFC_display.h"

//#include "../flight/EDS/src/EDS_fwPvt.h"
//#include "EDS/EDS_fw.h"
//#include "EDS/TMR.h"
//#include "EDS/LCBV.h"
#include "EDS/io/LCBP.h"
#include "EDS/io/EBF_stream.h"
#include "EDS/io/EBF_evts.h"
#include "EDS/FFS.h"

#include "CDM/CDM_pubdefs.h"
#include "CMX/CMX_lookupPub.h"
 


/* ---------------------------------------------------------------------- *\

  CONFIGURATION

\* ---------------------------------------------------------------------- */


#include "EFC/GFC.h"
#include "EFC/GFC_resultsPrint.h"
//#include "EFC/GFC_statsPrint.h"
//#include "EFC/GFC_cfgPrint.h"
//#include "EFC/GFC_status.h"
//#include "EFC/GFC_stats.h"

//#include "../flight/EFC/src/GFC_def.h"
//#include "../flight/EFC/src/GFC_resultDef.h"

#include "EFC/RFC.h"
#include "EFC/RFC_resultsPrint.h"
//#include "EFC/RFC_statsPrint.h"
//#include "EFC/RFC_cfgPrint.h"
//#include "EFC/RFC_status.h"
//#include "EFC/RFC_stats.h"

//#include "../flight/EFC/src/RFC_def.h"
#include "../flight/EFC/src/RFC_resultDef.h"

//#include "EDS/EBF_evt.h"
//#include "EDS/EBF_pkt.h"
//#include "EDS/EBF_mc.h"
#include "EDS/EBF_dir.h"
#include "EDS/ECR_cal.h"
#include "EDS/EBF_cid.h"
#include "EDS/EBF_gem.h"
#include "EDS/EBF_tkr.h"
#include "EDS/EDR_cal.h"
#include "EDS/EBF_cal.h"
#include "EDS/EDR_tkrUnpack.h"
#include "EDS/EDR_calUnpack.h"
#include "EDS/EDR_calPrint.h"

#if       defined (EFC_DFILTER)
#include "EFC/EFC_display.h"
#include "GEO_DB/GEO_DB_print.h"
#include "EFC/TFC_projectionPrint.h"
#endif

#if       defined (EFC_IFILTER)
#include "GEO_DB/GEO_DB_print.h"
#endif

 
#include "GaudiKernel/Algorithm.h"

#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Property.h"
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

#include "Event/TopLevel/EventModel.h"
#include "Event/MonteCarlo/McParticle.h"
#include "EbfWriter/Ebf.h"
#include "OnboardFilter/FilterStatus.h"
#include "OnboardFilter/OnboardFilterTDS.h"
#include "facilities/Util.h"

//#include "CDM_prvdefs.h"


/* ====================================================================== */
/* Forward References                                                     */
/* ---------------------------------------------------------------------- */
#ifndef CMX_DOXYGEN

#endif
/* ====================================================================== */




/* ====================================================================== */
/* Local Data Structures                                                  */
/* ---------------------------------------------------------------------- *//*!

  \struct _FilterResultCtx
  \brief   The filter result context, will replace FilterResultCtl
                                                                          *//*!
  \typedef FilterResultCtx
  \brief   Typedef for struct _FilterResultCtx
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _FilterResultCtx
{
  TMR_tick         beg;  /*!< Beginning time     */
  TMR_tick         end;  /*!< Ending time        */
  unsigned int options;  /*!< Print options      */
  int             size;  /*!< Size of one vector */
  unsigned int  vetoes;  /*!< The active vetoes  */
  unsigned int   quiet;  /*!< Minimum output     */
  void       *stats[2];  /*!< Statistics buffer  */
}
FilterResultCtx;
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \typedef int (*OutputClose)(void *prm)
  \brief   Close an output stream
  \return  Status

  \param   prm  The user context parameter
                                                                          */
/* ---------------------------------------------------------------------- */
typedef int (*OutputClose)(void *prm);
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \struct _OutputHandler
  \brief   Binds an output routine with its parameter
                                                                          *//*!
  \typedef OutputHandler
  \brief   Typedef for struct _OutputHandler
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _OutputHandler
{
  void                *prm; /*!< The output parameter                     */
  EDS_fwPostWriteRtn write; /*!< The write routine, must be non-NULL      */
  EDS_fwPostFlushRtn flush; /*!< The flush routien, must be non-NULL      */
  OutputClose        close; /*!< The close routine, must be non-NULL      */
}
OutputHandler;
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \struct _Stream
  \brief   Contains the posting context
                                                                          *//*!
  \typedef Stream
  \brief   Typedef for struct _Stream
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _Stream
{
  int  id;  /*!< The stream id               */
  int cnt;  /*!< The number of events output */
}
Stream;
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \struct _FilterCtx
  \brief   Structure to bind all the filtering parameters together into
           something suitable for passing to the LCB event call back
           handler.
                                                                          *//*!
  \typedef FilterCtx
  \brief   Typedef for struct _FilterCtx
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _FilterCtx
{
  const EDS_fwHandlerServices
                 *services; /*!< The filter services                      */
  void                *efc; /*!< The event filter context                 */
  int             efc_size; /*!< Size of the efc structure                */
  int           stats_size; /*!< Size of the statistics block             */
  int         results_size; /*!< Size of the result vectors               */
  OutputHandler     output; /*!< The output handler                       */
  EBF_evt             *evt; /*!< Pointer to the first packet of an event  */
  FilterResultCtx   result; /*!< The stash of result vectors              */
  int                nevts; /*!< The number of events                     */
  int                 ievt; /*!< Current event number                     */
  int             to_print; /*!< Number of events to print                */
  unsigned int ss_to_print; /*!< Bit mask of the subsystems to print      */
  int          useMcEnergy; /*!< Use the MC energy as the classifier      */
  int           listPasses; /*!< Flag to print the events that pass
                                 the filter                               */
}
FilterCtx;
/* ====================================================================== */


class FilterInfo
{
public:
   int   thrTkr;
   int   calHiLo;
    int condsumCno;
    int acd_vetoes_XZ;
    int acd_vetoes_YZ;
    int acd_vetoes_XY;
    int acd_vetoes_RU;
    int livetime;
    int trgtime;
    int ppstime;
    int discarded;
    int prescaled;
    int sent;   


};


class OnboardFilter:public Algorithm
{
public:
    OnboardFilter(const std::string& name, ISvcLocator *pSvcLocator);
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

    int eventCount;
    int eventProcessed;
    int eventBad; 
  
    /* ====================================================================== */
    /*  Internal functions                                                    */
    /* ---------------------------------------------------------------------- */

    #ifdef EDM_USE
    static void   setMessageLevels (MessageObjLevels                 lvl);
    EDM_level Filter_edm = EDM_K_FATAL;
    #endif

    static void printEnergy (int energy);

    static void *allocate (int                     nbytes,
                           const char               *name);

    static void  free_em  (void                      *ptr);

    //static int   createOutput      (OutputHandler           *output,
    //                                const char                *name);
    //static __inline int getMCsequence  (const EBF_evt *evt);
    int rtoFill (FilterRto *rto);
    static int loadLib (const char *library_name, int verbose, int libs);

    static void extractFilterInfo (Stream *stream, EDS_fwIxb *ixb);
    static int myOutputFlush (Stream *stream, int reason);
    static int passThrough (void        *unused,
                            unsigned int nbytes,
                            EBF_pkt        *pkt,
                            EBF_siv         siv,
                            EDS_fwIxb       ixb);

    int getMCsequence (const EBF_evt *evt);
    void storeHits(OnboardFilterTds::TowerHits *hits);

    /* ====================================================================== */

    /* ====================================================================== */

    static FilterInfo myFilterInfo;
    int m_mask;  //mask for setting filter to reject
    int m_rejected;
    int m_passThrough;
    // Path to shareables
    std::string m_FileNamePath;
    // File name for peds/gains
    std::string m_FileName_Filter;
    std::string m_FileName_Pedestals;
    std::string m_FileName_Gains;

    int m_vetoBits[17];      //array to count # of times each veto bit was set
    int m_statusBits[15];      //array to count # of times each veto bit was set
    FilterRto m_rto;
    EDS_fw                   *edsFw;
    LCBP                        lcb;

    EFC                        *efc;
    const EFC_DB_Schema     *schema;
    EDS_DB_handlerServicesGet                   get;
    const EDS_DB_HandlerConstructServices *services;
    FilterCtx                   ctx;

    EFC                        *efcRFC;
    const EFC_DB_Schema     *schemaRFC;
    EDS_DB_handlerServicesGet                   getRFC;
    const EDS_DB_HandlerConstructServices *servicesRFC;
    FilterCtx                   ctxRFC;

    /* ====================================================================== */
    /* Configurations                                                         */
    /* ---------------------------------------------------------------------- *//*!

    \var    EFC_definition Gfc_Definitions
    \brief  The defining parameters for this filter

    \par
    There are two sets of definitions, one for doing the reporting using
    the normal energies, the other for doing the reporting using the
    MC energies
                                                                          */
    /* ---------------------------------------------------------------------- */
    //static const  EFC_definition Gfc_Definitions[2];
    /* ---------------------------------------------------------------------- */
    static   EFC_definition Gfc_Definitions[2];
};



static const AlgFactory<OnboardFilter> Factory;
const IAlgFactory& OnboardFilterFactory = Factory;
FilterInfo OnboardFilter::myFilterInfo;

OnboardFilter::OnboardFilter(const std::string& name, ISvcLocator *pSvcLocator):Algorithm(name,pSvcLocator),m_rejected(0){
  declareProperty("FileNameFilter"  ,m_FileName_Filter="$(ONBOARDFILTERROOT)/$(BINDIR)/libOnboardFilter.so");
//  declareProperty("FileNamePeds"  ,m_FileName_Pedestals="$(ONBOARDFILTERROOT)/$(BINDIR)/libcal_db_pedestals.so");
//  declareProperty("FileNameGains"  ,m_FileName_Gains="$(ONBOARDFILTERROOT)/$(BINDIR)/libcal_db_gains.so");
  declareProperty("FileNamePath",  m_FileNamePath="$(FLIGHTCODELIBS)");
  declareProperty("FileNamePeds",  m_FileName_Pedestals="cal_db_pedestals");
  declareProperty("FileNameGains", m_FileName_Gains="cal_db_gains");
  declareProperty("mask",m_mask=0);
  declareProperty("PassThrough",m_passThrough=1);
//  GemInfo myGemInfo = GemInfo();
}

#define CACHE_cleanData(_ptr, _nbytes) /* NOOP */


/* ---------------------------------------------------------------------- *//*!

   \fn          int getMCsequence (const EBF_evt *evt)
   \brief       Retrieves the GLEAM Monte Carlo Event Sequence Number
   \param   evt Pointer to the event
   \return      The GLEAM Monte Carlo Event Sequence Number.
                                                                          */
/* ---------------------------------------------------------------------- */
int OnboardFilter::getMCsequence (const EBF_evt *evt)
{
    return evt->hdr.undef[0];
}
/* ---------------------------------------------------------------------- */
    



/* ---------------------------------------------------------------------- *//*!

  \def   CACHE_cleanData(_ptr, _nbytes)
  \brief Macro to move the data from cache to memory and then invalidate
         the cache. This is a NOP all but VxWorks platforms
                                                                          */
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- */


int OnboardFilter::rtoFill (FilterRto *rto)
{

    rto->type       = EBF_STREAM_TYPE_K_DATA;
//    rto->name       = name;
    rto->to_process = 1;
    rto->to_skip    = 0;
    rto->to_print   = 0;
    rto->quiet      = 0;
    rto->list       = 0;
    rto->esummary   = GFC_RESULTS_PRINT_OPT_M_SUMMARY
                    | (FLT_STATS_EBIN_M_DEF << 16);
    rto->vetoes     = 0;

#   if 0
    taskSpawn ("filter",    /* Task name     */
               100,         /* Task priority */
               0,           /* Task options  */
               1000000,     /* Stack size    */
               doFilter,    /* Entry point   */
               (int)rto,    /* Arg 1         */
               0,           /* Arg 2         */
               0,           /* Arg 3         */
               0,           /* Arg 4         */
               0,           /* Arg 5         */
               0,           /* Arg 6         */
               0,           /* Arg 7         */
               0,           /* Arg 8         */
               0,           /* Arg 9         */
               0);          /* Arg 10        */
#   endif

    return 0;

}

/* ---------------------------------------------------------------------- *//*!

  \fn     static int loadLib (const char *library_name, int verbose)
  \brief  Dynamically loads the specified configuration library
  \retval 0, success
  \retval -1, failure

  \param  library_name The name of the library to load
  \param  verbose      The verbosity level
                                                                          */
/* ---------------------------------------------------------------------- */
int OnboardFilter::loadLib (const char *library_name, int verbose, int libs)
{
   CDM_Database *cal_db;
   if (verbose) printf (" Loading: %s", library_name);

   cal_db = CDM_loadDatabase (library_name, 0);
   if (verbose) printf (cal_db == NULL ? " (FAILED)\n\n" : " (succeeded)\n\n");
   return cal_db == NULL ? -1 : 0;
}


StatusCode OnboardFilter::initialize()
{
    Stream ostreams[2];
    eventCount = 0;
    eventProcessed = 0;
    eventBad = 0;
//   MsgStream log(msgSvc(),name());
    setProperties();
    printf("Initializing Filter Settings\n");
    int      status; 
    status = rtoFill (&m_rto); 

    /* Initialize the time base */
    TMR_initialize  ();
//   loadLib ("$(ONBOARDFILTER)/$(BINDIR)/libOnboardFilter.so",1,0);
//   loadLib ("libcal_db_gains.so",1,0);
//   loadLib ("libcal_db_pedestals.so",1,0);
////   loadLib (m_FileName_Filter.c_str(),1,0);
    int ret = facilities::Util::expandEnvVar(&m_FileNamePath);

#ifdef WIN32
    std::string fType = ".dll";
    std::string delim = "/";
#else
    std::string fType = ".so";
    std::string delim = "/lib";
#endif

    std::string calPedFile = m_FileNamePath + delim + m_FileName_Pedestals + fType;
    loadLib (calPedFile.c_str(),1,0);
    
    std::string calGainFile = m_FileNamePath + delim + m_FileName_Gains + fType;
    loadLib (calGainFile.c_str(),1,0);
    
    std::string gfc_db_File = m_FileNamePath + delim +  "gfc_db_master" + fType;
    loadLib (gfc_db_File.c_str(),1,0);
    
    gfc_db_File = m_FileNamePath + delim + "gfc_db_default" + fType;
    loadLib (gfc_db_File.c_str(),1,0);
    
    gfc_db_File = m_FileNamePath + delim + "gfc_db_normal" + fType;
    loadLib (gfc_db_File.c_str(),1,0);

    std::string rfc_db_File = m_FileNamePath + delim + "rfc_db_master" + fType;
    loadLib (rfc_db_File.c_str(),1,0);

    rfc_db_File = m_FileNamePath + delim + "rfc_db_default" + fType;
    loadLib (rfc_db_File.c_str(),1,0);

    rfc_db_File = m_FileNamePath + delim + "rfc_db_normal" + fType;
    loadLib (rfc_db_File.c_str(),1,0);

    std::string eds_db_File = m_FileNamePath + delim + "eds_db" + fType;
    loadLib (eds_db_File.c_str(),1,0);

    std::string geo_db_File = m_FileNamePath + delim + "geo_db" + fType;
    loadLib (geo_db_File.c_str(),1,0);

    std::string ggf_db_File = m_FileNamePath + delim + "ggf_db" + fType;
    loadLib (ggf_db_File.c_str(),1,0);

   /* Print information about the runtime options */
//   filter_rtoPrint (&m_rto);


   /* Set the diagnostic message/print levels */
   EDM_CODE (setMessageLevels (m_rto.levels));


   /* Save the number of events, stash the result info */
   ctx.nevts          = 1;
   ctx.ievt           = m_rto.to_skip;
   ctx.to_print       = m_rto.to_print;
   ctx.ss_to_print    = m_rto.ss_to_print;
   ctx.listPasses     = m_rto.list;
   ctx.useMcEnergy    = m_rto.energy;
   ctx.result.vetoes  = m_rto.vetoes;
   ctx.result.vetoes  = 0;
   ctx.result.options = m_rto.esummary;
   ctx.result.quiet   = m_rto.quiet;


   /* Save the number of events, stash the result info */
   ctxRFC.nevts          = 1;
   ctxRFC.ievt           = m_rto.to_skip;
   ctxRFC.to_print       = m_rto.to_print;
   ctxRFC.ss_to_print    = m_rto.ss_to_print;
   ctxRFC.listPasses     = m_rto.list;
   ctxRFC.useMcEnergy    = m_rto.energy;
   ctxRFC.result.vetoes  = m_rto.vetoes;
   ctxRFC.result.vetoes  = 0;
   ctxRFC.result.options = m_rto.esummary;
   ctxRFC.result.quiet   = m_rto.quiet;

   /* Allocate and initialize the framework */
   lcb   = LCBP_get    ();
   edsFw = (EDS_fw*) allocate (EDS_fwSizeof (), "EDS framework");
   EDS_fwConstruct (edsFw, NULL, NULL, NULL);
   EDS_fwFreeSet   (edsFw, (LCBV_pktsRngFreeCb)LCBP_pktsRngFree, lcb);
   //    EDS_fwPostSet   (edsFw,
   //                     EDS_FW_K_POST_0,
   //                     ctx.output.write,
   //                     ctx.output.flush,
   //                     ctx.output.prm);

   ostreams[0].id  = 10;
   ostreams[0].cnt = 0;

//   EDS_fwPostSet   (edsFw,
//               EDS_FW_K_POST_0,
//               (EDS_fwPostWriteRtn)myOutputWrite,
//               (EDS_fwPostFlushRtn)myOutputFlush,
//               &ostreams[0]);
//   ostreams[1].id  = 11;
//   ostreams[1].cnt = 0;

//   EDS_fwPostSet   (edsFw,
//               EDS_FW_K_POST_1,
//               (EDS_fwPostWriteRtn)extractFilterInfo,
//               (EDS_fwPostFlushRtn)myOutputFlush,
//               &ostreams[0]);

   //    CMX_lookupPrint();

   /* Find the filter and get it ready for action */
   schema       = EFC_lookup (GFC_DB_SCHEMA, GFC_DB_INSTANCE_K_MASTER);
   const char               *fnd = schema->eds.get;
   printf("fnd %s\n",fnd);
   get          = (const EDS_DB_HandlerConstructServices*(*)(void*)) CMX_lookupSymbol(fnd);
   services     = (EDS_DB_HandlerConstructServices *)get (0);
   efc          = (EFC *)allocate (services->sizeOf (schema, &ctx), "Efc");
   ctx.services = services->construct (efc, schema, &ctx, edsFw);
   EFC_report (efc, -1);

   /* Register and enable the filter */
   int id1 = EDS_fwRegisterS (edsFw,  0,  ctx.services, efc);

   /* Find the filter and get it ready for action */
   schemaRFC       = EFC_lookup (RFC_DB_SCHEMA, RFC_DB_INSTANCE_K_MASTER);
   const char               *fndRFC = schemaRFC->eds.get;
   printf("fndRFC %s\n",fndRFC);
   getRFC          = (const EDS_DB_HandlerConstructServices*(*)(void*)) CMX_lookupSymbol(fndRFC);
   servicesRFC     = (EDS_DB_HandlerConstructServices *)getRFC (0);
   efcRFC          = (EFC *)allocate (servicesRFC->sizeOf (schemaRFC, &ctxRFC), "Efc_rfc");
   ctxRFC.services = servicesRFC->construct (efcRFC, schemaRFC, &ctxRFC, edsFw);
   EFC_report (efcRFC, -1);

   /* Register and enable the filter */
//   int id2 = EDS_fwRegisterS (edsFw,  -1,  ctxRFC.services, efcRFC);

// Register and enable the "passThrough" handler.  This handler isonly
// used to set EDS_FW_FN_M_POST_1, which allows the "post" routine to get filter
// info from the IXB block
   int id3 = EDS_fwRegister (edsFw,
             -1,
             EDS_FW_OBJ_M_DIR,
             EDS_FW_FN_M_DIR | EDS_FW_FN_M_POST_1,
             (EDS_fwHandlerProcess   )passThrough,
             (EDS_fwHandlerAssociate )NULL,
             (EDS_fwHandlerSelect    )NULL,
             (EDS_fwHandlerFlush     )NULL,
             (EDS_fwHandlerDestruct  )NULL,
             NULL);
//   printf("EDS_fwregisterS results id1 %d id2 %d id3 %d\n",id1,id2,id3);
   //   ostreams[0].id  = 10;
   //   ostreams[0].cnt = 0;

   //    EDS_fwPostSet   (edsFw,
   //                     EDS_FW_K_POST_0,
   //                     (EDS_fwPostWriteRtn)myOutputWrite,
   //                     (EDS_fwPostFlushRtn)myOutputFlush,
   //                     &ostreams[0]);
//   ostreams[1].id  = 11;
//   ostreams[1].cnt = 0;

   EDS_fwPostSet   (edsFw,
               EDS_FW_K_POST_1,
               (EDS_fwPostWriteRtn)extractFilterInfo,
               (EDS_fwPostFlushRtn)myOutputFlush,
               &ostreams[0]);


   EDS_fwChange(edsFw, EDS_FW_MASK(0), EDS_FW_MASK(0));
//   EDS_fwChange(edsFw, EDS_FW_MASK(id2), EDS_FW_MASK(id2));
   EDS_fwChange(edsFw, EDS_FW_MASK(id3), EDS_FW_MASK(id3));
   EDS_fwPostChange  (edsFw, EDS_FW_M_POST_1, EDS_FW_M_POST_1);
   //   EDS_fwPostChange  (edsFw, EDS_FW_M_POST_0 | EDS_FW_M_POST_1,
   //		          EDS_FW_M_POST_0 | EDS_FW_M_POST_1);



   /* Create the event stream */

   LCBP_create      (lcb);
  
  for(int counter=0;counter<17;counter++)
    m_vetoBits[counter]=0;
  for(int counter=0;counter<15;counter++)
    m_statusBits[counter]=0;
  
  
/*  if(!m_passThrough)
    m_rto.vetoes=EFC_GAMMA_STATUS_M_VETOES;

   statistics = (EFC_gammaStats  *) malloc (EFC_gammaStatsSizeof ());

   EFC_gammaStatsClear (statistics);


    efcCtl = efcCtlConstruct (m_rto.vetoes, m_rto.geo_id, m_rto.geometry);
    if (efcCtl == NULL) return -1;
    ctx.efc = efcCtl;


    ctx.ievt         = m_rto.to_skip;
    ctx.to_print     = m_rto.to_print;
    ctx.to_print = 0;
    ctx.ss_to_print  = m_rto.ss_to_print;
    ctx.listPasses   = m_rto.list;
    ctx.useMcEnergy  = m_rto.energy;
    ctx.ebo          = ebo;

    int nevts = 1;

    resultSize = EFC_gammaResultSizeof ();
    results    = (EFC_gammaResult *)allocate (resultSize * nevts,
				             "result vectors");
    ctx.result.beg   = results;
    ctx.result.end   = (EFC_gammaResult *)((char *)results + resultSize*nevts);
    ctx.result.cur   = results;    
    ctx.result.size  = resultSize;
    lcb = LCBP_get ();
    LCBP_create (lcb);
    fw = setupFw (lcb, &ctx);


     ctx.result.cur = ctx.result.beg;

    TMR_initialize  ();

  if (status){
    log<<MSG::DEBUG<<"Failed to set up default values for control structure"<<endreq;
    return StatusCode::FAILURE;
  }
  for(int counter=0;counter<17;counter++)
    m_vetoBits[counter]=0;
  for(int counter=0;counter<15;counter++)
    m_statusBits[counter]=0;

*/

//   loadLib ("libcal_db_gains.so",1,0);
//   loadLib ("libcal_db_pedestals.so",1,0);
//   loadLib ("libgeo_db_data.so",1,0);
//   loadLib ("libgfc_db_master.so",1,0);
//   loadLib ("libgfc_db_default.so",1,0);
//   loadLib ("libgfc_db_normal.so",1,0);

  return StatusCode::SUCCESS;
}
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \fn     static int passThrough (void        *unused,
                                  unsigned int nbytes,
                                  EBF_pkt        *pkt,
                                  EBF_siv         siv,
                                  EDS_fwIxb       ixb)
  \brief  Pass through filter routine

  \param  unused  Context variable (unused)
  \param  nbytes  Number of bytes in the packet
  \param  pkt     The event packet
  \param  siv     The state information vector
  \param  ixb    The EDS framework information exchange block
                                                                          */
/* ---------------------------------------------------------------------- */
int OnboardFilter::passThrough (void        *unused,
                        unsigned int nbytes,
                        EBF_pkt        *pkt,
                        EBF_siv         siv,
                        EDS_fwIxb       ixb)
{
  return EDS_FW_FN_M_NO_MORE | EDS_FW_FN_M_POST_1;
}



/* ---------------------------------------------------------------------- *//*!

  \fn     static int streamFlush (Stream *stream, int reason)
  \brief  Dummy stream flushing routine

  \param  stream The stream context handle
  \param  reason The reason the stream is being flushed
                                                                          */
/* ---------------------------------------------------------------------- */
int OnboardFilter::myOutputFlush (Stream *stream, int reason)
{
//  printf ("flush Stream Id: %8u   Cnt: %8u  Reason: %d\n",
//          stream->id, stream->cnt++, reason);
  return 0;
}
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- *//*!

   \fn         int doFilter (const FilterRto *rto)
   \brief      Common routine to do the filtering
   \param rto  The run time options or control parameters, assumed to be
               already filled in.
   \return     Status
                                                                          */
/* ---------------------------------------------------------------------- */
StatusCode OnboardFilter::execute()
//int doFilter (const EFC_rto *rto)
{
    unsigned int             status;
    EBF_stream                 *ebf;
    EBF_evts                   evts;
    int                       ievts;
    int                       isize;
    int                       nevts;



    eventCount++;
	for(int counter=0;counter<16;counter++) {
	    TDS_layers[counter]=0;
	}
    TDS_variables.tcids=0;
    TDS_variables.acd_xz=0;
    TDS_variables.acd_yz=0;
    TDS_variables.acd_xy=0;
    for(int counter=0;counter<16;counter++){
      TDS_variables.acdStatus[counter]=0;
      TDS_variables.xCnt[counter]=0;
      TDS_variables.yCnt[counter]=0;
      TDS_variables.numLogsHit = 0;
      //memset(&TDS_variables.prjs[counter],0,sizeof(TDS_variables.prjs[counter]));
      memset(&TDS_variables.prjs,0,sizeof(TDS_variables.prjs));
      for(int layerCounter=0;layerCounter<36;layerCounter++){
	TDS_variables.hits[counter].layers[layerCounter].cnt=0;
	TDS_variables.hits[counter].layers[layerCounter].beg=NULL;
      }
    }
//    printf("\nOnboardFilter: event num %d ",eventCount);
    
//
// Make the tds objects
    OnboardFilterTds::TowerHits *hits = new OnboardFilterTds::TowerHits;
    eventSvc()->registerObject("/Event/Filter/TowerHits",hits);
    OnboardFilterTds::FilterStatus *newStatus=new OnboardFilterTds::FilterStatus;
    eventSvc()->registerObject("/Event/Filter/FilterStatus",newStatus);

//    /* Create an output file by specified name iff m_rto.ofile is not NULL */
//    if (createOutput (&ctx.output, m_rto.ofile)) return errno;

//
// Check for ebf on tds
    SmartDataPtr<EbfWriterTds::Ebf> ebfData(eventSvc(),"/Event/Filter/Ebf");
    if(!ebfData){
      printf(" status %x energy %d passed? %d  ! noebf\n",0,0,0);
//      printf("   no ebf on tds, returning\n");
      return StatusCode::SUCCESS;
    }
    
    /* Open, read, close and process the input data */
    unsigned int length;
    char *data=ebfData->get(length);
    if(length==0){
      printf("Event has no EBF data. Ignoring\n");
      return StatusCode::SUCCESS;
    }

    ebf = EBF_streamOpen(EBF_STREAM_TYPE_K_DATA,data,length);
    if (ebf == NULL){
      printf("Unable to pass ebf data to the ebf reader\n");
      return StatusCode::SUCCESS;
    }


    status = EBF_streamRead   (ebf);
    evts   = EBF_streamLocate (ebf);
    isize  = EBF__evtsSize    (evts);
    ievts  = EBF_evtsCount    (evts);
    evts   = EBF_evtsSetup    (evts, m_rto.to_skip, m_rto.to_process);
    nevts  = EBF_evtsCount    (evts);

    if (ievts != 1) 
    {
        eventBad++;
        printf("OnboardFilter::execute; eventBad %d count %d\n",
            eventBad,eventCount);
    }
    eventProcessed++;
 
    /* Clean the cache where events are stored so timing is not distorted */
    CACHE_cleanData (EBF__evtsEvt (evts), EBF__evtsSize (evts));


    /* Connect to the event stream */
    LCBP_pktCbSet    (lcb, 1, (LCBV_pktCb)EDS_fwProcess, edsFw);
    LCBP_pktsConnect (lcb, evts);


    /* Start the event flow */
    ctx.result.beg = TMR_GET ();
    LCBP_pktsDeliver (lcb);


    /* Flush the output  */
    EDS_fwFlush (edsFw, EDS_FW_MASK(0), 0);
    EDS_fwPostFlush (edsFw, EDS_FW_M_POST_1, 0xee);
    EBF_streamFree (ebf);

   int passed = 0;
    if((TDS_variables.status & GFC_STATUS_M_GEM_THROTTLE) != 0)
      m_statusBits[0]++;
    if((TDS_variables.status & GFC_STATUS_M_GEM_TKR) != 0)
      m_statusBits[1]++;
    if((TDS_variables.status & GFC_STATUS_M_GEM_CALLO) != 0)
      m_statusBits[2]++;
    if((TDS_variables.status & GFC_STATUS_M_GEM_CALHI) != 0)
      m_statusBits[3]++;
    if((TDS_variables.status & GFC_STATUS_M_GEM_CNO) != 0)
      m_statusBits[4]++;
    if((TDS_variables.status & GFC_STATUS_M_ACD_TOP) != 0)
      m_statusBits[5]++;
    if((TDS_variables.status & GFC_STATUS_M_ACD_SIDE) != 0)
      m_statusBits[6]++;
    if((TDS_variables.status & GFC_STATUS_M_ACD_SIDE_FILTER) != 0)
      m_statusBits[7]++;
    if((TDS_variables.status & GFC_STATUS_M_TKR_EQ_1) != 0)
      m_statusBits[8]++;
    if((TDS_variables.status & GFC_STATUS_M_TKR_GE_2) != 0)
      m_statusBits[9]++;

    if((TDS_variables.status & GFC_STATUS_M_TKR_LT_2_ELO) != 0) {
      m_vetoBits[0]++;
      //printf("OnboardFilterV15: GFC_STATUS_M_TKR_LT_2_ELO\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_TKR_SKIRT) != 0) {
      m_vetoBits[1]++;
      //printf("OnboardFilterV16: GFC_STATUS_M_TKR_SKIRT\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_TKR_EQ_0) != 0) {
      m_vetoBits[2]++;
      //printf("OnboardFilterV17: GFC_STATUS_M_TKR_EQ_0\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_TKR_ROW2) != 0) {
      m_vetoBits[3]++;
      //printf("OnboardFilterV18: GFC_STATUS_M_TKR_ROW2\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_TKR_ROW01) != 0) {
      m_vetoBits[4]++;
      //printf("OnboardFilterV19: GFC_STATUS_M_TKR_ROW01\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_TKR_TOP) != 0) {
      m_vetoBits[5]++;
      //printf("OnboardFilterV20: GFC_STATUS_M_TKR_TOP\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_ZBOTTOM) != 0) {
      m_vetoBits[6]++;
      //printf("OnboardFilterV21: GFC_STATUS_M_ZBOTTOM\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_EL0_ETOT_90) != 0) {
      m_vetoBits[7]++;
      //printf("OnboardFilterV22: GFC_STATUS_M_EL0_ETOT_90\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_EL0_ETOT_01) != 0) {
      m_vetoBits[8]++;
      //printf("OnboardFilterV23: GFC_STATUS_M_EL0_ETOT_01\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_SIDE) != 0) {
      m_vetoBits[9]++;
      //printf("OnboardFilterV24: GFC_STATUS_M_SIDE\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_TOP) != 0) {
      m_vetoBits[10]++;
      //printf("OnboardFilterV25: GFC_STATUS_M_TOP\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_SPLASH_1) != 0) {
      m_vetoBits[11]++;
      //printf("OnboardFilterV26: GFC_STATUS_M_SPLASH_1\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_E350_FILTER_TILE) != 0) {
      m_vetoBits[12]++;
      //printf("OnboardFilterV27: GFC_STATUS_M_E350_FILTER_TILE\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_E0_TILE) != 0) {
      m_vetoBits[13]++;
      //printf("OnboardFilterV28: GFC_STATUS_M_E0_TILE\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_SPLASH_0) != 0) {
      m_vetoBits[14]++;
      //printf("OnboardFilterV29: GFC_STATUS_M_SPLASH_0\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_NOCALLO_FILTER_TILE) != 0) {
      m_vetoBits[15]++;
      //printf("OnboardFilterV30: GFC_STATUS_M_NOCALLO_FILTER_TILE\n");
    }
    if((TDS_variables.status & GFC_STATUS_M_VETOED) != 0) {
      m_vetoBits[16]++;
      //printf("OnboardFilterV: GFC_STATUS_M_VETOED\n");
    } else {
      //printf("OnboardFilterV: NOT VETOED!\n");
      passed = 1;
    }

//    //printf("Filter status %x energy %x\n",   
//         TDS_variables.status,TDS_variables.stageEnergy);
    //printf(" status %x energy %d passed? %d\n",TDS_variables.status,TDS_variables.stageEnergy ,passed);
    newStatus->set(TDS_variables.status);
    newStatus->setEbfSize(length);
    newStatus->setStageEnergy(TDS_variables.stageEnergy);
    newStatus->setGemThrTkr(myFilterInfo.thrTkr);
    newStatus->setGemCalHiLo(myFilterInfo.calHiLo);
    newStatus->setGemCondsumCno(myFilterInfo.condsumCno);
    newStatus->setGemAcd_vetoes_XZ(myFilterInfo.acd_vetoes_XZ);
    newStatus->setGemAcd_vetoes_YZ(myFilterInfo.acd_vetoes_YZ);
    newStatus->setGemAcd_vetoes_XY(myFilterInfo.acd_vetoes_XY);
    newStatus->setGemAcd_vetoes_RU(myFilterInfo.acd_vetoes_RU);
    newStatus->setGemLivetime(myFilterInfo.livetime);
    newStatus->setGemTrgtime(myFilterInfo.trgtime);
    newStatus->setGemPpstime(myFilterInfo.ppstime);
    newStatus->setGemDiscarded(myFilterInfo.discarded);
    newStatus->setGemPrescaled(myFilterInfo.prescaled);
    newStatus->setGemSent(myFilterInfo.sent);
    newStatus->setTcids(TDS_variables.tcids);
    newStatus->setAcdMap(TDS_variables.acd_xz,TDS_variables.acd_yz,TDS_variables.acd_xy);
    newStatus->setLayerEnergy(TDS_variables.layerEnergy);
    newStatus->setCapture(TDS_variables.xcapture,TDS_variables.ycapture);
    
    newStatus->setXY(TDS_variables.xy00, TDS_variables.xy11, TDS_variables.xy22, TDS_variables.xy33);
    newStatus->setProjections(TDS_variables.prjs);
    newStatus->setTkr(TDS_variables.tkr);
    newStatus->setLayers(TDS_layers);
    newStatus->setTmsk(TDS_variables.tmsk);
    newStatus->setLogData(TDS_variables.numLogsHit,TDS_variables.logData);
    
    storeHits(hits);


    if(m_mask!=0 && (m_mask & (TDS_variables.status >> 15)) !=0){
      this->setFilterPassed(false);
      m_rejected++;
    }

    return StatusCode::SUCCESS;
}
/* ---------------------------------------------------------------------- */

void OnboardFilter::storeHits(OnboardFilterTds::TowerHits *hits){

  struct OnboardFilterTds::TowerHits::towerRecord hitRecords[16];
  for(int counter=0;counter<16;counter++){
    hitRecords[counter].lcnt[0]=TDS_variables.hits[counter].lcnt[0];
    hitRecords[counter].lcnt[1]=TDS_variables.hits[counter].lcnt[1];
    hitRecords[counter].layerMaps[0]=TDS_variables.hits[counter].layerMaps[0];
    hitRecords[counter].layerMaps[1]=TDS_variables.hits[counter].layerMaps[1];
//    printf("counter %x lmap0 %x laymap1 %x\n",counter,
//         hitRecords[counter].layerMaps[0],hitRecords[counter].layerMaps[1]);
    for(int layerCounter=0;layerCounter<36;layerCounter++){
      hitRecords[counter].cnt[layerCounter]=TDS_variables.hits[counter].layers[layerCounter].cnt;
      hitRecords[counter].beg[layerCounter]=new short int[hitRecords[counter].cnt[layerCounter]];
      if(hitRecords[counter].cnt[layerCounter]>0){
	memcpy(hitRecords[counter].beg[layerCounter],TDS_variables.hits[counter].layers[layerCounter].beg,hitRecords[counter].cnt[layerCounter]*sizeof(short int));
      }
    }
  }
  hits->set(hitRecords);
  for(int counter=0;counter<16;counter++){
    for(int layerCounter=0;layerCounter<36;layerCounter++){
      if(hitRecords[counter].beg[layerCounter]){
	delete[] hitRecords[counter].beg[layerCounter];
	if(TDS_variables.hits[counter].layers[layerCounter].beg!=NULL){
	  free(TDS_variables.hits[counter].layers[layerCounter].beg);
	}
      }
    }
  }

}




/* ---------------------------------------------------------------------- *//*!

  \fn     void *allocate (int nbytes, const char *name)
  \brief  Allocates the number of bytes, prints an error message if
          unsuccessful.
  \return Pointer to the allocated memory or NULL if unsuccessful.

  \param  nbytes  The number of bytes to allocate
  \param    name  The name of the object being allocated
                                                                          */
/* ---------------------------------------------------------------------- */
void *OnboardFilter::allocate (int nbytes, const char *name)
{
   void *ptr = malloc (nbytes);

   if (ptr == NULL) printf ("Cannot allocate %8.8x(%d) bytes for %s\n",
                             nbytes,
                             nbytes,
                             name);

   return ptr;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn     void free_em (void *ptr)
  \brief  Jacketing routine to the free function

  \param ptr  The pointer to free, if NULL, do nothing
                                                                          */
/* ---------------------------------------------------------------------- */
void OnboardFilter::free_em (void *ptr)
{
   if (ptr) free (ptr);
   return;
}
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- */
#if EDM_USE
/* ---------------------------------------------------------------------- *//*!

  \fn     static void setMessageLevels (MessageObjLevels lvl)
  \brief  Sets the diagnostic message level for each object

  \param  lvl  The message levels for each object.

                                                                          */
/* ---------------------------------------------------------------------- */
void OnboardFilter::setMessageLevels (MessageObjLevels lvl)
{
  extern EDM_level              GFC_Filter_edm;
  extern EDM_level                 TFC_Acd_edm;
  extern EDM_level               TFC_Skirt_edm;
  extern EDM_level TFC_ProjectionTowerFind_edm;

  Filter_edm                  = lvl.filter;
  GFC_Filter_edm              = lvl.efc_filter;
  TFC_ProjectionTowerFind_edm = lvl.tfc_ptf;
  TFC_Acd_edm                 = lvl.tfc_acd;
  TFC_Skirt_edm               = lvl.tfc_skirt;

  return;
}
/* ---------------------------------------------------------------------- */
#endif
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \fn  void extractFilterInfo(Stream *ostream, EDS_fwIxb *ixb)
                            
  \brief         Extracts info from the onboard filter algorithm. 
  \param ostream dummy var
  \param     ixb The information exchange block.

   This routine extracts both the results and the some of the intermediate
   info used by the filter in deciding if an event should be vetoed or not.
                                                                          */
/* ---------------------------------------------------------------------- */
void  OnboardFilter::extractFilterInfo(Stream *ostream, EDS_fwIxb *ixb)
{
   const EBF_dirCtbDsc *contributors;
   int                          cids;
   int                           cid;
   int                         tids;
   int                         tcids;
//   int                        status;
   EDR_tkr                   *tkr;
   EDR_tkrTower          *ttrs;
   const EDR_calTower   *ctr;
   const EBF_calLogData_cf     *logs;
   const EBF_calLogData_cf      *log;
   const EBF_calLogData_bf      *log_bf;
   const int               *energy;
            const ECR_calLogEnd *rN;
            const ECR_calLogEnd *rP;
//
// Get a pointer to the directory
   EDS_fwEvt *evt =  &ixb->blk.evt;
   const EBF_dir *dir = evt->dir;
   const  ECR_cal    *constants = evt->calCal;
   EDR_cal *cal = evt->cal;
   EDR_calUnpack (cal, dir, evt->calCal);
   int twrMap = EDR_CAL_TWRMAP_JUSTIFY (cal->twrMap);
   //printf("twrMap %x\n",twrMap);
   TDS_variables.numLogsHit = 0;
   for (int tower=0; tower<16; tower++) {
      if ((twrMap    & FFS_mask (tower)) > 0) {
         ctr      = &cal->twrs[tower];
         int layerMap =  EDR_CAL_TOWER_LAYERMAP_JUSTIFY (ctr->layerMap);
         energy       = ctr->logEnergies;
         logs         = (EBF_calLogData_cf *)ctr->data + 1;
         log          = logs;
         log_bf         = (EBF_calLogData_bf *)ctr->data + 1;
         int four_range   = ctr->flags & 1;
         int counts = ctr->layerCnts;

         const  ECR_calTower       *calTower = constants->twrs + tower;
         int nlogs = 0;
         //printf("OBF: tower %d counts %x\n",tower,counts);
         for (int layer=0; layer<8; layer++) 
         {
           nlogs = (counts & 0xf);
           //printf("OBF: layer %d counts %x nlogs %d\n",layer,counts,nlogs);
           if (nlogs > 0) {
            const EDR_calColumnMap *colMapP = ctr->colMap;
            int logMap =  EDR_CAL_TOWER_COLMAP_JUSTIFY (colMapP[layer]);
            //printf("OBF: Tower %d layer %d logs %x counts %d\n",tower,layer,logMap,nlogs);
            int colMap = logMap;
            for (int ibit=0; ibit<12; ibit++, colMap <<= 1) {
               if (colMap<0) {
                  int  eA = energy[0];
                  int  eB = energy[1];
//                printf("log %d ea %d eb %d",ibit,eA,eB);  
//                  printEnergy(eA);
//                  printEnergy(eB);
                  int phaN  = log->phaN;
                  int phaP  = log->phaP;
                  int logNum = layer * EDA_CAL_K_LOGS_PER_LAYER + ibit;
                  int valN = log_bf->valN;
                  int valP = log_bf->valP;
                  int rngN = log_bf->rngN;
                  int rngP = log_bf->rngP;                  
                  //printf("tow lay log lognum %d %d %d %d  numhit %d\n",tower,layer,ibit,logNum,TDS_variables.numLogsHit);
                  rN   = &calTower->ranges[rngN].logs[logNum].n;
                  rP   = &calTower->ranges[rngP].logs[logNum].p;
//            rP   = &calTower->ranges[rngP].logs[logNum].p;

               //printf(" +p,v,r %x %x %x -p,v,r %x %x %x \n",phaP,valP,rngP,phaN,valN,rngN);
//            printf("   -ped shift gain %d %d %d\n",rN->bf.pedestal, rN->bf.shift,rN->bf.gain);
//            printf("   +ped shift gain %d %d %d\n",rP->bf.pedestal, rP->bf.shift,rP->bf.gain);
                  int ilayer;
                  if (layer < 4) {
                     ilayer = layer*2;
                  } else {
                     ilayer = layer*2 - 7;
                  }
                  TDS_variables.logData[TDS_variables.numLogsHit].tower = tower;
                  TDS_variables.logData[TDS_variables.numLogsHit].layer = ilayer;
                  TDS_variables.logData[TDS_variables.numLogsHit].column = ibit;
                  TDS_variables.logData[TDS_variables.numLogsHit].valN = valN;
                  TDS_variables.logData[TDS_variables.numLogsHit].rangeN = rngN;
                  TDS_variables.logData[TDS_variables.numLogsHit].eN = eB;
                  TDS_variables.logData[TDS_variables.numLogsHit].pedN = rN->bf.pedestal;
                  TDS_variables.logData[TDS_variables.numLogsHit].gainN = rN->bf.gain;
                  TDS_variables.logData[TDS_variables.numLogsHit].shiftN = rN->bf.shift;
                  TDS_variables.logData[TDS_variables.numLogsHit].valP = valP;
                  TDS_variables.logData[TDS_variables.numLogsHit].rangeP = rngP;
                  TDS_variables.logData[TDS_variables.numLogsHit].eP = eA;
                  TDS_variables.logData[TDS_variables.numLogsHit].pedP = rP->bf.pedestal;
                  TDS_variables.logData[TDS_variables.numLogsHit].gainP = rP->bf.gain;
                  TDS_variables.logData[TDS_variables.numLogsHit].shiftP = rP->bf.shift;
                  TDS_variables.numLogsHit++;
                  energy  += 2;
                  log  += 1;
                  log_bf  += 1;
               }
             }
            }
            counts = counts>>4;
         }
      }
   }

//
// Display event info for debugging
//   EFC_display(evt,0xffffffff);
//
// Get the result vector information
   TDS_variables.status = ixb->statusGamma;
   TDS_variables.stageEnergy = ixb->stageEnergyGamma;
//
//
   tcids        = 0;
   tids         = dir->redux.ctids << 16;
   cids         = EBF_DIR_TEMS_TKR (dir->redux.ctids);
   contributors = dir->ctbs;
//   tkr = ixb->blk.evt.tkr;

   unsigned int          twrMsk = 0xffffffff;
//   TDS_variables.tmsk=EBF_DIR_TEMS_TKR (dir->redux.ctids);
//
// Get the layer energies
   memcpy(TDS_variables.layerEnergy,ixb->blk.evt.cal->layerEnergies,8*sizeof(int));
//
// Get the projections 
   const TFC_projections *prjs;
   prjs = (const TFC_projections *)ixb->blk.ptrs[EFC_EDS_FW_OBJ_K_PRJS];
   if (twrMsk == -1) twrMsk  = prjs->twrMsk << 16;
   else              twrMsk &= 0xffff0000;
   TDS_variables.tmsk= twrMsk;

   while (twrMsk)
   {
      int towerId = FFS (twrMsk);
      const TFC_projectionDir *dir = prjs->dir + towerId;


      TDS_variables.xCnt[towerId]=dir->xCnt;
      TDS_variables.yCnt[towerId]=dir->yCnt;
//      printf("towerid %d dir->idx %d dir->xCnt %d yCnt %d\n",
//            towerId,dir->idx,dir->xCnt,dir->yCnt);
      

      twrMsk = FFS_eliminate (twrMsk, towerId);
   }



//   printf("twrMsk %x ebftwrmsk %x \n",twrMsk,EBF_DIR_TEMS_TKR (dir->redux.ctids));
   TDS_variables.tmsk= EBF_DIR_TEMS_TKR (dir->redux.ctids);
   memcpy(&(TDS_variables.prjs),prjs,sizeof(*prjs));

   twrMsk = 0xffff0000;
   tkr = ixb->blk.evt.tkr;
   EDR_tkrUnpack (tkr, dir, twrMsk);
   ttrs   = tkr->twrs;
   memcpy(&(TDS_variables.tkr),tkr,sizeof(*tkr));
//
// init
   for (int i=0; i<16; i++) {
      TDS_variables.xCnt[i]=0;
      TDS_variables.yCnt[i]=0;
   }

//
   while (twrMsk)
   {
      int towerId = FFS (twrMsk);
//      const TFC_projectionDir *dir = prjs->dir + towerId;


      EDR_tkrTower          *ttr;

      ttr   = ttrs + towerId;

      TDS_variables.hits[towerId].lcnt[0] = ttr->lexycnts[0];
      TDS_variables.hits[towerId].lcnt[1] = ttr->lexycnts[0];
      TDS_variables.hits[towerId].layerMaps[0] = ttr->layerMaps[0];
      TDS_variables.hits[towerId].layerMaps[1] = ttr->layerMaps[1];
      //printf("towerID %d xlay %x ylay %x xcnt %d ycnt %d\n",towerId,ttr->layerMaps[0],
      //         ttr->layerMaps[1],ttr->lexycnts[0],ttr->lexycnts[1]);
      int counter = towerId;
      for(int layers=0;layers<36;layers++){
         TDS_variables.hits[counter].layers[layers].cnt
               =  ttr->layers[layers].cnt;
         TDS_variables.hits[counter].layers[layers].beg
               =  (TFC_strip*)malloc(TDS_variables.hits[counter].layers[layers].cnt*sizeof(TFC_strip));
         memcpy(TDS_variables.hits[counter].layers[layers].beg,
         ttr->layers[layers].beg,
         TDS_variables.hits[counter].layers[layers].cnt*sizeof(TFC_strip));
      }

//      TDS_variables.layerEnergy[0] = ixb->blk.evt.cal->layerEnergies[0];      

      twrMsk = FFS_eliminate (twrMsk, towerId);
   }
//   }

   /* Loop over all towers that may have tracker data */
   for (cid = EBF_CID_K_TEM; cid < EBF_CID_K_TEM + 16; cid++)
   {
      TDS_variables.xy00[cid]=0;
      TDS_variables.xy11[cid]=0;
      TDS_variables.xy22[cid]=0;
      TDS_variables.xy33[cid]=0;
      TDS_variables.xcapture[cid]=0;
      TDS_variables.ycapture[cid]=0;
      TDS_variables.acd_xz=0;
      TDS_variables.acd_yz=0;
      TDS_variables.acd_xy=0;
   }
   
   /* Loop over all towers that may have tracker data */
   while (tids)
   {
      const EBF_dirCtbDsc *contributor;
      int                     xcapture;
      int                     ycapture;
      int         xy00, xy11, xy22, xy33;
      const EBF_tkr               *tkr;
      int                          cid;


/*
| Find the next tower with tracker hits and then eliminate it
| from further consideration
*/
      cid  = FFS (tids);
      tids = FFS_eliminate (tids, cid);

/* Locate the TEM contributor and its tracker data */
      contributor = &contributors[cid];
      tkr         = EBF__dirCtbTkrLocate (contributor);

       
      EBF_TKR_ACCEPTS_RIGHT_ORED_UNPACK (tkr->accepts, xcapture, ycapture);
      TDS_variables.xcapture[cid]=xcapture;
      TDS_variables.ycapture[cid]=ycapture;
      //printf("tids %x xcap %x ycap %x\n",cid,xcapture,ycapture);
      xy00 = xcapture & ycapture;
      if (xy00)
      {
               /*
                | Need to form the coincidence between the odd and even
                | portions. Want to form a variable which has layer n
                | and layer n+1 adjacent.
                |
                |             10fedcba9876543210
                |   xy      = 13579bdfh02468aceg  xy00
                |   xy >> 9 = .........13579bdfh  xy11
                |   xy << 1 = ........02468aceg_  xy22
                |   xy >> a = ..........13579bdf  xy33
                |
                |   triplet = xy00 & xy11 & xy22
                |           | xy33 & xy00 & xy11
                |
                | Note that when shifting down by 8, must eliminate the '1'
                | which appears in bit position 0.
                |
                | If one ORs the xy and xy << 1 (ignoring the bits
                | past bit 9, each bit position will contain the OR of
                | layer n-1 and layer n+1 relative to the xy >> 9
                | variable. Thus ANDing this variable| against this ORd
                | variable will produce a coincidence of layer N and
                | either layer N-1 or layer N+1, exactly what is needed.
                |
                |
         */
         xy11 = xy00 >> 9;
         xy22 = xy00 << 1;
         xy33 = xy00 >> 0xa;
         TDS_variables.xy00[cid]=xy00;
         TDS_variables.xy11[cid]=xy11;
         TDS_variables.xy22[cid]=xy22;
         TDS_variables.xy33[cid]=xy33;

         if ( xy00 & xy11  & xy22){
            tcids |= 0x80000000 >> cid;
         } else if ( xy33 & xy00  & xy11) {
            tcids |= 0x80000000 >> cid;
         } else if ((xy00 | xy22) & xy11) {
            tcids |= 0x00008000 >> cid;
         }
      }           

   }
//
// Get the GEM info
   const EBF_gem *gem;
   gem = (const EBF_gem *)dir->ctbs[EBF_CID_K_GEM].ctb->dat;

   myFilterInfo.thrTkr = gem->thrTkr;
   myFilterInfo.calHiLo = gem->calHiLo;
   myFilterInfo.condsumCno = gem->condsumCno;
   myFilterInfo.acd_vetoes_XZ = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XZ];
   myFilterInfo.acd_vetoes_YZ = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_YZ];
   myFilterInfo.acd_vetoes_XY = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XY];
   myFilterInfo.acd_vetoes_RU = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_RU];
   myFilterInfo.livetime = gem->livetime;
   myFilterInfo.trgtime = gem->trgtime;
   myFilterInfo.ppstime = gem->ppstime;
   myFilterInfo.discarded = gem->discarded;
   myFilterInfo.prescaled = gem->prescaled;


   unsigned int acd_top = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XY];
   unsigned int acd_x   = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_YZ];
   unsigned int acd_y   = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XZ];
   TDS_variables.acd_xz=acd_y;
   TDS_variables.acd_yz=acd_x;
   TDS_variables.acd_xy=acd_top;
   TDS_variables.tcids=tcids;


   return;
   
}

/* --------------------------------------------------------------------- */
#define      MEV(_energy)  (_energy/ECR_CAL_K_LEU_PER_MEV)
#define FRACTION(_energy)                                                 \
   ((_energy & (ECR_CAL_K_LEU_PER_MEV - 1)) * (100/ECR_CAL_K_LEU_PER_MEV))
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- *//*!

  \fn     static void printEnergy (int energy)
  \brief  Prints the energy in units of Mev

  \param  energy The energy to print

                                                                         */
/* --------------------------------------------------------------------- */
void OnboardFilter::printEnergy (int energy)
{
    if (energy >= 0)
    {
        printf (" %8u.%02d", MEV (energy), FRACTION(energy));
    }
    else
    {
        int      mev;
        int fraction;

        energy   = -energy;
        mev      = MEV      (energy);
        fraction = FRACTION (energy);

        if (mev) printf (" %8d.%02d", -mev, fraction);
        else     printf ("        -.%02d", fraction);
    }

    return;
}
/* --------------------------------------------------------------------- */

StatusCode OnboardFilter::finalize()
{
//    using namespace std;
   
    MsgStream log(msgSvc(),name());
//    printf("OnboardFilter::finalize: events total %d; processed %d; bad %d\n",
//         eventCount,eventProcessed,eventBad);
    log << MSG::INFO << "OnboardFilter::finalize: events total " << eventCount <<
           "; processed " << eventCount << "; bad " << eventBad << endreq;
    log << MSG::INFO << "Rejected " << m_rejected << " triggers using mask " << m_mask  << endreq;
    log << MSG::INFO;


    /* No EFC_deconstruct to call */

//    ctx.output.close (ctx.output.prm);
    free_em (edsFw);
    free_em (efc);

//   EFC_gammaStatsPrint (statistics, m_rto.vetoes);

//   free (statistics);
//    free_em ((void *)efcCtl);
 //      free_em ((void *)results);
//   free_em ((void *)fw);

//    if(log.isActive()){
/*
      printf("Status Bit                         Value\n");
      printf("EFC_GAMMA_STATUS_M_ACD             %d\n",m_statusBits[0]);       
      printf("EFC_GAMMA_STATUS_M_DIR             %d\n",m_statusBits[1]);       
      printf("EFC_GAMMA_STATUS_M_ATF             %d\n",m_statusBits[2]);      
      printf("EFC_GAMMA_STATUS_M_CAL1            %d\n",m_statusBits[3]);
      printf("EFC_GAMMA_STATUS_M_TKR             %d\n",m_statusBits[4]);
      printf("EFC_GAMMA_STATUS_M_ACD_TOP         %d\n",m_statusBits[5]);
      printf("EFC_GAMMA_STATUS_M_ACD_SIDE        %d\n",m_statusBits[6]);
      printf("EFC_GAMMA_STATUS_M_ACD_SIDE_FILTER %d\n",m_statusBits[7]);
      printf("EFC_GAMMA_STATUS_M_TKR_POSSIBLE    %d\n",m_statusBits[8]);
      printf("EFC_GAMMA_STATUS_M_TKR_TRIGGER     %d\n",m_statusBits[9]);
      printf("EFC_GAMMA_STATUS_M_CAL_LO          %d\n",m_statusBits[10]);
      printf("EFC_GAMMA_STATUS_M_CAL_HI          %d\n",m_statusBits[11]);
      printf("EFC_GAMMA_STATUS_M_TKR_EQ_1        %d\n",m_statusBits[12]);
      printf("EFC_GAMMA_STATUS_M_TKR_GE_2        %d\n",m_statusBits[13]);
      printf("EFC_GAMMA_STATUS_M_TKR_THROTTLE    %d\n",m_statusBits[14]);
    
      printf("Veto Bit Summary\n");
      printf("Trigger Name                           Count\n");
      printf("EFC_GAMMA_STATUS_M_TKR_LT_2_ELO        %d\n",m_vetoBits[0]);
      printf("EFC_GAMMA_STATUS_M_TKR_SKIRT           %d\n",m_vetoBits[1]);
      printf("EFC_GAMMA_STATUS_M_TKR_EQ_0            %d\n",m_vetoBits[2]);
      printf("EFC_GAMMA_STATUS_M_TKR_ROW2            %d\n",m_vetoBits[3]);
      printf("EFC_GAMMA_STATUS_M_TKR_ROW01           %d\n",m_vetoBits[4]);
      printf("EFC_GAMMA_STATUS_M_TKR_TOP             %d\n",m_vetoBits[5]);
      printf("EFC_GAMMA_STATUS_M_ZBOTTOM             %d\n",m_vetoBits[6]);
      printf("EFC_GAMMA_STATUS_M_EL0_ETOT_90         %d\n",m_vetoBits[7]);
      printf("EFC_GAMMA_STATUS_M_EL0_ETOT_01         %d\n",m_vetoBits[8]);
      printf("EFC_GAMMA_STATUS_M_SIDE                %d\n",m_vetoBits[9]);
      printf("EFC_GAMMA_STATUS_M_TOP                 %d\n",m_vetoBits[10]);
      printf("EFC_GAMMA_STATUS_M_SPLASH_1            %d\n",m_vetoBits[11]);
      printf("EFC_GAMMA_STATUS_M_E350_FILTER_TILE    %d\n",m_vetoBits[12]);
      printf("EFC_GAMMA_STATUS_M_E0_TILE             %d\n",m_vetoBits[13]);
      printf("EFC_GAMMA_STATUS_M_SPLASH_0            %d\n",m_vetoBits[14]);
      printf("EFC_GAMMA_STATUS_M_NOCALLO_FILTER_TILE %d\n",m_vetoBits[15]);
      printf("EFC_GAMMA_STATUS_M_VETOED              %d\n",m_vetoBits[16]);
*/
      log << MSG::INFO << "Status Bit                         Value" << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_ACD             " << m_statusBits[0] << endreq;       
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_DIR             " << m_statusBits[1] << endreq;       
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_ATF             " << m_statusBits[2] << endreq;      
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_CAL1            " << m_statusBits[3] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR             " << m_statusBits[4] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_ACD_TOP         " << m_statusBits[5] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_ACD_SIDE        " << m_statusBits[6] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_ACD_SIDE_FILTER " << m_statusBits[7] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_POSSIBLE    " << m_statusBits[8] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_TRIGGER     " << m_statusBits[9] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_CAL_LO          " << m_statusBits[10] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_CAL_HI          " << m_statusBits[11] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_EQ_1        " << m_statusBits[12] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_GE_2        " << m_statusBits[13] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_THROTTLE    " << m_statusBits[14] << endreq;
    
      log << MSG::INFO << "Veto Bit Summary" << endreq;
      log << MSG::INFO << "Trigger Name                           Count\n" << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_LT_2_ELO        " << m_vetoBits[0] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_SKIRT           " << m_vetoBits[1] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_EQ_0            " << m_vetoBits[2] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_ROW2            " << m_vetoBits[3] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_ROW01           " << m_vetoBits[4] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_TOP             " << m_vetoBits[5] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_ZBOTTOM             " << m_vetoBits[6] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_EL0_ETOT_90         " << m_vetoBits[7] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_EL0_ETOT_01         " << m_vetoBits[8] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_SIDE                " << m_vetoBits[9] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_TOP                 " << m_vetoBits[10] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_SPLASH_1            " << m_vetoBits[11] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_E350_FILTER_TILE    " << m_vetoBits[12] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_E0_TILE             " << m_vetoBits[13] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_SPLASH_0            " << m_vetoBits[14] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_NOCALLO_FILTER_TILE " << m_vetoBits[15] << endreq;
      log << MSG::INFO << "EFC_GAMMA_STATUS_M_VETOED              " << m_vetoBits[16] << endreq;
//    }
      
    return StatusCode::SUCCESS;
}

