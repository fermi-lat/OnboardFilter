/* ---------------------------------------------------------------------- *//*!

   \file  OnboardFilter.cxx
   \brief  run filter
   \author JJRussell - russell@slac.stanford.edu

\verbatim

  CVS $Id: OnboardFilter.cxx,v 1.78 2008/04/09 20:40:21 usher Exp $
\endverbatim
                                                                          */
/* ---------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
 
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"

#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Property.h"
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"
#include "facilities/Util.h"

#include "Event/TopLevel/EventModel.h"
#include "EbfWriter/Ebf.h"
//#include "OnboardFilterTds/FilterStatus.h"
#include "OnboardFilterTds/ObfFilterStatus.h"

#include "ObfInterface.h"
#include "IFilterTool.h"

class OnboardFilter:public Algorithm
{
public:
    // Standard constructor
    OnboardFilter(const std::string& name, ISvcLocator *pSvcLocator);
    // virtual destructor
    ~OnboardFilter() {}

    // The three methods which do the work
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

private:

    /* ====================================================================== */
    /* Member variables                                                       */
    /* ====================================================================== */

    // Filters to run 
    bool         m_passThrough;

    bool         m_rejectEvents;    // Enables rejection of events from list of active filters
    unsigned     m_gamBitsToIgnore; // This sets a mask of gamma filter veto bits to ignore
    int          m_rejected;
    int          m_noEbfData;
    bool         m_failNoEbfData;

    ObfInterface* m_obfInterface;

    // Filters to run
    StringArrayProperty m_filterList;

    // The list of filters to use to reject events
    //typedef std::list<OnboardFilterTds::IObfStatus::FilterKeys> FilterList;
    typedef std::vector<unsigned int> FilterRejectList;
    FilterRejectList  m_filterRejectList;
};



static const AlgFactory<OnboardFilter> Factory;
const IAlgFactory& OnboardFilterFactory = Factory;
//FilterInfo OnboardFilter::myFilterInfo;

OnboardFilter::OnboardFilter(const std::string& name, ISvcLocator *pSvcLocator) : Algorithm(name,pSvcLocator), 
          m_rejected(0), m_noEbfData(0)
{

    // Properties for this algorithm
    declareProperty("RejectEvents",     m_rejectEvents       = false);
    declareProperty("PassThrough",      m_passThrough        = true);
    declareProperty("FailNoEbfData",    m_failNoEbfData      = false);
    declareProperty("FilterRejectList", m_filterRejectList);
    declareProperty("FilterList",       m_filterList);

    // Set up default filter configuration
    std::vector<std::string> filterList;
    filterList.push_back("GammaFilter");
    filterList.push_back("MIPFilter");
    filterList.push_back("HIPFilter");
    filterList.push_back("DGNFilter");
    filterList.push_back("FilterTrack");
    //filterList.push_back("TkrOutput");
    //filterList.push_back("CalOutput");
    //filterList.push_back("GemOutput");
    m_filterList = filterList;

    // Set up the default to filter on Gamma and HFC filters
    m_filterRejectList.clear();
    m_filterRejectList.push_back(OnboardFilterTds::ObfFilterStatus::GammaFilter);
    m_filterRejectList.push_back(OnboardFilterTds::ObfFilterStatus::HFCFilter);
}
/* --------------------------------------------------------------------- */


StatusCode OnboardFilter::initialize()
{
    // Some shenanigans to get a persistent stream reference to pass to obfInterface...
    MsgStream* logPtr = new MsgStream(msgSvc(), name());
    MsgStream& log   = (*logPtr);

    setProperties();

    log << MSG::INFO << "Initializing Filter Settings" << endreq;

    // Get an instance of the filter interface
    m_obfInterface = ObfInterface::instance();

    // Retrieve (and initialize) the FSWAuxLibsTool which will load pedestal, gain and geometry libraries
    IFilterTool* toolPtr = 0;
    if (StatusCode sc = toolSvc()->retrieveTool("FSWAuxLibsTool", toolPtr) == StatusCode::FAILURE)
    {
        log << MSG::ERROR << "Failed to load the FSW Auxiliary libraries" << endreq;
        return sc;
    }

    // Loop through the list of filters to configure
    int                             nFilters   = 0;
    const std::vector<std::string>& filterList = m_filterList;
    for(std::vector<std::string>::const_iterator filterIter = filterList.begin(); filterIter != filterList.end(); filterIter++)
    {
        std::string filterTool = *filterIter + "Tool";

        if (StatusCode sc = toolSvc()->retrieveTool(filterTool, toolPtr) == StatusCode::FAILURE)
        {
            log << MSG::ERROR << "Failed to initialize the " << *filterIter << " tool" << endreq;
            return sc;
        }

        // Dump the initialized configuration
        toolPtr->dumpConfiguration();

        nFilters++;
    }

    // Set up a "passthrough" filter which allows us to always retrieve results from 
    // the filters after they have run
    if (!m_obfInterface->setupPassThrough(0))
    {
        log << MSG::ERROR << "Failed to initialize pass through Filter" << endreq;
    }
  
    return StatusCode::SUCCESS;
}

StatusCode OnboardFilter::execute()
{
    MsgStream log(msgSvc(), name());
    
    // Check for ebf on tds
    SmartDataPtr<EbfWriterTds::Ebf> ebfData(eventSvc(),"/Event/Filter/Ebf");
    if(!ebfData)
    {
        log << MSG::DEBUG << "No ebf data found "<<endreq;
        
        // If no ebf data then no point continuing on 
        if (m_failNoEbfData) this->setFilterPassed(false);
        m_noEbfData++;

        return StatusCode::SUCCESS;
    }

    //  Make the tds objects
    //OnboardFilterTds::FilterStatus *newStatus=new OnboardFilterTds::FilterStatus;
    //eventSvc()->registerObject("/Event/Filter/FilterStatus",newStatus);

    OnboardFilterTds::ObfFilterStatus *obfStatus=new OnboardFilterTds::ObfFilterStatus;
    eventSvc()->registerObject("/Event/Filter/ObfFilterStatus",obfStatus);

    try
    {
        // Call the filter
        unsigned int fate = m_obfInterface->filterEvent(ebfData);

        if (fate != 4)
        {
            log << MSG::ERROR << "Error in filter processing, fate = " << fate << endreq;
        }
    }
    catch(ObfInterface::ObfException& obfException)
    {
        log << MSG::INFO << obfException.m_what << endreq;
    }

    // Check to see if we are vetoing events at this stage
    if (m_rejectEvents)
    {
        bool rejectEvent = true;

        // Loop through the list of filters to apply
        for(FilterRejectList::iterator listItr = m_filterRejectList.begin(); listItr != m_filterRejectList.end(); listItr++)
        {
            // Retrieve enum
            OnboardFilterTds::ObfFilterStatus::FilterKeys key = (OnboardFilterTds::ObfFilterStatus::FilterKeys)(*listItr);

            // Look up the information for this filter
            const OnboardFilterTds::IObfStatus* filterStat = obfStatus->getFilterStatus(key);

            // Make sure the filter ran
            if (!filterStat) continue;

            // Look at sb to determine how to handle the event
            unsigned char sb = filterStat->getFiltersb();

            // Two cases: event was accepted and no prescale or event was rejected and prescale
            sb = (sb & (EDS_RSD_SB_M_VETOED | EDS_RSD_SB_M_PRESCALE_OUT)) >> EDS_RSD_SB_V_PRESCALE_OUT;

            if (  (sb == 0)   // Event accepted and prescale does not flip the decision
               || (sb == 3) ) // Event rejected and prescale flips the decision (making it accepted)
            {
                rejectEvent = false;
                break;
            }
        }

        // High order bit set means we reject events, at this point combStatus would be non-zero
        if (rejectEvent)
        {
            this->setFilterPassed(false);
            m_rejected++;
        }
    }

    return StatusCode::SUCCESS;
}

/* --------------------------------------------------------------------- */

StatusCode OnboardFilter::finalize()
{
    m_obfInterface->dumpCounters();

    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "Encountered " << m_noEbfData << " events with no ebf data"
        << endreq;
    if (m_rejectEvents) log << MSG::INFO << "Rejected " << m_rejected << endreq;

    return StatusCode::SUCCESS;
}

