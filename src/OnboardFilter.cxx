/* ---------------------------------------------------------------------- */
/*!
   
   \file  filter.c
   \brief Driver program to test filtering code
   \author JJRussell - russell@slac.stanford.edu
   
\verbatim

  CVS $Id
\endverbatim
    
                                                                          */
/* ---------------------------------------------------------------------- */


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "DFC/DFC_filter.h"
#include "DFC_rto.h"
#include "DFC/EDM.h"

#include "DFC/EBF_fileIn-Gleam.h"
#include "DFC/EBF_fileOut.h"

#include "DFC/DFC_ss.h"
#include "DFC/DFC_ctl.h"
#include "DFC/DFC_evt.h"

#include "DFC_resultsDef.h"
#include "DFC/DFC_status.h"
#include "DFC/DFC_statistics.h"
#include "DFC/DFC_latUnpack.h"


#include "DFC/DFC_display.h"
#include "DFC/TFC_geometryPrint.h"
#include "DFC/TFC_projectionPrint.h"
#include "DFC/TFC_geos.h"
#include "DFC/TFC_geoIds.h"
#include "DFC_resultsDef.h"
#include "DFC/TFC_triggerFill.h"

#include "tmr.h"

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

#define DFC_M_STATUS_TKR_LT_2_ELO        0x8000
#define DFC_M_STATUS_TKR_SKIRT           0x10000
#define DFC_M_STATUS_TKR_EQ_0            0x20000
#define DFC_M_STATUS_TKR_ROW2            0x40000
#define DFC_M_STATUS_TKR_ROW01           0x80000
#define DFC_M_STATUS_TKR_TOP             0x100000
#define DFC_M_STATUS_ZBOTTOM             0x200000
#define DFC_M_STATUS_EL0_ETOT_90         0x400000
#define DFC_M_STATUS_EL0_ETOT_01         0x800000
#define DFC_M_STATUS_SIDE                0x1000000
#define DFC_M_STATUS_TOP                 0x2000000
#define DFC_M_STATUS_SPLASH_1            0x4000000
#define DFC_M_STATUS_E350_FILTER_TILE    0x8000000
#define DFC_M_STATUS_E0_TILE             0x10000000
#define DFC_M_STATUS_SPLASH_0            0x20000000
#define DFC_M_STATUS_NOCALLO_FILTER_TILE 0x40000000
#define DFC_M_STATUS_VETOED              0x80000000

/* ---------------------------------------------------------------------- */
/*!

\def   CACHE_invalidateData(_ptr, _nbytes)
\brief Macro to laundry the cache invalidate routine. This is a NOP on
all but VxWorks platforms
                                                                          */
/* ---------------------------------------------------------------------- */  
#define CACHE_invalidateData(_ptr, _nbytes) /* NOOP */

/* --------------------------------------------------------------------- */
/*!

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




/* --------------------------------------------------------------------- */
/*!

  \var   const struct _TFC_geometry *TFC_Geos[]
  \brief A list of the available geometries.

   This is used as a cheap database of the known geometries.
                                                                         */
/* --------------------------------------------------------------------- */
extern const struct _TFC_geometry *TFC_Geos[];
/* --------------------------------------------------------------------- */


class OnboardFilter:public Algorithm{
public:
  OnboardFilter(const std::string& name, ISvcLocator *pSvcLocator);
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
private:
  std::string convertBase(unsigned int number);
  void *allocate (int nbytes, const char *name);
  void   free_em (void *ptr);
  
  
  const struct _DFC_ctl *dfcCtlConstruct (TFC_geoId geoId, int geoPrint);
  struct _DFC_evt *dfcEvtConstruct (void);
  
  const struct _TFC_geometry *locateGeo (int            id, 
					 int      printIt);
  
  int  countEvts (const unsigned int *evts, 
		  int                 size);
  
  const unsigned int *setupEvts (const unsigned int  *evt,
				 int              totsize,
				 int              maxevts,
				 int                nskip,
				 int               *nevts,
				 unsigned int       *size);
  
  int filterEvts         (const unsigned int         *evt,
			  int                     nbegevt,
			  int                     nendevt,
			  int                      nprint,
			  unsigned int        ss_to_print,
			  const struct _DFC_ctl   *dfcCtl,
			  struct _DFC_results    *results,
			  int                 resultsSize,
			  struct _DFC_evt         *dfcEvt,
			  unsigned int             vetoes);
  
  int filterCompleteEvts (const unsigned int         *evt,
			  int                     nbegevt,
			  int                     nendevt,
			  EBF_ofile                  *ebo,
			  const struct _DFC_ctl   *dfcCtl,
			  struct _DFC_results    *results,
			  int                 resultsSize,
			  struct _DFC_evt         *dfcEvt,
			  unsigned int        listPasses);
  
  void     resultsPrint  (TMR_tick                       beg, 
			  TMR_tick                       end,
			  const struct _DFC_results *results,
			  int                       nresults,
			  unsigned int               options);
  
  void statisticsPrint   (const struct _DFC_results *results,
			  int                       nresults);
  
  __inline int getMCsequence (const unsigned int *evt, int esize);
  EBF_ofile *createOutputFile (const char *name);
  void storeHits(OnboardFilterTds::TowerHits *hits);
  
  IntegerProperty m_mask;  //mask for setting filter to reject
  int m_rejected;
  int m_passThrough;
  int m_vetoBits[17];      //array to count # of times each veto bit was set
  DFC_rto m_rto;
};

static const AlgFactory<OnboardFilter> Factory;
const IAlgFactory& OnboardFilterFactory = Factory;

OnboardFilter::OnboardFilter(const std::string& name, ISvcLocator *pSvcLocator):Algorithm(name,pSvcLocator),m_rejected(0){
  declareProperty("mask",m_mask=0);
  declareProperty("PassThrough",m_passThrough=1);
}


/* ---------------------------------------------------------------------- */
/*!
  
   \fn          int getMCsequence (const unsigned int *evt, int esize)
   \brief       Retrieves the GLEAM Monte Carlo Event Sequence Number
   \param   evt Pointer to the event
   \param esize The event size
   \return      The GLEAM Monte Carlo Event Sequence Number.
                                                                          */
/* ---------------------------------------------------------------------- */
__inline int OnboardFilter::getMCsequence (const unsigned int *evt, int esize)
{
  return evt[0x10];
}
/* ---------------------------------------------------------------------- */
    

StatusCode OnboardFilter::initialize()
{
  MsgStream log(msgSvc(),name());
  setProperties();
  log<<MSG::INFO<<"Initializing Filter Settings"<<endreq;
  int      status; 
  status = DFC_rtoFill (&m_rto, 0, NULL);
  if(!m_passThrough)
    m_rto.vetoes=DFC_M_STATUS_VETOES;
  if (status){
    log<<MSG::DEBUG<<"Failed to set up default values for control structure"<<endreq;
    return StatusCode::FAILURE;
  }
  for(int counter=0;counter<17;counter++)
    m_vetoBits[counter]=0;
  return StatusCode::SUCCESS;
}
/* ---------------------------------------------------------------------- */



StatusCode OnboardFilter::execute()
{
    unsigned int             status;
    EBF_ofile                  *ebo;
    EBF_file                   *ebf;
    TMR_tick                    beg;
    TMR_tick                    end;
    struct _DFC_evt         *dfcEvt;
    const struct _DFC_ctl   *dfcCtl;
    struct _DFC_results    *results=NULL;
    int                 resultsSize;
    const unsigned int        *evts;
    unsigned int               size;
    int                       nevts;
    int                     ibegevt;
    int                     iendevt;
    int                      evtCnt;
    
    //Initialize variables that will temporarily store data to be put in TDS
    for(int counter=0;counter<16;counter++)
      TDS_layers[counter]=0;
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
    OnboardFilterTds::TowerHits *hits = new OnboardFilterTds::TowerHits;
    eventSvc()->registerObject("/Event/Filter/TowerHits",hits);
    OnboardFilterTds::FilterStatus *newStatus=new OnboardFilterTds::FilterStatus;
    eventSvc()->registerObject("/Event/Filter/FilterStatus",newStatus);
    
    /* Initialize the time base */
    TMR_initialize  ();
    
    /* Print information about the runtime options */
    //DFC_rtoPrint (rto);
    
    /* Set the diagnostic message/print levels */
    EDM_CODE (setMessageLevels (rto->levels));
    
    /* Construct, i.e. alloc and initialize, the filtering control structure */
    dfcCtl = dfcCtlConstruct (m_rto.geo_id, m_rto.geometry);
    if (dfcCtl == NULL) return StatusCode::FAILURE;
    
    /* Construct the storage needed to analayze an event */
    dfcEvt = dfcEvtConstruct ();
    if (dfcEvt == NULL){  
      free_em ((void *)dfcCtl);
      log<<MSG::ERROR<<"Cannot allocated memory for DFC unpack record"<<endreq;
      return StatusCode::FAILURE;
    }
    SmartDataPtr<EbfWriterTds::Ebf> ebfData(eventSvc(),"/Event/Filter/Ebf");
    if(!ebfData){
      log<<MSG::ERROR<<"Unable to retrieve ebf data from TDS"<<endreq;
      return StatusCode::FAILURE;
    }
    
    /* Open, read, close and process the input data */
    unsigned int length;
    char *data=ebfData->get(length);
    if(length==0){
      log<<MSG::DEBUG<<"Event has no EBF data. Ignoring"<<endreq;
      free_em((void *)dfcCtl);
      free_em((void *)dfcEvt);
      return StatusCode::SUCCESS;
    }
    ebf    = EBF_openGleam(length);
    if (ebf == NULL){
      free_em ((void *)dfcCtl);
      free_em ((void *)dfcEvt);
      log<<MSG::ERROR<<"Unable to pass ebf data to the ebf reader"<<endreq;
      return StatusCode::FAILURE;
    }

    status = EBF_readGleam    (ebf,(unsigned int *)data);
    status = EBF_closeGleam   (ebf);
    size   = EBF_esizeGleam   (ebf);
    evts   = EBF_edataGleam   (ebf);
    nevts  = countEvts   (evts, size);
    
    
    /* Report the size of the file and the number of events*/
    //printf ("File Size  : %8d bytes\n"
    //        "Event Count: %8d\n",
    //        size,
    //        nevts);
    
    
    /* Create an output file by specified name iff rto->ofile is not NULL */
    ebo = 0;
    if ( m_rto.ofile && ((ebo = createOutputFile (m_rto.ofile)) == 0) ){
      free_em ((void *)dfcCtl);
      free_em ((void *)dfcEvt);
      EBF_free (ebf);
      log<<MSG::ERROR<<"Unable to create output file"<<endreq;
      return StatusCode::FAILURE;
    }
    
    
    /* 
     | Position the beginning event pointer by obeying the number of 
     | events to skip. Also make sure there are enough events to process
    */
    ibegevt = m_rto.to_skip;
    evts    = setupEvts (evts, 
		         size, 
		         m_rto.to_process, 
		         ibegevt,
		         &evtCnt,
		         &size);
    iendevt = ibegevt + evtCnt;


    /* Allocate the memory for the results vector */
    resultsSize = sizeof(DFC_results);
    results     = (struct _DFC_results *)allocate (resultsSize * evtCnt,
						   "result vectors");
    if (results == NULL){
      free_em ((void *)dfcCtl);
      free_em ((void *)dfcEvt);
      EBF_ofree (ebo);
      EBF_free  (ebf);
      log<<MSG::ERROR<<"Cannot allocate memory for the results vector"<<endreq;
      return StatusCode::FAILURE;
    }


    /* Invalidate the cache where the events are stored */
    CACHE_invalidateData (evts, size);
    

    /* Time and do the actual filtering */
    beg     = TMR_GET ();
    status  = filterEvts (evts, ibegevt, iendevt,
			  m_rto.to_print, m_rto.ss_to_print, 
			  dfcCtl, 
			  results, resultsSize, dfcEvt,  m_rto.vetoes);
    end     = TMR_GET ();
    
    
    /* Ensure that the energy status is filled in */
    status = filterCompleteEvts (evts, ibegevt, iendevt,
                                 ebo,
                                 dfcCtl, 
				 results, resultsSize, dfcEvt, m_rto.list);
    
    
    /*
      |  Print the results of the filtering process
    */
    //resultsPrint (beg, end, results, evtCnt, rto->esummary);

    /* If not quiet mode, print the statistics */
    //if (rto->quiet == 0) statisticsPrint (results, evtCnt);

    if(m_mask!=0 && (m_mask & (status >> 15)) !=0){
      this->setFilterPassed(false);
      m_rejected++;
    }

    if((results->status & DFC_M_STATUS_TKR_LT_2_ELO) != 0)
      m_vetoBits[0]++;
    if((results->status & DFC_M_STATUS_TKR_SKIRT) != 0)
      m_vetoBits[1]++;
    if((results->status & DFC_M_STATUS_TKR_EQ_0) != 0)
      m_vetoBits[2]++;
    if((results->status & DFC_M_STATUS_TKR_ROW2) != 0)
      m_vetoBits[3]++;
    if((results->status & DFC_M_STATUS_TKR_ROW01) != 0)
      m_vetoBits[4]++;
    if((results->status & DFC_M_STATUS_TKR_TOP) != 0)
      m_vetoBits[5]++;
    if((results->status & DFC_M_STATUS_ZBOTTOM) != 0)
      m_vetoBits[6]++;
    if((results->status & DFC_M_STATUS_EL0_ETOT_90) != 0)
      m_vetoBits[7]++;
    if((results->status & DFC_M_STATUS_EL0_ETOT_01) != 0)
      m_vetoBits[8]++;
    if((results->status & DFC_M_STATUS_SIDE) != 0)
      m_vetoBits[9]++;
    if((results->status & DFC_M_STATUS_TOP) != 0)
      m_vetoBits[10]++;
    if((results->status & DFC_M_STATUS_SPLASH_1) != 0)
      m_vetoBits[11]++;
    if((results->status & DFC_M_STATUS_E350_FILTER_TILE) != 0)
      m_vetoBits[12]++;
    if((results->status & DFC_M_STATUS_E0_TILE) != 0)
      m_vetoBits[13]++;
    if((results->status & DFC_M_STATUS_SPLASH_0) != 0)
      m_vetoBits[14]++;
    if((results->status & DFC_M_STATUS_NOCALLO_FILTER_TILE) != 0)
      m_vetoBits[15]++;
    if((results->status & DFC_M_STATUS_VETOED) != 0)
      m_vetoBits[16]++;
    
    newStatus->set(results->status);
    newStatus->setCalEnergy(results->energy);
    newStatus->setTcids(TDS_variables.tcids);
    newStatus->setAcdMap(TDS_variables.acd_xz,TDS_variables.acd_yz,TDS_variables.acd_xy);
    newStatus->setLayerEnergy(TDS_variables.layerEnergy);
    newStatus->setCapture(TDS_variables.xcapture,TDS_variables.ycapture);
    
    newStatus->setXY(TDS_variables.xy00, TDS_variables.xy11, TDS_variables.xy22, TDS_variables.xy33);
    OnboardFilterTds::projections prjs;
    for(int counter=0;counter<16;counter++){
      newStatus->setAcdStatus(counter,TDS_variables.acdStatus[counter]);
      prjs.xy[counter][0]=TDS_variables.xCnt[counter];
      prjs.xy[counter][1]=TDS_variables.yCnt[counter];
      prjs.curCnt[counter]=prjs.xy[counter][0]+prjs.xy[counter][1];
      for(int prjCounter=0;prjCounter<1000;prjCounter++){
	prjs.prjs[prjCounter].layers=TDS_variables.prjs.prjs[prjCounter].layers;
	prjs.prjs[prjCounter].min   =TDS_variables.prjs.prjs[prjCounter].min;
	prjs.prjs[prjCounter].max   =TDS_variables.prjs.prjs[prjCounter].max;
	prjs.prjs[prjCounter].nhits =TDS_variables.prjs.prjs[prjCounter].nhits;
	for(int hitCounter=0;hitCounter<18;hitCounter++)
	  prjs.prjs[prjCounter].hits[hitCounter]=TDS_variables.prjs.prjs[prjCounter].hits[hitCounter];
      }
    }
    newStatus->setProjection(prjs);
    newStatus->setLayers(TDS_layers);
    newStatus->setTmsk(TDS_variables.tmsk);
    storeHits(hits);
    log<<MSG::DEBUG;
    if(log.isActive()){
      log.stream()<< "FilterStauts Code: " << std::setbase(16) << (unsigned int)results->status<<" : "
		  << convertBase(results->status);
    }
    log << endreq;
    
    free_em ((void *)results);
    free_em ((void *)dfcCtl);
    free_em ((void *)dfcEvt);
    EBF_ofree (ebo);    
    EBF_free(ebf);
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

/* ---------------------------------------------------------------------- */
/*!

  \fn          int countEvts (const unsigned int *evts, int size)
  \param evts  Pointer to the list of events.
  \param size  Total size, in bytes of the list of events.
  \return      The number of events in the event list.
                                                                          */
/* ---------------------------------------------------------------------- */  
int OnboardFilter::countEvts (const unsigned int *evts, int size)
{
    int nevts = 0;

    while (size > 0) 
    {
        int esize = *evts;
        size     -= esize;
        nevts    += 1;

        evts = (unsigned int *)_ADVANCE (evts, esize);
    }
    
    return nevts;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn      const unsigned int *setupEvts (const unsigned int   *evt,
                                          int               totsize,
					  int               maxevts,
                                          int                 nskip,
					  int                *nevts,
					  unsigned int        *size)
  \brief  Skips to the first event to be output
  \param  evt      The beginning of the events
  \param  totsize  The total size of all the events (in bytes)  
  \param  maxevts  The maximum number of events to process, if -1, do all
  \param  nskip    The number of events to skip
  \param  nevts    Returned as the number of events to process
  \param  size     Returned as the number of bytes in the events to
                   process.
  \return          A pointer to the first event to be output
                                                                          */
/* ---------------------------------------------------------------------- */
const unsigned int *OnboardFilter::setupEvts (const unsigned int  *evt,
                                      int              totsize,
	 		              int              maxevts,
                                      int                nskip,
				      int               *nevts,
				      unsigned int       *size)

{
   const unsigned int *begevt;
   int                   ievt;


   /* Skip the request number of events */
   ievt = nskip;
   while (--ievt >= 0)
   {
      int esize = *evt;
      totsize -= esize;

      /* Make sure there are this many events */
      if (totsize < 0)
      {
         printf ("Error: Cannot skip %d events\n"
                 "File does not contain that many events\n",
                 nskip);
         exit (-1);
      }

      evt = (unsigned int *)_ADVANCE (evt, esize);
   }


   begevt = evt;
   ievt   = maxevts;

   /* Size the events */
   if (maxevts < 0)
   {
     maxevts = 0;

      /* Just process what's there */
     while (totsize > 0)
     {
       int esize = *evt;
       totsize  -=  esize;
       maxevts  +=  1;
       evt       = (unsigned int *)_ADVANCE (evt, esize);

     }
      
   }
   else
   {
     /* Size the events, but make sure there are enough */
     while (--ievt >= 0)
     {
       int esize = *evt;
       totsize  -=  esize;

       if (totsize < 0)
       {
	 printf (
	 "Error: Not enought events in the file to skip %d and process %d\n",
	 nskip,
	 maxevts);

	 exit (-1);
       }

       evt = (unsigned int *)_ADVANCE (evt, esize);
     }
   }
     
   *nevts = maxevts;
   *size  = (const char *)evt - (const char *)begevt;
   
   return begevt;
   
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- */
/*!

  \fn     static const struct _DFC_ctl *dfcCtlConstruct (TFC_geoId geoId,
                                                         int    geoPrint)
  \brief  Helper function to allocate and initialize the static, readonly
          filtering control structure.
  \return If successful, pointer to the initialized control structure,
          if unsuccessful, NULL.

  \param     geoId The id of the geometry to use. 
  \param  geoPrint Simply flag indicating whether to print a display of
                   the geometry.
                                                                          */
/* ---------------------------------------------------------------------- */  
const struct _DFC_ctl *OnboardFilter::dfcCtlConstruct (TFC_geoId geoId,  
                                               int    geoPrint)
{
    struct _DFC_ctl         *dfcCtl;
    const struct _TFC_geometry *geo;


    /* Find the specified geometry */
    geo = locateGeo (geoId, geoPrint);
 
    /* 
     | Allocate and initialize the static control structure. This includes
     | calibration constants and those types of things.
    */
    dfcCtl = (struct _DFC_ctl *)allocate (DFC_ctlSizeof(),"control structure");

    if (dfcCtl) DFC_ctlInit (dfcCtl, geo);

    return dfcCtl;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

  \fn     static const struct _DFC_ctl *dfcEvtConstruct (void)
  \brief  Helper function to allocate and initialize the event structure.
  \return If successful, pointer to the initialize event structure,
          if unsuccessful, NULL.

   The event structure is provides the temporary storage used on an
   event-by-event basis. If this was code was not running on an embedded
   system, this memory would be provided by each routine on the stack.
   But without automatic stack expansion, guessing at the stack size is
   potentially dangerous so this method is used.

   It has the nice side effect that it can be used to 'look' at the 
   internal filtering products.
                                                                          */
/* ---------------------------------------------------------------------- */
struct _DFC_evt *OnboardFilter::dfcEvtConstruct (void)
{
    struct _DFC_evt *dfcEvt;

    /* 
     | Allocate and initialize enough memory to hold an unpacked LAT event.
     | Normally this kind of thing would be allocated on the stack, but we
     | don't want to do that for the embedded systems where there is no
     | concept of automatic stack expansion.
    */
    dfcEvt   = (struct _DFC_evt *)allocate (DFC_evtSizeof (),
					    "lat event record");
    if (dfcEvt) DFC_evtInit (dfcEvt);
    return dfcEvt;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

  \fn static int filterEvts (const unsigned int         *evt,
			     int                     nbegevt,
			     int                     nendevt,
			     int                      nprint,
			     unsigned int        ss_to_print,
			     const struct _DFC_ctl   *dfcCtl,
			     struct _DFC_results    *results,
			     int                 resultsSize,
			     struct _DFC_evt         *dfcEvt,
			     unsigned int             vetoes)
  \brief  Calls the filter function for each of the specified events
  \return Status

  \param         evt Pointer to the events
  \param     nbegevt Sequence number of the first event
  \param     nendevt Sequence number of the  last event 
  \param      nprint The number of events to print a diagnostic display for
  \param ss_to_print A mask representing the subsystems to display
  \param      dfcCtl Structure representing the static, readonly
                     parameters governing the filtering code
  \param     results An array of results vectors, one for each event to
                     be filtered. This gives a brief summary for each
                     event.
  \param resultsSize The size, in bytes, of one results vector 
  \param      dfcEvt Structure providing memory for all the dynamic 
                     storage needs to filter one event. Normally this 
                     kind of memory would be provided on the stack, but
                     in an embedded environment, there is no automatic
                     stack expansion and guessing at a stack size is
                     potentially dangerous.
  \param      vetoes A bit mask of conditions. If any condition is 
                     satisfied, the filter ceases processing  
                                                                          */
/* ---------------------------------------------------------------------- */
int OnboardFilter::filterEvts (const unsigned int         *evt,
		       int                      nbegevt,
		       int                      nendevt,
		       int                       nprint,
		       unsigned int         ss_to_print,
		       const struct _DFC_ctl    *dfcCtl,
		       struct _DFC_results     *results,
		       int                  resultsSize,
		       struct _DFC_evt          *dfcEvt,
		       unsigned int              vetoes)
{
    int ievt;
    for (ievt = nbegevt; ievt < nendevt; ievt++)
    {
        int status;
        int  esize;


	/* Indicate the event being processed */
        EDM_DEBUGPRINTF((Filter_edm,
			"GLASTSIM = %9d\n",
			getMCsequence (evt, *evt)));


        /* Perform any pre-filter diagnostic print-out of the event */
        if (nprint > 0)
        {
            DFC_latRecordUnpack (&dfcEvt->dlr, evt, dfcCtl);
            DFC_display         (&dfcEvt->dlr, ss_to_print);
        }


	/* Do the actual filtering */
        esize   = *evt;
        status  = DFC_filter (results, dfcCtl, dfcEvt, evt, esize, vetoes);


	/* Perform any post-filter diagnostic print-out */
	if (nprint > 0)
	{
            nprint -= 1;
            if (  (status      & DFC_M_STATUS_TKR           ) && 
                 ((status      & DFC_M_STATUS_TKR_EQ_0) == 0) &&
                  (ss_to_print & FLT_M_PRJS)                     ) 
                 TFC_projectionsPrint (&dfcEvt->prjs, -1);
	}


	/* Print out a list of events escaping the filter */
        EDM_CODE(if ((status & DFC_M_STATUS_VETOES) == 0)
	         {
		   EDM_INFOPRINTF ((
                    Filter_edm,
		   "GLASTSIM EVENT = %9d (%9d) NOT REJECTED (%8.8x)\n",
		    getMCsequence(evt, esize), ievt, status));
                 });

        

	/* Next event */
        evt     = (unsigned int *)_ADVANCE (evt, esize);
        results = (struct _DFC_results *)((unsigned char *)results
                                                         + resultsSize);
    }

    return 0;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- */
/*!

  \fn static int filterCompleteEvts (const unsigned int         *evt,
			             int                     nbegevt,
                		     int                     nendevt,
			             EBF_ofile                  *ebo,
			             const struct _DFC_ctl   *dfcCtl,
			             struct _DFC_results    *results,
			             int                 resultsSize,
			             struct _DFC_evt         *dfcEvt,
			             unsigned int         listPasses)
  \brief  Completes the filter function for each of the specified events
          and, potentially writes an output file of events that pass
  \return Status

  \param         evt Pointer to the events
  \param     nbegevt Sequence number of the first event
  \param     nendevt Sequence number of the  last event 
  \param         ebo An EBF output file handle. If NULL, no output file
                     is written
  \param      dfcCtl Structure representing the static, readonly
                     parameters governing the filtering code
  \param     results An array of results vectors, one for each event to
                     be filtered. This gives a brief summary for each
                     event.
  \param resultsSize The size, in bytes, of one results vector 
  \param      dfcEvt Structure providing memory for all the dynamic 
                     storage needs to filter one event. Normally this 
                     kind of memory would be provided on the stack, but
                     in an embedded environment, there is no automatic
                     stack expansion and guessing at a stack size is
                     potentially dangerous.
  \param  listPasses Simple flag indicating whether to print those events
                     that pass the filter.
                                                                          */
/* ---------------------------------------------------------------------- */
int OnboardFilter::filterCompleteEvts (const unsigned int         *evt,
			       int                     nbegevt,
			       int                     nendevt,
			       EBF_ofile                  *ebo,
			       const struct _DFC_ctl   *dfcCtl,
			       struct _DFC_results    *results,
			       int                 resultsSize,
			       struct _DFC_evt         *dfcEvt,
			       unsigned int         listPasses)
{
    int ievt;
				   
    for (ievt = nbegevt; ievt < nendevt; ievt++)
    {
      int   size = *evt;
      int   status;
      
      status = DFC_filterComplete (results, dfcCtl, dfcEvt, evt, size);


      /* Writing an output file ? */
      if ( ebo && (status & DFC_M_STATUS_VETOES) == 0)
      {
            EBF_write (ebo, evt, size);
      }
                        
        
      if (listPasses && (status & DFC_M_STATUS_VETOES) == 0)
      {
	printf ("0000 %8d %8d\n", getMCsequence (evt, size), ievt);
      }

      results = (DFC_results *)_ADVANCE (results, resultsSize);
      evt     = (unsigned int *)_ADVANCE (evt, size);
        
    }

    return 0;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

  \fn     const struct _TFC_geometry *locateGeo (int id, int printIt)
  \brief  Locates the specified geometry within the list of available
          geometries.
  \return A pointer to the specified geometry.

  \param      id  The id of the specified geometry.
  \param printIt  If non-zero, prints the geometry.

   If the specified geometry is not located, a list of the available 
   geometries is printed and the program exits
									  */
/* ---------------------------------------------------------------------- */
const struct _TFC_geometry *OnboardFilter::locateGeo (int id, int printIt)
{
    int                         idx;
    const struct _TFC_geometry *geo;

    /* Lookup the specified geometry */
    geo = TFC_geosLocate (TFC_Geos, -1, id);

    /* If can't find the specified geometry, report error and quit */
    if (!geo) 
    {
       printf ("ERROR: Unable to locate the specified geometry, id = %d\n"
	       "       The available choices are\n\n");

       /* Print just a header line */
       //TFC_geometryPrint (NULL, TFC_M_GEO_OPTS_TAG_HDR);

       /* Print the tag for each */
       idx = 0;
       while ((geo = TFC_Geos[idx++])) 
       {
	 //TFC_geometryPrint (geo, TFC_M_GEO_OPTS_TAG);
       }

       exit (-1);
    }


    /* Print the detector geometry if desired */
    //TFC_geometryPrint (geo, 
    //                   printIt ? TFC_M_GEO_OPTS_ALL 
    //                           : TFC_M_GEO_OPTS_TAG_HDR | TFC_M_GEO_OPTS_TAG);
 
    return geo;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- */
/*!

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




/* ---------------------------------------------------------------------- */
/*!

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
/*!

  \fn     EBF_ofile *createOutputFile (const char *name)
  \brief  Creates an output file if \a name is not NULL
  \return A handle to write to if name is not NULL, else NULL

  \param  name The name of the output file to create
									  */
/* ---------------------------------------------------------------------- */
EBF_ofile *OnboardFilter::createOutputFile (const char *name)
{
    EBF_ofile *ebo;
 
    ebo = EBF_create (name);
    if (ebo == 0)
    {
      printf (" Error %d creating output file: %s\n", errno, name);
      return 0;
    }
    else
    {
      printf ("Output file: %s\n", name);
    }

    return ebo;
}
/* ---------------------------------------------------------------------- */




#if EDM_USE
/* ---------------------------------------------------------------------- */
/*!

  \fn     static void setMessageLevels (MessageObjLevels lvl)
  \brief  Sets the diagnostic message level for each object
 
  \param  lvl  The message levels for each object.

                                                                          */
/* ---------------------------------------------------------------------- */
void OnboardFilter::setMessageLevels (MessageObjLevels lvl)
{
  extern EDM_level              DFC_Filter_edm;
  extern EDM_level                 TFC_Acd_edm;
  extern EDM_level               TFC_Skirt_edm;
  extern EDM_level TFC_ProjectionTowerFind_edm;

  Filter_edm                  = lvl.filter;
  DFC_Filter_edm              = lvl.dfc_filter;
  TFC_ProjectionTowerFind_edm = lvl.tfc_ptf;
  TFC_Acd_edm                 = lvl.tfc_acd;
  TFC_Skirt_edm               = lvl.tfc_skirt;

  return;
}
#endif



/* ---------------------------------------------------------------------- */
/*!

  \fn  void resultsPrint (TMR_tick                       beg, 
			  TMR_tick                       end,
			  const struct _DFC_results *results,
			  int                       nresults,
			  unsigned int               options)
  \brief Prints the results vector or a summary thereof.

  \param      beg  The beginning time, in TMR_tick  
  \param      end  The ending time,    in TMR_tick 
  \param  results  The results vector
  \param nresults  The number of results vectors and, by implication,
                   the number of events processed
  \param  options  An bit mask of options passed to DFC_resultsPrint
                                                                          */
/* ---------------------------------------------------------------------- */
void OnboardFilter::resultsPrint (TMR_tick                       beg, 
			  TMR_tick                       end,
			  const struct _DFC_results *results,
			  int                       nresults,
			  unsigned int               options)
{
   unsigned int      eticks=0;
   TMR_usecs_nsecs  elapsed;  /* Total elapsed time                       */
   TMR_usecs_nsecs nelapsed;  /* Normalized (by event count) elapsed time */


   /* Print the results summary */    
   //DFC_resultsPrint (results,  nresults, options);


   eticks   = TMR_DELTA_IN_NSECS (beg, end);
   //elapsed  = TMR_ticks_to_usecs_nsecs (eticks);
   //nelapsed = TMR_ticks_to_usecs_nsecs ((eticks   + nresults / 2)  / 
   //                                     (nresults ? nresults : 1));

   printf ("Elapsed Time: %6u.%03d / %5d = %6u.%03d usecs\n",
	   elapsed.usecs, elapsed.nsecs,
	   nresults,
	   nelapsed.usecs, nelapsed.nsecs);

   return;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

  \fn    void statisticsPrint (const struct _DFC_results *results,
                               int                       nresults)
  \brief Prints the summary statistics

  \param  results The vector of results from which the statistics are
                  made.
  \param nresults The number of results vectors
                                                                          */
/* ---------------------------------------------------------------------- */
void OnboardFilter::statisticsPrint (const struct _DFC_results *results,
			     int                       nresults)
{
   DFC_statistics             statistics;

   DFC_statisticsClear      (&statistics);
   DFC_statisticsAccumulate (&statistics, results, nresults);
   DFC_statisticsPrint      (&statistics);

   return;
}
/* ---------------------------------------------------------------------- */

std::string OnboardFilter::convertBase(unsigned int number){
    std::string output;
    int count=1;
    do{
        if(number%2)
            output = "1" + output;
        else
            output = "0" + output;
        number/=2;
        if(!(count%4))
            output = " " + output;
        count++;
    }while(number);
    for(int counter=count;counter<=32;counter++){
        output = "0" + output;
        if(!(counter%4))
            output = " " + output;
    }
    return output;
}

StatusCode OnboardFilter::finalize(){
	using namespace std;
    MsgStream log(msgSvc(),name());
    log  << MSG::INFO << "Rejected " << m_rejected << " triggers using mask 0x" << std::hex << m_mask << std::dec << endreq;
    log << MSG::INFO;
    if(log.isActive()){
      log.stream() << "Veto Bit Summary" << std::endl << setw(35) << "Trigger Name" << setw(10) << "Count" << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_TKR_LT_2_ELO" << setw(10) << m_vetoBits[0] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_TKR_SKIRT" << setw(10) << m_vetoBits[1] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_TKR_EQ_0" << setw(10) << m_vetoBits[2] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_TKR_ROW2" << setw(10) << m_vetoBits[3] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_TKR_ROW01" << setw(10) << m_vetoBits[4] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_TKR_TOP" << setw(10) << m_vetoBits[5] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_ZBOTTOM" << setw(10) << m_vetoBits[6] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_EL0_ETOT_90" << setw(10) << m_vetoBits[7] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_EL0_ETOT_01" << setw(10) << m_vetoBits[8] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_SIDE" << setw(10) << m_vetoBits[9] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_TOP" << setw(10) << m_vetoBits[10] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_SPLASH_1" << setw(10) << m_vetoBits[11] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_E350_FILTER_TILE" << setw(10) << m_vetoBits[12] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_E0_TILE" << setw(10) << m_vetoBits[13] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_SPLASH_0" << setw(10) << m_vetoBits[14] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_NOCALLO_FILTER_TILE" << setw(10) << m_vetoBits[15] << std::endl;
      log.stream() << setw(35) << "DFC_M_STATUS_VETOED" << setw(10) << m_vetoBits[16] << std::endl;
    }
    log << endreq;
  return StatusCode::SUCCESS;
}
