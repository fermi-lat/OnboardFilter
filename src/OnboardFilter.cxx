/* ---------------------------------------------------------------------- *//*!
   
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

#include "DFC/EBF_fileIn-Gleam.h"
#include "DFC/EBF_fileOut.h"
#include "DFC/DFC_ss.h"
#include "DFC/DFC_ctl.h"
#include "DFC/DFC_ctlSysDef.h"
#include "DFC/DFC_results.h"
#include "DFC/DFC_status.h"
#include "DFC/DFC_statistics.h"
#include "DFC/DFC_filter.h"
#include "DFC/DFC_latRecord.h"
#include "DFC/DFC_latUnpack.h"


#include "DFC/DFC_display.h"
#include "DFC/TFC_geometryPrint.h"
#include "DFC_resultsDef.h"

#include "DFC_ctlCfcDef.h"
#include "DFC/CFC_constants.h"

// Gaudi system includes
#include "GaudiKernel/Algorithm.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

#include "Event/TopLevel/EventModel.h"
#include "EbfWriter/Ebf.h"
#include "OnboardFilter/FilterStatus.h"

#include "ntupleWriterSvc/INTupleWriterSvc.h"

#include <string.h>
#ifdef   __linux
#include <getopt.h>
#endif


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
         (typeof (_ptr))((unsigned char *)(_ptr) + (_nbytes))
/* --------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \def    _DBG(statement)
  \brief  Simple macro to drop the specified statement into the code. This
          macro will only drop the statement if DEBUG is defined, otherwise
          this macro is a NOOP.
                                                                          */
/* ---------------------------------------------------------------------- */  
#ifdef DEBUG
#include <stdio.h>
#define _DBG(statement) statement
#else
#define _DBG(statement)
#endif
/* ---------------------------------------------------------------------- */



//#include "tmr.h"

/* ---------------------------------------------------------------------- *//*!
  \var   TFC_GeometryStd
  \brief Data structure defining the standard geometry used in the
         filtering process.
                                                                          */
/* ---------------------------------------------------------------------- */
struct _TFC_geometry;
extern const struct _TFC_geometry TFC_GeometryStd;
/* ---------------------------------------------------------------------- */

   
    
/* ---------------------------------------------------------------------- *//*!
  \struct _Ctl
  \brief   Collections all the control parameters together. These are
           essentially the interpretted command line options.
                                                                          *//*!
  \typedef Ctl
  \brief   Typedef for struct _Ctl
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _Ctl
{
    const char  *ifile;  /*!< The name of the input file (Evt Bld  format)*/
    const char  *ofile;  /*!< The name of the output file (optional)      */
    unsigned int *data;  /*!< Pointer to a EBD data structure             */
    int     to_process;  /*!< The number of events to process.            */
    int        to_skip;  /*!< The number of events to initially skip.     */
    int       to_print;  /*!< The number of events to print.              */
    int    ss_to_print;  /*!< The mask of which subsystems to print       */
    int          quiet;  /*!< Minimal summary information                 */
    int           list;  /*!< Print list of events not rejected           */
    int       esummary;  /*!< Event results summary flags                 */
    int       geometry;  /*!< Print the detector geometry                 */
}
Ctl;
/* ---------------------------------------------------------------------- */


class OnboardFilter:public Algorithm{
public:
  OnboardFilter(const std::string& name, ISvcLocator* pSvcLocator);

  //static void print_ctl (const Ctl *ctl);
  int countEvts  (const unsigned int *evts, int size);
  /* ---------------------------------------------------------------------- *//*!
   \fn          int getMCsequence (const unsigned int *evt, int esize)
   \brief       Retrieves the GLEAM Monte Carlo Event Sequence Number
   \param   evt Pointer to the event
   \param esize The event size
   \return      The GLEAM Monte Carlo Event Sequence Number.
                                                                            */
  /* ---------------------------------------------------------------------- */
  static inline int getMCsequence (const unsigned int *evt, int esize){
    return evt[0x10];
  }
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
private:
  std::string convertBase(unsigned int number);
  Ctl ctl_buf;
  Ctl *ctl;
};

static const AlgFactory<OnboardFilter>  Factory;
const IAlgFactory& OnboardFilterFactory = Factory;

StatusCode OnboardFilter::finalize(){
  return StatusCode::SUCCESS;
}

OnboardFilter::OnboardFilter(const std::string& name, ISvcLocator* pSvcLocator):Algorithm(name, pSvcLocator){
}

/* ---------------------------------------------------------------------- *//*!

  \fn      main (int argc, char **argv)
  \brief   Entry point for filter
  \param argc Standard argument count.
  \param argv Standard vector of command line parameter strings.
  \retval     0, if successful
             -1, if the file does not exist or some other internal error.

   The usage of this routine is self-documenting. Simply type filter
   with no parameters for its usage.
                                                                          */
/* ---------------------------------------------------------------------- */  
StatusCode OnboardFilter::initialize()
{
  MsgStream log(msgSvc(),name());
  log<< MSG::INFO << "Initializing Filter Settings"<<endreq;
  ctl_buf.ifile="test.ebf";
  ctl_buf.ofile=NULL;
  ctl_buf.data=NULL;
  ctl_buf.to_process=1;
  ctl_buf.to_skip=0;
  ctl_buf.to_print=0;
  ctl_buf.ss_to_print=0;
  ctl_buf.quiet=1;
  ctl_buf.list=0;
  ctl_buf.esummary=0;
  ctl_buf.geometry=0;
  ctl    = &ctl_buf;
  return StatusCode::SUCCESS;
}
/* ---------------------------------------------------------------------- */


   

/* ---------------------------------------------------------------------- *//*!

   \fn         int doFilter (const Ctl *ctl)
   \brief      Common routine to do the filtering
   \param ctl  The control parameters, assumed to be already filled in
   \return     Status
                                                                          */
/* ---------------------------------------------------------------------- */

StatusCode OnboardFilter::execute()
{
    TMR_tick                  beg;
    TMR_tick                  end;
    unsigned int          elapsed;
    int                nprocessed;
    struct _DFC_ctl       *dfcCtl;
    int                   dfcSize;
    EBF_file                 *ebf;
    int                     nevts;
    int                    evtCnt;
    int                      size;
    const unsigned int       *evt;
    const unsigned int    *begevt;
    int                       idx;
    int                       iss;
    unsigned int           nprint;
    unsigned int            nskip;
    int                to_process;
    DFC_ctlSys            *dfcSys;
    int               resultsSize;
    struct _DFC_results  *results;
    struct _DFC_results   *result;
    DFC_statistics     statistics;
    struct _DFC_latRecord *dfcEvt;
    const struct _DFC_ctlCfc *cfc;
    
    MsgStream log(msgSvc(),name());
    Event::FilterStatus *newStatus=new Event::FilterStatus;
    eventSvc()->registerObject(EventModel::Filter::FilterStatus,newStatus);

    log << MSG::INFO << "Processing Event"<<endreq;
    /* Initialize the time base */
    TMR_initialize ();

    dfcSize = DFC_ctlSizeof ();
    dfcCtl  = (struct _DFC_ctl *)malloc (dfcSize);
    if (dfcCtl == NULL)
    {
        printf ("Cannot allocate %8.8x(%d) bytes for control structure\n",
                dfcSize, dfcSize);
        return StatusCode::FAILURE;
    }
    DFC_ctlInit (dfcCtl);


    dfcSize  = DFC_latRecordSizeof ();
    dfcEvt   = (struct _DFC_latRecord *)malloc (dfcSize);
    if (dfcEvt == NULL)
    {
        printf ("Cannot allocate %8.8x(%d) bytes for DFC unpack record\n",
                dfcSize, dfcSize);
        return StatusCode::FAILURE;
    }
    DFC_latRecordInit (dfcEvt);

    SmartDataPtr<EbfWriterTds::Ebf> ebfData(eventSvc(),EventModel::Filter::Ebf);
    if(!ebfData){
      return StatusCode::FAILURE;
    }
    unsigned int length;
    char *data=ebfData->get(length);
    ebf   = EBF_openGleam   (length);
    iss   = EBF_readGleam   (ebf,(unsigned int *)data);
    iss   = EBF_closeGleam  (ebf);
    size  = EBF_esizeGleam  (ebf);
    evt   = EBF_edataGleam  (ebf);
    nevts = countEvts       (evt, size);

    /* If number of events not specified, use all */
    to_process = ctl->to_process;
    if (to_process < 0) to_process = nevts;
    
    
    nskip       = ctl->to_skip;
    nprint      = ctl->to_print;
    to_process += nskip;
    nevts       = to_process > nevts ? nevts : to_process;

    
    /* Can't print more than there are */
    if (nskip  > nevts) nskip  = nevts;
    if (nprint > nevts) nprint = nevts;
    evtCnt = nevts - nskip;


    /* Allocate the memory for the results vector */
    resultsSize     = DFC_resultsSizeof ();
    results         = (struct _DFC_results *)malloc (resultsSize * evtCnt);
    if (results == NULL)
    {
        printf ("Cannot allocate %d bytes for % d result vectors\n",
                resultsSize * evtCnt, evtCnt);
        return StatusCode::FAILURE;
    }

    
    /* Skip the request number of events */
    for (idx = 0; idx < nskip; idx++)
    {
        int esize = *evt;
        size -= esize;
        //evt   = _ADVANCE (evt, esize);
	evt = (const unsigned int *)((char *)(evt)+esize);
    }
    

    begevt          = evt;
    result          = results;
    cfc             = DFC_ctlCfcLocate (dfcCtl);
    dfcSys          = DFC_ctlSysLocate (dfcCtl);
    dfcSys->toPrint = nprint;
    DFC_statisticsClear (&statistics);


    /* Invalidate the cache where the events are stored */
    //CACHE_invalidateData (evt, size);
    

       
    beg             = TMR_GET ();
    for (idx = nskip; idx < nevts; idx++)
    {
        int status;
        int  esize;

        _DBG (printf("GLASTSIM = %9d\n", mcsequence));
        
     
        /* Perform any diagnostic print-out of the event */
        if (dfcSys->toPrint > 0)
        {
            dfcSys->toPrint -= 1;
            DFC_latRecordUnpack (dfcEvt, evt, dfcCtl);
            DFC_display (dfcEvt);
        }

        
        esize   = *evt;
        status  = DFC_filter (dfcCtl, result, dfcEvt, evt, esize);

        if ((status & DFC_M_STATUS_VETOES) == 0)
            _DBG (printf("GLASTSIM EVENT = %9d (%9d) NOT REJECTED (%8.8x)\n",
                    getMCsequence(evt), idx, status));
        
        //evt     = _ADVANCE (evt, esize);
	evt = (const unsigned int *)((char *)(evt)+esize);
        result  = (struct _DFC_results *)((unsigned char *)result
                                                         + resultsSize);
    }
    end        = TMR_GET ();
    elapsed    = TMR_DELTA_IN_NSECS (beg, end);

    

    /* Ensure that the energy status is filled in */
    evt    = begevt;
    result = results;
    for (idx = nskip; idx < nevts; idx++)
    {
        int   size = *evt;
        int   status;
        
        
        DFC_filterComplete (dfcCtl, result, dfcEvt, evt, size);
        
        status = result->status;
	newStatus->set(status);

        if (ctl->list && (result->status & DFC_M_STATUS_VETOES) == 0)
        {
            printf ("0000 %8d %8d\n", getMCsequence (evt, size), idx);
        }

        //result  = _ADVANCE (result, resultsSize);
	result = (struct _DFC_results   *)((char *)(result)+resultsSize);
        //evt     = _ADVANCE (evt, size);
	evt = (const unsigned int *)((char *)(evt)+size);
        
    }
    
        
    nprocessed = nevts - nskip;
    DFC_resultsPrint (results, evtCnt, ctl->esummary);
    
    printf ("Elapsed Time: %10d / %5d = %7d nsecs\n",
            elapsed,
            nprocessed,
            elapsed/(nprocessed ? nprocessed : 1));
        
    if (ctl->quiet == 0)
    {
        DFC_statisticsAccumulate (&statistics, results, evtCnt);
        DFC_statisticsPrint      (&statistics);
    }
    
    

    EBF_free (ebf);
    free(results);
    free(dfcCtl);
    free(dfcEvt);
    return StatusCode::SUCCESS;
}
/* ---------------------------------------------------------------------- */



   
/* ---------------------------------------------------------------------- *//*!

  \fn        void print_ctl (const Ctl *ctl)
  \brief     Prints the contents of the print control context.
  \param ctl Pointer to the print control context block.
                                                                          */
/* ---------------------------------------------------------------------- */
/*
  static void OnboardFilter::print_ctl (const Ctl *ctl)
  {
  printf ("Input  file: %s\n",  ctl->ifile);
  
  if (ctl->to_process < 0) printf ("Processing : All\n");
  else                     printf ("Processing : %d\n", ctl->to_process);
  
  
  if (ctl->to_skip   == 0) printf ("Skipping   : None\n");
  else                     printf ("Skipping   : %d\n", ctl->to_skip);
  
  
  if (ctl->to_print  == 0) printf ("Printing   : None\n");
  else                     printf ("Printing   : %d\n", ctl->to_print);
  
  
  }
*/
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

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
        
        //evts = _ADVANCE (evts, esize);
	evts = (const unsigned int *)((char *)(evts)+esize);
    }
    
    return nevts;
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
