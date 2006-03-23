/* ---------------------------------------------------------------------- *//*!
   
   \file  filter.c
   \brief Driver program to test filtering code
   \author JJRussell - russell@slac.stanford.edu
   
\verbatim

  CVS $Id: OnboardFilter.cxx,v 1.47 2005/09/22 14:21:38 hughes Exp $
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
 * 02.09.05 jjr Corrected use of CACHE_invalidate, one must first do a
 *              flush to ensure everything is in memory before 
 *              invalidating the cache.
 *
\* ---------------------------------------------------------------------- */



//#define EFC_DFILTER
//#define EFC_FILTER

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "EFC/EDM.h"
//#include "EFC_ss.h"
#include "filter_rto.h"
#include "EFC/EFC_edsFw.h"
#include "AFC_splash.h"
#include "AFC_splashMapDef.h"

#include "EFC/EFC_gammaResultsPrint.h"
#include "EFC/EFC_gammaStatsPrint.h"
#include "EFC/EFC_gammaCfgPrint.h"
#include "EFC/EFC_gammaCfg.h"
#include "EFC/EFC_gamma.h"



#include "EFC/EFC_gammaStatus.h"
#include "EFC/EFC_gammaResult.h"
#include "EFC/EFC_gammaStats.h"
#include "EFC/TFC_projectionDef.h"


//#if       defined (EFC_DFILTER)
#include "EFC/EFC_display.h"
#include "EFC/TFC_geometryPrint.h"
#include "EFC/TFC_projectionPrint.h"
//#endif

#include "EFC/TFC_geos.h"
#include "EFC/TFC_geoIds.h"
#include "EFC_gammaResultDef.h"


#include "EDS/LCBV.h"
#include "EDS/EBF_dir.h"
#include "EDS/EBF_evt.h"
#include "EDS/EBF_pkt.h"
#include "EDS/EBF_mc.h"
#include "EDS/TMR.h"
#include "EDS/io/LCBP.h"
#include "EDS/io/EBF_stream.h"
#include "EDS/io/EBF_evts.h"
#include "EDS/EDS_fw.h"
#include "EDS/ECR_cal.h"
#include "EDS/EDR_cal.h"
#include "EDS/EDR_tkr.h"


#include "EDS/FFS.h"
#include "EDS/EBF_cid.h"
#include "EDS/EBF_ctb.h"
#include "EDS/EBF_gem.h"
#include "EDS/EBF_tkr.h"
#include "EDS/EBF_dir.h"
#include "EDS/EBF_evt.h"
#include "EDS/EDR_calUnpack.h"
#include "EDS/EDR_tkrUnpack.h"
#include "EDS/EDR_gemPrint.h"
#include "EDS/EDR_calPrint.h"
#include "EDS/EDR_tkrPrint.h"

 
#include "GaudiKernel/Algorithm.h"

#include "GaudiKernel/MsgStream.h"
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
#include "ATF_shadowedTowers2x2.h"
#include "AFC_splashMap1x1.h"


/* --------------------------------------------------------------------- */
/* Forward References                                                    */
/* --------------------------------------------------------------------- */
#ifndef CMX_DOXYGEN


#ifndef         AFC_SPLASHMAP_TD
#define         AFC_SPLASHMAP_TD
typedef struct _AFC_splashMap AFC_splashMap;
#endif


#ifndef         ATF_SHADOWEDTOWERS_TD
#define         ATF_SHADOWEDTOWERS_TD
typedef struct _ATF_shadowedTowers ATF_shadowedTowers;
#endif


#ifndef         EFC_GAMMACTL_TD
#define         EFC_GAMMACTL_TD
typedef struct _EFC_gammaCtl EFC_gammaCtl;
#endif

#endif
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

   \def    _ADVANCE(_ptr, _nbytes)
   \brief   Advances \a _ptr by \a _nbytes. The return value is always
            recast to the type of \a _ptr.
   \param   _ptr    The pointer to advance
   \param   _nbytes The number of nbytes to advance \a _ptr by.
   \return  The advanced pointer, recast to the type of \a _ptr.
                                                                         */
/* --------------------------------------------------------------------- */
#define _ADVANCE(_ptr, _nbytes) \
         ((unsigned char *)(_ptr) + (_nbytes))
/* --------------------------------------------------------------------- */


/*
  \par
   This structure describes the set of result vectors that are available
   to be used. It is considered a 'write-once' structure, that is, once
   initialized, it does not get altered until it is destroyed.
									 */
/* --------------------------------------------------------------------- */
typedef struct _FilterResultCtl
{
  EFC_gammaResult  *beg; /*!< The first result vector                    */
  EFC_gammaResult  *end; /*!< The last  result vector                    */
  EFC_gammaResult  *cur; /*!< The current result vector                  */
  int              size; /*!< The size, in bytes, of 1 result vector     */
}
FilterResultCtl;
/* --------------------------------------------------------------------- */


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

/* --------------------------------------------------------------------- *//*!

  \struct _FilterCtx
  \brief   Structure to bind all the filtering parameters together into
           something suitable for passing to the LCB event call back
           handler.
									 *//*!
  \typedef FilterCtx
  \brief   Typedef for struct _FilterCtx
									 */
/* --------------------------------------------------------------------- */
typedef struct _FilterCtx
{
  EFC_gammaCtl        *efc; /*!< The filter context proper               */
  EBF_evt             *evt; /*!< Pointer to the first packet of an event */
  FilterResultCtl   result; /*!< The stash of result vectors             */
  int                 ievt; /*!< Current event number                    */
  int             to_print; /*!< Number of events to print               */
  unsigned int ss_to_print; /*!< Bit mask of the subsystems to print     */
  int          useMcEnergy; /*!< Use the MC energy as the classifier     */
  EBF_stream          *ebo; /*!< Potential output stream                 */
  int           listPasses; /*!< Flag to print the events that pass
                                 the filter                              */
}
FilterCtx;
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \var   const TFC_geometry *TFC_Geos[]
  \brief A list of the available geometries.

   This is used as a cheap database of the known geometries.
                                                                         */
/* --------------------------------------------------------------------- */
extern const TFC_geometry *TFC_Geos[];
/* --------------------------------------------------------------------- */


class OnboardFilter:public Algorithm{

public:
  OnboardFilter(const std::string& name, ISvcLocator *pSvcLocator);
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

  int eventCount;
  int eventProcessed;
  int eventBad; 
  
  
      EDS_fw                      *fw;
    FilterCtx                   ctx;
    EFC_gammaCtl            *efcCtl;
    LCBP                        lcb;
    EFC_gammaResult        *results;
    int                  resultSize;
    EBF_stream                 *ebo;

/* ---------------------------------------------------------------------- */
/*  Internal functions                                                    */
/* ---------------------------------------------------------------------- */
//int       doFilter (const EFC_rto *rto);

static FilterInfo myFilterInfo;
static void     *allocate (int nbytes, const char *name);
static void       free_em (void *ptr);
static void  printElapsed (TMR_tick beg, TMR_tick end, int n);
int rtoFill (FilterRto *rto);

#ifdef EDM_USE
static void setMessageLevels (MessageObjLevels lvl);
EDM_level Filter_edm = EDM_K_FATAL;
#endif


static EFC_gammaCtl 
              *efcCtlConstruct (unsigned int vetoes,
				TFC_geoId     geoId,
				int        geoPrint); 

static EDS_fw             *setupFw   (LCBP          lcb,
				      FilterCtx    *ctx);

static const TFC_geometry *locateGeo (int            id, 
                                      int      printIt);


static void  fillCfg           (EFC_gammaCfgPrms                *cfg,
			        const AFC_splashMap       *splashMap,
		                const ATF_shadowedTowers   *shadowed,
 			        const TFC_geometry         *geometry);

static void  fillSampler       (EFC_gammaCfgSampler         *cfg,
				unsigned int             vetoes);

static int  filterEvt          (FilterCtx                  *ctx,
				unsigned int           pktBytes,
				EBF_pkt                    *pkt,
				EBF_siv                     siv,
				EDS_fwIxb                  *ixb);
static int  extractInfo         (FilterCtx                  *ctx,
				unsigned int           pktBytes,
				EBF_pkt                    *pkt,
				EBF_siv                     siv,
				EDS_fwIxb                  *ixb);

static void  fillTDS(FilterCtx *ctx, EDS_fwIxb *ixb, const EBF_dir *dir);


//#endif

void OnboardFilter::printRaw (unsigned int *pkt,
		      int psize);

static void resultsPrint       (TMR_tick                    beg, 
		   	        TMR_tick                    end,
			        const EFC_gammaResult  *results,
			        int                    nresults,
			        unsigned int            options);


static void statisticsPrint    (const EFC_gammaResult      *beg,
                                const EFC_gammaResult      *end,
				unsigned int             vetoes);

static void statisticsAltPrint (const EFC_gammaResult      *beg,
                                const EFC_gammaResult      *end,
				unsigned int             vetoes);

static __inline int getMCsequence  (const EBF_evt *evt);
static EBF_stream *createOutputFile (const char *name);

  void storeHits(OnboardFilterTds::TowerHits *hits);


#ifdef EDM_USE
void setMessageLevels (MessageObjLevels lvl);
EDM_level Filter_edm = EDM_K_FATAL;
#endif


  int m_mask;  //mask for setting filter to reject
  int m_rejected;
  int m_passThrough;
  int m_vetoBits[17];      //array to count # of times each veto bit was set
  int m_statusBits[15];      //array to count # of times each veto bit was set
   EFC_gammaStats  *statistics;

  FilterRto m_rto;
};

static const AlgFactory<OnboardFilter> Factory;
const IAlgFactory& OnboardFilterFactory = Factory;
FilterInfo OnboardFilter::myFilterInfo;

OnboardFilter::OnboardFilter(const std::string& name, ISvcLocator *pSvcLocator):Algorithm(name,pSvcLocator),m_rejected(0){
  declareProperty("mask",m_mask=0);
  declareProperty("PassThrough",m_passThrough=1);
//  GemInfo myGemInfo = GemInfo();
}


/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

   \fn          int getMCsequence (const EBF_evt *evt)
   \brief       Retrieves the GLEAM Monte Carlo Event Sequence Number
   \param   evt Pointer to the event
   \return      The GLEAM Monte Carlo Event Sequence Number.
                                                                          */
/* ---------------------------------------------------------------------- */
__inline int OnboardFilter::getMCsequence (const EBF_evt *evt)
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



    
/* ---------------------------------------------------------------------- *//*!

  \var    const AFC_splashMap AFC_SplashMap1x1
  \brief  Defines the 'far' neighbors of a given filter tile to be
          those tiles outside the tiles that touch that tile.

                                                                          */
/* ---------------------------------------------------------------------- */
extern const AFC_splashMap AFC_SplashMap1x1;
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \var    const ATF_shadowedTowersCfg ATF_ShadowedTowers2x2;
  \brief  Defines the shadowing towers to be consistent with the overlapping
          2 x 2 set of ACD tiles. Given an ACD tile, this data structure
          returns the set of shadowed towers.
                                                                          */
/* ---------------------------------------------------------------------- */
extern const ATF_shadowedTowers ATF_ShadowedTowers2x2;
/* ---------------------------------------------------------------------- */
    

#define CACHE_cleanData(_ptr, _nbytes) /* NOOP */

/* ---------------------------------------------------------------------- */
int OnboardFilter::rtoFill (FilterRto *rto)
{
   extern int     optind;
   extern char   *optarg;
   const char     *ofile;
//   char                c;
   int           nevents;
   int            nprint;
   int             nskip;
   int             quiet;
   int              list;
   unsigned int   vetoes;
   unsigned int       ss;
   unsigned int   energy;
   unsigned int esummary;
   unsigned int geometry;
   TFC_geoId      geo_id;

   EDM_CODE(
   EDM_level      dlevel;
   MessageObjLevels 
                  levels;)
   
   
   /* Establish the defaults */   
   nevents  = 1;
   ofile    =  NULL;
   nskip    =  0;
   nprint   =  1;
   ss       =  0;
   quiet    =  0;
   list     =  0;
   energy   =  0;
   esummary =  0;
   geometry =  0;
   geo_id   = TFC_K_GEO_ID_DEFAULT;
   vetoes   = 0;


   EDM_CODE
   (
      /* Establish the default message level */
      dlevel            = EDM_K_FATAL;   
      levels.filter     = dlevel;
      levels.efc_filter = dlevel;
      levels.tfc_ptf    = dlevel;
      levels.tfc_acd    = dlevel;
      levels.tfc_skirt  = dlevel;
   )
      
 
   /* Pick off one time initialization */
   geometry = ss & FLT_M_GEO;


   /* If nothing selected, default to all */
   if (ss == 0) ss = -1;


   /* Default classification scheme */
   if (energy   == 0) energy   = FLT_RESULT_ENERGY_K_CALCULATED;
   if (esummary == 0) esummary = EFC_GAMMA_RESULTS_PRINT_OPT_M_SUMMARY;

   
   /* Fill in the control structure */
   rto->type        = EBF_STREAM_TYPE_K_FILE;
   rto->to_process  = nevents;
   rto->to_skip     = nskip;
   rto->to_print    = nprint;
   rto->ss_to_print = ss;
   rto->quiet       = quiet;
   rto->geo_id      = TFC_K_GEO_ID_DEFAULT;
   rto->ofile       = ofile;
   rto->list        = list;
   rto->vetoes      = vetoes;
   rto->energy      = energy;
   rto->esummary    = esummary;
   rto->geometry    = geometry;
   rto->geo_id      = geo_id;   

   EDM_CODE(rto->levels = levels);

   return 0;
}


StatusCode OnboardFilter::initialize()
{
  eventCount = 0;
  eventProcessed = 0;
  eventBad = 0;
  MsgStream log(msgSvc(),name());
  setProperties();
  log<<MSG::INFO<<"Initializing Filter Settings"<<endreq;
  int      status; 
  status = rtoFill (&m_rto);
  if(!m_passThrough)
    m_rto.vetoes=EFC_GAMMA_STATUS_M_VETOES;

   statistics = (EFC_gammaStats  *) malloc (EFC_gammaStatsSizeof ());

   EFC_gammaStatsClear (statistics);


    /* Construct, i.e. alloc and initialize, the filtering control structure */
    efcCtl = efcCtlConstruct (m_rto.vetoes, m_rto.geo_id, m_rto.geometry);
    if (efcCtl == NULL) return -1;
    ctx.efc = efcCtl;


    /* Fill-in the EFC_filter context parameter */
    ctx.ievt         = m_rto.to_skip;
    ctx.to_print     = m_rto.to_print;
    ctx.to_print = 0;
    ctx.ss_to_print  = m_rto.ss_to_print;
    ctx.listPasses   = m_rto.list;
    ctx.useMcEnergy  = m_rto.energy;
    ctx.ebo          = ebo;

    int nevts = 1;

    /* Allocate the memory for the results vector */
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


    /* 
     | Set the callback routine and connect the packet stream for the
     | real filter
    */
    ctx.result.cur = ctx.result.beg;
//    LCBP_pktCbSet    (lcb, 1, (LCBV_pktCb)EDS_fwProcess, fw);

    /* Initialize the time base */
    TMR_initialize  ();

  if (status){
    log<<MSG::DEBUG<<"Failed to set up default values for control structure"<<endreq;
    return StatusCode::FAILURE;
  }
  for(int counter=0;counter<17;counter++)
    m_vetoBits[counter]=0;
  for(int counter=0;counter<15;counter++)
    m_statusBits[counter]=0;
  return StatusCode::SUCCESS;
}


/* ---------------------------------------------------------------------- *//*!

   \fn         int doFilter (const EFC_rto *rto)
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
    TMR_tick                    beg;
    TMR_tick                    end;
    EBF_evts                   evts;
    int                       ievts;
    int                       isize;
    int                       nevts;
//         unsigned int              resultCnt;
    int                    nresults;

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
      //memset(&TDS_variables.prjs[counter],0,sizeof(TDS_variables.prjs[counter]));
      memset(&TDS_variables.prjs,0,sizeof(TDS_variables.prjs));
      for(int layerCounter=0;layerCounter<36;layerCounter++){
	TDS_variables.hits[counter].layers[layerCounter].cnt=0;
	TDS_variables.hits[counter].layers[layerCounter].beg=NULL;
      }
    }
    
    MsgStream log(msgSvc(),name());
//
// Make the tds objects
    OnboardFilterTds::TowerHits *hits = new OnboardFilterTds::TowerHits;
    eventSvc()->registerObject("/Event/Filter/TowerHits",hits);
    OnboardFilterTds::FilterStatus *newStatus=new OnboardFilterTds::FilterStatus;
    eventSvc()->registerObject("/Event/Filter/FilterStatus",newStatus);
//
// Check for ebf on tds
    SmartDataPtr<EbfWriterTds::Ebf> ebfData(eventSvc(),"/Event/Filter/Ebf");
    if(!ebfData){
//      log<<MSG::ERROR<<"Unable to retrieve ebf data from TDS"<<endreq;
      return StatusCode::SUCCESS;
    }




    /* Set the diagnostic message/print levels */
    EDM_CODE (setMessageLevels (m_rto.levels));

   nevts = 1;
    
    /* Open, read, close and process the input data */
    unsigned int length;
    char *data=ebfData->get(length);
    if(length==0){
      log<<MSG::DEBUG<<"Event has no EBF data. Ignoring"<<endreq;
      return StatusCode::SUCCESS;
    }

    ebf = EBF_streamOpen(EBF_STREAM_TYPE_K_DATA,data,length);
    if (ebf == NULL){
      log<<MSG::ERROR<<"Unable to pass ebf data to the ebf reader"<<endreq;
      return StatusCode::SUCCESS;
    }
    
    //ebf    = EBF_streamOpen   (EBF_STREAM_TYPE_K_FILE, "/darkdata1/GLAST/FESDisks/test/AllGamma_0001_V2p0.ebf",0);
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
//    lcb = LCBP_get ();
//    LCBP_create (lcb);
//    fw = setupFw (lcb, &ctx);

    /* Allocate the memory for the results vector */
/*    resultSize = EFC_gammaResultSizeof ();
    results    = (EFC_gammaResult *)allocate (resultSize * nevts,
				             "result vectors");
    ctx.result.beg   = results;
    ctx.result.end   = (EFC_gammaResult *)((char *)results + resultSize*nevts);
    ctx.result.cur   = results;    
    ctx.result.size  = resultSize;
*/

    /* 
     | Set the callback routine and connect the packet stream for the
     | real filter
    */
    ctx.result.cur = ctx.result.beg;
    LCBP_pktCbSet    (lcb, 1, (LCBV_pktCb)EDS_fwProcess, fw);
    LCBP_pktsConnect (lcb, evts); 
    beg = TMR_GET ();
    LCBP_pktsDeliver (lcb);
    end = TMR_GET ();


    /* Print the results of the filtering process */
    nresults = ((char *)ctx.result.cur - (char *)ctx.result.beg) / resultSize;

   EFC_gammaStatsAccumulate (statistics, ctx.result.beg, ctx.result.end);


    if((results->status & EFC_GAMMA_STAGE_M_ACD) != 0)
      m_statusBits[0]++;
    if((results->status & EFC_GAMMA_STAGE_M_DIR) != 0)
      m_statusBits[1]++;
    if((results->status & EFC_GAMMA_STAGE_M_ATF) != 0)
      m_statusBits[2]++;
    if((results->status & EFC_GAMMA_STAGE_M_CAL1) != 0)
      m_statusBits[3]++;
    if((results->status & EFC_GAMMA_STAGE_M_TKR) != 0)
      m_statusBits[4]++;
    if((results->status & EFC_GAMMA_STATUS_M_ACD_TOP) != 0)
      m_statusBits[5]++;
    if((results->status & EFC_GAMMA_STATUS_M_ACD_SIDE) != 0)
      m_statusBits[6]++;
    if((results->status & EFC_GAMMA_STATUS_M_ACD_SIDE_FILTER) != 0)
      m_statusBits[7]++;
//    if((results->status & EFC_GAMMA_STATUS_M_TKR_POSSIBLE) != 0)
//      m_statusBits[8]++;
//    if((results->status & EFC_GAMMA_STATUS_M_TKR_TRIGGER) != 0)
//      m_statusBits[9]++;
//    if((results->status & EFC_GAMMA_STATUS_M_CAL_LO) != 0)
//      m_statusBits[10]++;
//    if((results->status & EFC_GAMMA_STATUS_M_CAL_HI) != 0)
//      m_statusBits[11]++;
    if((results->status & EFC_GAMMA_STATUS_M_TKR_EQ_1) != 0)
      m_statusBits[12]++;
    if((results->status & EFC_GAMMA_STATUS_M_TKR_GE_2) != 0)
      m_statusBits[13]++;
//    if((results->status & EFC_GAMMA_STATUS_M_TKR_THROTTLE) != 0)
//      m_statusBits[14]++;
/*
    if((results->status & EFC_GAMMA_STATUS_M_TKR_LT_2_ELO) != 0)
      m_vetoBits[0]++;
    if((results->status & EFC_GAMMA_STATUS_M_TKR_SKIRT) != 0)
      m_vetoBits[1]++;
    if((results->status & EFC_GAMMA_STATUS_M_TKR_EQ_0) != 0)
      m_vetoBits[2]++;
    if((results->status & EFC_GAMMA_STATUS_M_TKR_ROW2) != 0)
      m_vetoBits[3]++;
    if((results->status & EFC_GAMMA_STATUS_M_TKR_ROW01) != 0)
      m_vetoBits[4]++;
    if((results->status & EFC_GAMMA_STATUS_M_TKR_TOP) != 0)
      m_vetoBits[5]++;
    if((results->status & EFC_GAMMA_STATUS_M_ZBOTTOM) != 0)
      m_vetoBits[6]++;
    if((results->status & EFC_GAMMA_STATUS_M_EL0_ETOT_90) != 0)
      m_vetoBits[7]++;
    if((results->status & EFC_GAMMA_STATUS_M_EL0_ETOT_01) != 0)
      m_vetoBits[8]++;
    if((results->status & EFC_GAMMA_STATUS_M_SIDE) != 0)
      m_vetoBits[9]++;
    if((results->status & EFC_GAMMA_STATUS_M_TOP) != 0)
      m_vetoBits[10]++;
    if((results->status & EFC_GAMMA_STATUS_M_SPLASH_1) != 0)
      m_vetoBits[11]++;
    if((results->status & EFC_GAMMA_STATUS_M_E350_FILTER_TILE) != 0)
      m_vetoBits[12]++;
    if((results->status & EFC_GAMMA_STATUS_M_E0_TILE) != 0)
      m_vetoBits[13]++;
    if((results->status & EFC_GAMMA_STATUS_M_SPLASH_0) != 0)
      m_vetoBits[14]++;
    if((results->status & EFC_GAMMA_STATUS_M_NOCALLO_FILTER_TILE) != 0)
      m_vetoBits[15]++; 
    if((results->status & EFC_GAMMA_STATUS_M_VETOED) != 0)
      m_vetoBits[16]++;
*/ 
    if((results->status & EFC_GAMMA_STATUS_M_TKR_LT_2_ELO) != 0) {
      m_vetoBits[0]++;
      //printf("OnboardFilterV15: EFC_GAMMA_STATUS_M_TKR_LT_2_ELO\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_TKR_SKIRT) != 0) {
      m_vetoBits[1]++;
      //printf("OnboardFilterV16: EFC_GAMMA_STATUS_M_TKR_SKIRT\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_TKR_EQ_0) != 0) {
      m_vetoBits[2]++;
      //printf("OnboardFilterV17: EFC_GAMMA_STATUS_M_TKR_EQ_0\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_TKR_ROW2) != 0) {
      m_vetoBits[3]++;
      //printf("OnboardFilterV18: EFC_GAMMA_STATUS_M_TKR_ROW2\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_TKR_ROW01) != 0) {
      m_vetoBits[4]++;
      //printf("OnboardFilterV19: EFC_GAMMA_STATUS_M_TKR_ROW01\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_TKR_TOP) != 0) {
      m_vetoBits[5]++;
      //printf("OnboardFilterV20: EFC_GAMMA_STATUS_M_TKR_TOP\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_ZBOTTOM) != 0) {
      m_vetoBits[6]++;
      //printf("OnboardFilterV21: EFC_GAMMA_STATUS_M_ZBOTTOM\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_EL0_ETOT_90) != 0) {
      m_vetoBits[7]++;
      //printf("OnboardFilterV22: EFC_GAMMA_STATUS_M_EL0_ETOT_90\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_EL0_ETOT_01) != 0) {
      m_vetoBits[8]++;
      //printf("OnboardFilterV23: EFC_GAMMA_STATUS_M_EL0_ETOT_01\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_SIDE) != 0) {
      m_vetoBits[9]++;
      //printf("OnboardFilterV24: EFC_GAMMA_STATUS_M_SIDE\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_TOP) != 0) {
      m_vetoBits[10]++;
      //printf("OnboardFilterV25: EFC_GAMMA_STATUS_M_TOP\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_SPLASH_1) != 0) {
      m_vetoBits[11]++;
      //printf("OnboardFilterV26: EFC_GAMMA_STATUS_M_SPLASH_1\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_E350_FILTER_TILE) != 0) {
      m_vetoBits[12]++;
      //printf("OnboardFilterV27: EFC_GAMMA_STATUS_M_E350_FILTER_TILE\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_E0_TILE) != 0) {
      m_vetoBits[13]++;
      //printf("OnboardFilterV28: EFC_GAMMA_STATUS_M_E0_TILE\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_SPLASH_0) != 0) {
      m_vetoBits[14]++;
      //printf("OnboardFilterV29: EFC_GAMMA_STATUS_M_SPLASH_0\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_NOCALLO_FILTER_TILE) != 0) {
      m_vetoBits[15]++;
      //printf("OnboardFilterV30: EFC_GAMMA_STATUS_M_NOCALLO_FILTER_TILE\n");
    }
    if((results->status & EFC_GAMMA_STATUS_M_VETOED) != 0) {
      m_vetoBits[16]++;
      //printf("OnboardFilterV: EFC_GAMMA_STATUS_M_VETOED\n");
    } else {
      //printf("OnboardFilterV: NOT VETOED!\n");
    }


    newStatus->set(results->status);
    newStatus->setStageEnergy(results->stageEnergy);
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
    storeHits(hits);


    if(m_mask!=0 && (m_mask & (results->status >> 15)) !=0){
      this->setFilterPassed(false);
      m_rejected++;
    }

//   free_em ((void *)results);
//   free_em ((void *)fw);

    EBF_streamFree (ebf);

    return StatusCode::SUCCESS;
}
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- */


void OnboardFilter::storeHits(OnboardFilterTds::TowerHits *hits){

  struct OnboardFilterTds::TowerHits::towerRecord hitRecords[16];
  for(int counter=0;counter<16;counter++){
    hitRecords[counter].lcnt[0]=TDS_variables.hits[counter].lcnt[0];
    hitRecords[counter].lcnt[1]=TDS_variables.hits[counter].lcnt[1];
    hitRecords[counter].layerMaps[0]=TDS_variables.hits[counter].layerMaps[0];
    hitRecords[counter].layerMaps[1]=TDS_variables.hits[counter].layerMaps[1];
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

  \fn     static EFC_gammaCtl *efcCtlConstruct (unsigned int vetoes,
                                                TFC_geoId     geoId,
                                                int        geoPrint)
  \brief  Helper function to allocate and initialize the static, readonly
          filtering control structure.
  \return If successful, pointer to the initialized control structure,
          if unsuccessful, NULL.

  \param    vetoes The default veto mask
  \param     geoId The id of the geometry to use. 
  \param  geoPrint Simply flag indicating whether to print a display of
                   the geometry.
                                                                          */
/* ---------------------------------------------------------------------- */  
EFC_gammaCtl *OnboardFilter::efcCtlConstruct (unsigned int vetoes,
				      TFC_geoId     geoId,  
			              int        geoPrint)
{
    static EFC_gammaCfgPrms cfg;
    EFC_gammaCtl           *ctl;
    EFC_gammaCtlCfg     *ctlCfg;
    const TFC_geometry     *geo;


    /* Find the specified geometry */
    geo = locateGeo (geoId, geoPrint);
 
    /* 
     | Allocate and initialize the static control structure. This includes
     | calibration constants and those types of things.
    */
    ctl = (EFC_gammaCtl *)allocate (EFC_gammaCtlSizeof(),
			           "control structure");

    ctlCfg = (EFC_gammaCtlCfg *)allocate (EFC_gammaCtlCfgSizeof(),
					  "configuration structure");

    if (ctl) 
    {
      EFC_gammaCfgSampler sampler;
      OnboardFilter::fillCfg (&cfg, &AFC_SplashMap1x1, &ATF_ShadowedTowers2x2, geo);
      OnboardFilter::fillSampler (&sampler, vetoes);
      EFC_gammaCfgPrmsPrint    (&cfg, "Default");
      EFC_gammaCfgSamplerPrint (&sampler, "Default");
      EFC_gammaCtlCfgCompile   (ctlCfg, &cfg, &sampler);
      EFC_gammaCtlCfgSet       (ctl, ctlCfg);
      EFC_gammaCtlInit         (ctl);
    }


    return ctl;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn static int filterEvt (FilterCtx        *ctx,
                            unsigned int pktBytes,
			    EBF_pkt          *pkt,
			    EBF_siv           siv,
			    EDS_fwIxb        *ixb)
                             

  \brief  Calls the filter function for each of the specified events
  \return Status

  \param         ctx The filter context
  \param    pktBytes Number of bytes in this packet
  \param         pkt The event packet
  \param         siv The State Information Vector
  \param         ixb The Information Exchange Vector
                                                                          */
/* ---------------------------------------------------------------------- */
int OnboardFilter::filterEvt (FilterCtx        *ctx,
		      unsigned int pktBytes,
		      EBF_pkt          *pkt,
		      EBF_siv           siv,
		      EDS_fwIxb        *ixb)
{
    int                fate;
    EFC_gammaResult *result;
    int              status;

    result = ctx->result.cur;


    /* Do the actual filtering */
    fate   = EFC_gammaFilter (ctx->efc, 
			      pktBytes,
			      pkt,
			      siv,
			      ixb,
			      result);
    status = fate == EFC_FATE_K_MORE ? EDS_FW_FN_M_DIR : EDS_FW_FN_M_NO_MORE;


    /* Get a new result vector for the next go around */
    if (EBF__sivIsLast (siv) || (status & EDS_FW_FN_M_NO_MORE))
    {
        ctx->ievt      += 1;
        if (result < ctx->result.end) 
	    ctx->result.cur = (EFC_gammaResult *) _ADVANCE (result, ctx->result.size);
    }

    return status;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
//#include "EDS/ECR_cal.h"



int OnboardFilter::extractInfo (FilterCtx        *ctx,
		       unsigned int pktBytes,
		       EBF_pkt          *pkt,
		       EBF_siv           siv,
		       EDS_fwIxb        *ixb)
{
    EFC_gammaResult *result;
    int              status;

    result = &ctx->result.cur[-1];
    status = result->status;

    /* New event, the event being processed */
    EDM_DEBUGPRINTF( (Filter_edm, "GLASTSIM = %9d\n",
		      getMCsequence ((EBF_evt *)pkt)));

    /* If have stuff to print, then need a directory */
//    if (ctx->to_print > 0) 
//    {
         ctx->to_print -= 1;


    EDS_fwEvt *evt =  &ixb->blk.evt;
    unsigned int opts = ctx->ss_to_print;
//   if (opts)
//   {
      unsigned int sequence;
      unsigned int       mc;
      EBF_esw           esw;
      EBF_dir          *dir;
      const EBF_evt *rawEvt;

      dir      = evt->dir;
      rawEvt   = dir->evt;
      esw.ui   = dir->ctbs[EBF_CID_K_GEM].ctb->hdr.esw.ui;
      sequence = EBF_ESW_SEQ (esw.ui);
      mc       = rawEvt->hdr.undef[0];


      fillTDS(ctx,ixb,dir);
      
      if (opts & EFC_DISPLAY_SS_M_GEM) 
      {
	  const EBF_gem *gem;
	  gem = (const EBF_gem *)dir->ctbs[EBF_CID_K_GEM].ctb->dat;
//	  EDR_gemPrint (gem);
      
 unsigned int          twrMsk = 0xffffffff;
      
  if (result->stageEnergy    & EFC_GAMMA_STAGE_M_TKR  ) {
      const TFC_projections *prjs;
 prjs = (const TFC_projections *)ixb->blk.ptrs[EFC_EDS_FW_OBJ_K_PRJS];
   if (prjs->twrMsk > 0) 
	       {
 	       }
      }

      
      
   
   

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
    myFilterInfo.sent = gem->sent;   
   }
//   }}

 
    return 0;
}
/* ---------------------------------------------------------------------- */
//#endif
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \fn  void EFC_acdTkrDirDisplay (unsigned int style,
                                  const EBF_dir *dir)
                            
  \brief         Draws a display of the TKR layer hit mask and the struck
                 ACD tiles using only ASCII characters. 
  \param   style Display style, 0 = plain, 1 = fancy
  \param     dir The directory of event contributors.

   This routine displays the tracker and ACD data using the event data
   as the source. The tower data for each TKR is taken as the coincidence
   of the X and Y of the layer end OR of the accept list. The ACD data
   is from the veto list in the GEM block.
                                                                          */
/* ---------------------------------------------------------------------- */
void  OnboardFilter::fillTDS(FilterCtx *ctx,EDS_fwIxb *ixb,const EBF_dir *dir)
{
   const EBF_dirCtbDsc *contributors;
   int                          cids;
   int                           cid;
   int                         tids;
   int                         tcids;
   EFC_gammaResult *result;
//   int              status;
   EDR_tkr                   *tkr;
   EDR_tkrTower          *ttrs;

   result = &ctx->result.cur[-1];
   

   tcids        = 0;
   tids         = dir->ctids << 16;
   cids         = EBF_DIR_TEMS_TKR (dir->ctids);
   contributors = dir->ctbs;
//   EDR_tkrUnpack (tkr, dir, cids);
   tkr = ixb->blk.evt.tkr;

   unsigned int          twrMsk = 0xffffffff;
   TDS_variables.tmsk=EBF_DIR_TEMS_TKR (dir->ctids);

   memcpy(TDS_variables.layerEnergy,ixb->blk.evt.cal->layerEnergies,8*sizeof(int));
//   TDS_variables.layerEnergy[0] = ixb->blk.evt.cal->layerEnergies[0];
      
   if (result->stageEnergy    & EFC_GAMMA_STAGE_M_TKR  ) {
      const TFC_projections *prjs;
      prjs = (const TFC_projections *)ixb->blk.ptrs[EFC_EDS_FW_OBJ_K_PRJS];
   ttrs   = tkr->twrs;



      memcpy(&(TDS_variables.prjs),prjs,sizeof(*prjs));
      memcpy(&(TDS_variables.tkr),tkr,sizeof(*tkr));
      if (twrMsk == -1) twrMsk  = prjs->twrMsk << 16;
      else              twrMsk &= 0xffff0000;
      while (twrMsk)
      {
         int towerId = FFS (twrMsk);
         const TFC_projectionDir *dir = prjs->dir + towerId;


//       int                  tower;
       EDR_tkrTower          *ttr;
//       unsigned int     tkrStatus;
//       TFC_projectionDir     *dir;
       
       ttr   = ttrs + towerId;

      TDS_variables.hits[towerId].lcnt[0] = ttr->lexycnts[0];
      TDS_variables.hits[towerId].lcnt[1] = ttr->lexycnts[0];
      TDS_variables.hits[towerId].layerMaps[0] = ttr->layerMaps[0];
      TDS_variables.hits[towerId].layerMaps[1] = ttr->layerMaps[1];
      int counter = towerId;
		 for(int layers=0;layers<36;layers++){
		   TDS_variables.hits[counter].layers[layers].cnt
            =ttr->layers[layers].cnt;
		   TDS_variables.hits[counter].layers[layers].beg
            =(TFC_strip*)malloc(TDS_variables.hits[counter].layers[layers].cnt*sizeof(TFC_strip));
		   memcpy(TDS_variables.hits[counter].layers[layers].beg,
            ttr->layers[layers].beg,
            TDS_variables.hits[counter].layers[layers].cnt*sizeof(TFC_strip));
		 }

   TDS_variables.layerEnergy[0] = ixb->blk.evt.cal->layerEnergies[0];

         TDS_variables.xCnt[towerId]=dir->xCnt;
         TDS_variables.yCnt[towerId]=dir->yCnt;

         twrMsk = FFS_eliminate (twrMsk, towerId);
      }
   }

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

   /* Is there a GEM contribution?  */
//   if (dir->cids & (EBF_CID_K_GEM << 16))
//   {
      const EBF_gem *gem;
      gem     = (const EBF_gem *)dir->ctbs[EBF_CID_K_GEM].ctb->dat;
      unsigned int acd_top = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XY];
      unsigned int acd_x   = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_YZ];
      unsigned int acd_y   = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XZ];
      TDS_variables.acd_xz=acd_y;
      TDS_variables.acd_yz=acd_x;
      TDS_variables.acd_xy=acd_top;
//   }

    TDS_variables.tcids=tcids;
   
    
   return;
   
}



/* ---------------------------------------------------------------------- *//*!

  \fn static EDS_fw *setupFw (LCBP lcb, FilterCtx *ctx)
  \brief Sets up the EDS framework

  \param lcb The LCB device handle
  \param ctx The filtering context
									  */
/* ---------------------------------------------------------------------- */
EDS_fw *OnboardFilter::setupFw (LCBP lcb, FilterCtx *ctx)
{
    EDS_fw                       *fw;
    unsigned int             objects;
    EDS_fwHandlerProcessor processor;

    fw  = (EDS_fw*)allocate (EDS_fwSizeof (), "EDS framework");
    EDS_fwInit (fw,
		NULL, NULL, NULL,
		NULL, NULL,
		(LCBV_pktsRngFreeCb)LCBP_pktsRngFree, lcb);


    objects = EFC_gammaFwObjects ();

    /* Add the projections to the list of managed objects */
    EFC_edsFwObjectsAdd (fw, objects);


    /* Determine which processor to use */
    processor = (EDS_fwHandlerProcessor)filterEvt;

          ctx->to_print = 0;
    EDS_fwRegister (fw,
		    0,
		    objects,
		    EFC_gammaFwNeeds (),
		    processor,
		    (EDS_fwHandlerFlush)NULL,
		    (EDS_fwHandlerDestructor)NULL,
		    ctx);

    EDS_fwChange (fw, EDS_FW_MASK(0), EDS_FW_MASK(0));



    /* If have something to display ... */
//#   if  defined (EFC_DFILTER)

//        if (ctx->ss_to_print && ctx->to_print)
//        {
          ctx->to_print = 1;

  	    int id;
	    id = EDS_fwRegister (fw,
				 -1,
				 objects,
               EDS_FW_FN_M_LAST | EDS_FW_FN_M_DIR,
				 (EDS_fwHandlerProcessor)extractInfo,
				 (EDS_fwHandlerFlush)NULL,
				 (EDS_fwHandlerDestructor)NULL,
				 ctx);
	    EDS_fwChange (fw, EDS_FW_MASK(id), EDS_FW_MASK(id));

//	}

//#   endif



    return fw;
}



/* ---------------------------------------------------------------------- *//*!

  \fn     static void  fillCfg  (EFC_gammaCfgPrms          *cfg,
	                   const AFC_splashMap       *splashMap,
		           const ATF_shadowedTowers   *shadowed,
 	                   const TFC_geometry         *geometry);

  \brief  Silly routine to fill in the configaration block

  \param cfg The configuration block to fill in
  \param splashMap The ACD splashMap to use
  \param shadowed  The structure giving which tiles shadow which towers
  \param geometry  The tracker geometry

  \par
   This routine fills the configuration block with, what were at one time,
   sensible values.
									  */
/* ---------------------------------------------------------------------- */
void  OnboardFilter::fillCfg  (EFC_gammaCfgPrms          *cfg,
	         const AFC_splashMap       *splashMap,
		 const ATF_shadowedTowers   *shadowed,
 	         const TFC_geometry         *geometry)
{
    cfg->v0.tag.fld.version        = 0;
    cfg->v0.tag.fld.type           = EDS_TAG_TYPE_K_BUILTIN;
    cfg->v0.tag.fld.instance       = 0;
    cfg->v0.acd.topSideEmax        = ECR_CAL_MEV_TO_LEU (10);
    cfg->v0.acd.topSideFilterEmax  = ECR_CAL_MEV_TO_LEU (350);
    cfg->v0.acd.splashEmax         = ECR_CAL_MEV_TO_LEU (40000);
    cfg->v0.acd.splashCount        = 4;
    cfg->v0.acd.splashMap          = splashMap;

    cfg->v0.atf.emax               = ECR_CAL_MEV_TO_LEU (5000);
    cfg->v0.atf.shadowed           = shadowed;
    cfg->v0.zbottom.emin           = ECR_CAL_MEV_TO_LEU (100);

    cfg->v0.cal.emin               = ECR_CAL_MEV_TO_LEU (0);
    cfg->v0.cal.emax               = ECR_CAL_MEV_TO_LEU (300);
    cfg->v0.cal.layer0RatioLo      = 10;                /* 10 /1024 = 1%    */
    cfg->v0.cal.layer0RatioHi      = 900;               /* 900/1024 = 90%   */

    cfg->v0.tkr.row2Emax           = ECR_CAL_MEV_TO_LEU (30000);
    cfg->v0.tkr.row01Emax          = ECR_CAL_MEV_TO_LEU (10000);
    cfg->v0.tkr.topEmax            = ECR_CAL_MEV_TO_LEU (30000);
    cfg->v0.tkr.zeroTkrEmin        = ECR_CAL_MEV_TO_LEU (250);
    cfg->v0.tkr.twoTkrEmax         = ECR_CAL_MEV_TO_LEU ( 5);
    cfg->v0.tkr.skirtEmax          = ECR_CAL_MEV_TO_LEU (20);
    cfg->v0.tkr.geometry           = geometry;

    return;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     static void  fillSampler  (EFC_gammaCfgSampler   *cfg,
	                             unsigned int        vetoes)

  \brief  Silly routine to fill the filter sampling control structure

  \param     cfg  The filter sampling configuration structure to fill
  \param  vetoes  The bit mask of active vetoes
									  */
/* ---------------------------------------------------------------------- */
void  OnboardFilter::fillSampler  (EFC_gammaCfgSampler   *cfg,
			   unsigned int        vetoes)
{
    cfg->tag.fld.version  = 0;
    cfg->tag.fld.type     = EDS_TAG_TYPE_K_BUILTIN;
    cfg->tag.fld.instance = 0;
    cfg->vetoes           = vetoes;
    cfg->enables          = 0;
    return;
}
/* ---------------------------------------------------------------------- */   

  



/* ---------------------------------------------------------------------- *//*!

  \fn     const TFC_geometry *locateGeo (int id, int printIt)
  \brief  Locates the specified geometry within the list of available
          geometries.
  \return A pointer to the specified geometry.

  \param      id  The id of the specified geometry.
  \param printIt  If non-zero, prints the geometry.

   If the specified geometry is not located, a list of the available 
   geometries is printed and the program exits
									  */
/* ---------------------------------------------------------------------- */
const TFC_geometry *OnboardFilter::locateGeo (int id, int printIt)
{

    const TFC_geometry *geo;

    /* Lookup the specified geometry */
    geo = TFC_geosLocate (TFC_Geos, -1, id);

    /* If can't find the specified geometry, report error and quit */
    if (!geo) 
    {
       printf ("ERROR: Unable to locate the specified geometry, id = %d\n"
	       "       The available choices are\n\n");

       EDM_CODE 
       (
          {
            int  idx;

	    /* Print just a header line */
	    puts ("");
	    TFC_geometryPrint (NULL, TFC_M_GEO_OPTS_TAG_HDR);

	    /* Print the tag for each */
	    idx = 0;
	    while ((geo = TFC_Geos[idx++])) 
	    {
	     TFC_geometryPrint (geo, TFC_M_GEO_OPTS_TAG);
	    }
         }
      )

       exit (-1);
    }

   
    /* Print the detector geometry if desired */
    EDM_CODE
    (
     puts ("");
     TFC_geometryPrint (geo, 
                        printIt ? TFC_M_GEO_OPTS_ALL 
                                : TFC_M_GEO_OPTS_TAG_HDR | TFC_M_GEO_OPTS_TAG);
    )
 
    return geo;
}
/* ---------------------------------------------------------------------- */





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




/* ---------------------------------------------------------------------- *//*!

  \fn     EBF_stream *createOutputFile (const char *name)
  \brief  Creates an output file if \a name is not NULL
  \return A handle to write to if name is not NULL, else NULL

  \param  name The name of the output file to create
									  */
/* ---------------------------------------------------------------------- */
EBF_stream *OnboardFilter::createOutputFile (const char *name)
{
    EBF_stream *ebo;
 
    ebo = EBF_streamCreate (EBF_STREAM_TYPE_K_FILE, name, 0);
    if (ebo == 0)
    {
      printf (" Error %d creating output file: %s\n", errno, name);
      return 0;
    }
    else
    {
      printf ("Output file   : %s\n", name);
    }

    return ebo;
}
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
  extern EDM_level              EFC_Filter_edm;
  extern EDM_level                 TFC_Acd_edm;
  extern EDM_level               TFC_Skirt_edm;
  extern EDM_level TFC_ProjectionTowerFind_edm;

  Filter_edm                  = lvl.filter;
  EFC_Filter_edm              = lvl.efc_filter;
  TFC_ProjectionTowerFind_edm = lvl.tfc_ptf;
  TFC_Acd_edm                 = lvl.tfc_acd;
  TFC_Skirt_edm               = lvl.tfc_skirt;

  return;
}
/* ---------------------------------------------------------------------- */
#endif
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn  void resultsPrint (TMR_tick                   beg, 
			  TMR_tick                   end,
			  const EFC_gammaResult *results,
			  int                   nresults,
			  unsigned int           options)
  \brief Prints the results vector or a summary thereof.

  \param      beg  The beginning time, in TMR_tick  
  \param      end  The ending time,    in TMR_tick 
  \param  results  The results vector
  \param nresults  The number of results vectors and, by implication,
                    the number of events processed
  \param  options  An bit mask of options passed to EFC_gammaResultsPrint
                                                                          */
/* ---------------------------------------------------------------------- */
void OnboardFilter::resultsPrint (TMR_tick                    beg, 
			  TMR_tick                    end,
			  const EFC_gammaResult  *results,
			  int                    nresults,
			  unsigned int            options)
{


   /* Print the results summary */    
   EFC_gammaResultsPrint (results,  nresults, options);
   printElapsed (beg, end, nresults);

   return; 
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn    void statisticsPrint (const EFC_gammaResult  *beg,
                               const EFC_gammaResult  *end,
			       unsigned int         vetoes)
  \brief Prints the summary statistics

  \param      beg The first result vector in the array
  \param      end The last  (actually 1 past the last) result vector in 
                  the array
  \param   vetoes The set of active vetoes
                                                                          */
/* ---------------------------------------------------------------------- */
void OnboardFilter::statisticsPrint (const EFC_gammaResult  *beg,
			     const EFC_gammaResult  *end,
			     unsigned int         vetoes)
{
   TMR_tick     begClr, endClr;
   TMR_tick     begAcc, endAcc;
   EFC_gammaStats  *statistics;
   int                nresults;

   statistics = (EFC_gammaStats  *)malloc (EFC_gammaStatsSizeof ());

   begClr = TMR_GET();
   EFC_gammaStatsClear (statistics);
   endClr = TMR_GET();


   begAcc = TMR_GET();
   EFC_gammaStatsAccumulate (statistics, beg, end);
   endAcc = TMR_GET();

   nresults = end - beg;
   EFC_gammaStatsPrint (statistics, vetoes);
   printElapsed         (begClr, endClr, nresults);
   printElapsed         (begAcc, endAcc, nresults);

   free (statistics);

   return;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn    void statisticsAltPrint (const EFC_gammaResult  *beg,
                                  const EFC_gammaResult  *end,
				  unsigned int        vetoes)
  \brief Prints the summary statistics using alternate energy bins

  \param      beg The first result vector in the array
  \param      end The last  (actually 1 past the last) result vector in 
                  the array
  \param   vetoes The set of active vetoes
                                                                          */
/* ---------------------------------------------------------------------- */
void OnboardFilter::statisticsAltPrint (const EFC_gammaResult  *beg,
                                const EFC_gammaResult  *end,
				unsigned int         vetoes)
{
   static const unsigned int Energies[] =
   {
     ECR_CAL_MEV_TO_LEU (    18),
     ECR_CAL_MEV_TO_LEU (   180),
     ECR_CAL_MEV_TO_LEU (  1800),
     ECR_CAL_MEV_TO_LEU ( 18000),
     ECR_CAL_MEV_TO_LEU (180000)
   };

   TMR_tick     begClr, endClr;
   TMR_tick     begAcc, endAcc;
   EFC_gammaStats  *statistics;
   int                nresults;

   statistics = (EFC_gammaStats  *)malloc (EFC_gammaStatsSizeof ());

   begClr = TMR_GET();
   EFC_gammaStatsClear (statistics);
   endClr = TMR_GET();


   begAcc = TMR_GET();
   EFC_gammaStatsAltAccumulate (statistics, 
				beg, 
				end,
				sizeof (Energies)/ sizeof (*Energies),
				Energies);
   endAcc = TMR_GET();

   EFC_gammaStatsAltPrint (statistics,
			   vetoes,
			   sizeof (Energies)/ sizeof (*Energies),
			   Energies);

   nresults = end - beg;
   printElapsed (begClr, endClr, nresults);
   printElapsed (begAcc, endAcc, nresults);

   free (statistics);

   return;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn    printElapsed (TMR_tick beg, TMR_tick end, int n)
  \brief Utility routine to print elapsed times

  \param beg The beginning time
  \param end The ending time
  \param   n The normalizing count

									  */
/* ---------------------------------------------------------------------- */
void  OnboardFilter::printElapsed (TMR_tick beg, TMR_tick end, int n)
{
//   unsigned int      eticks;
//   TMR_usecs_nsecs  elapsed;  /* Total elapsed time                       */
//   TMR_usecs_nsecs nelapsed;  /* Normalized (by event count) elapsed time */


   return;
}


/* ---------------------------------------------------------------------- *//*!
    
    \fn  static void printRaw (const EBF_pkt         *pkt,
                               unsigned int         psize,
                               int               sequence)
    \brief           Prints a very crude hex dump of the event to stdout.
    \param      pkt  Pointer to the packet to print
    \param    psize  The packet size in bytes (includes header)
    \param sequence  The event sequence number.
                                                                          */
/* ---------------------------------------------------------------------- */
void OnboardFilter::printRaw (unsigned int *pdata,
		      int psize)
{
//   int         pcktseq;
   int        idy  = 0;
   int        nout = 8;



   printf (" Printraw Size = %d", psize);
      
   psize /= sizeof (int);
   while (idy < psize)
   {
     /* Is a new header line needed? */
     if (nout >= 8)
     {
         printf ("\n %3d:", idy);
         nout = 0;
     }
           
     printf (" %8.8x", pdata[idy]);
     nout += 1;
     idy  += 1;
   }

   if (nout == 8) printf ("\n");

   return;
}

/* ---------------------------------------------------------------------- */

StatusCode OnboardFilter::finalize(){
	using namespace std;
    printf("OnboardFilter::finalize: events total %d; processed %d; bad %d: %d\n",
         eventCount,eventProcessed,eventBad);
    MsgStream log(msgSvc(),name());
    log  << MSG::INFO << "Rejected " << m_rejected << " triggers using mask " << m_mask  << endreq;
    log << MSG::INFO;

   EFC_gammaStatsPrint (statistics, m_rto.vetoes);

   free (statistics);
    free_em ((void *)efcCtl);
       free_em ((void *)results);
   free_em ((void *)fw);

//    if(log.isActive()){

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
//    }
  return StatusCode::SUCCESS;
}

