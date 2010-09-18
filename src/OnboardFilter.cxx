/* ---------------------------------------------------------------------- *//*!

   \file  OnboardFilter.cxx
   \brief  run filter
   \author JJRussell - russell@slac.stanford.edu

\verbatim

  CVS $Id: OnboardFilter.cxx,v 1.84 2008/08/01 04:01:15 usher Exp $
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

// Moot stuff for discerning filter configurations
#include "CalibData/Moot/MootData.h"
#include "MootSvc/IMootSvc.h"

// We will need to check the run mode event-by-event from the input data
#include "LdfEvent/LsfMetaEvent.h"

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
    /* Private methods                                                        */
    /* ====================================================================== */

    StatusCode initFilters();

    /* ====================================================================== */
    /* Member variables                                                       */
    /* ====================================================================== */

    // Start with the list of Job Options parameters
    // Are we expecting to use Moot to get filter configuration information?
    BooleanProperty m_mootConfig;

    // Various filtering parameters
    BooleanProperty m_passThrough;     // Set up the passthrough filter?
    BooleanProperty m_rejectEvents;    // Enables rejection of events from list of "active" filters
    IntegerProperty m_gamBitsToIgnore; // This sets a mask of gamma filter veto bits to ignore

    // Filters to configure and run, not necessarily the "active" filters...
    StringArrayProperty m_filterList;

    // "Active" Filters are those which participate in the decision to reject events
    typedef std::vector<unsigned int> ActiveFilterVec;
    ActiveFilterVec  m_activeFilters;

    // Set up some counters for keeping track of various things we might encounter
    int              m_events;          // # events run through filter
    int              m_rejected;        // # events rejected by filters
    int              m_noEbfData;       // # events with no ebf data (MC only? Trigger reject)
    bool             m_failNoEbfData;   // If we don't have ebf data should we crash?

    // Now to member variables
    // Pointer to the obf interface
    ObfInterface*    m_obfInterface;

    // Pointer to MootSvc
    IMootSvc*        m_mootSvc;

    // Map to relate filter schema ids to "our" filter tool names
    typedef std::map<unsigned int, std::string> IdToNameMap;
    IdToNameMap      m_idToToolNameMap;

    // Cache the "current mode" we are running
    enums::Lsf::Mode m_curMode;

    // Cache our initialization status
    bool             m_initialized;
};



//static const AlgFactory<OnboardFilter> Factory;
//const IAlgFactory& OnboardFilterFactory = Factory;
//FilterInfo OnboardFilter::myFilterInfo;

DECLARE_ALGORITHM_FACTORY(OnboardFilter);

OnboardFilter::OnboardFilter(const std::string& name, ISvcLocator *pSvcLocator) : Algorithm(name,pSvcLocator), 
          m_events(0), m_rejected(0), m_noEbfData(0), m_curMode(enums::Lsf::NoMode), m_mootSvc(0), m_initialized(false)
{

    // Properties for this algorithm
    // Parameter: UseMootConfig
    // Default is to expect TO use Moot for configuration information
    // MUST SET FALSE IN JO FILE for other than pipeline running
    declareProperty("UseMootConfig",    m_mootConfig         = true);
    // Parameter: RejectEvents
    // Default is TO NOT use this algorithm to accept/reject events (for MC production running)
    declareProperty("RejectEvents",     m_rejectEvents       = false);
    // Parameter: PassThrough
    // Dafault is TO configure the "pass through" filter for running
    declareProperty("PassThrough",      m_passThrough        = true);
    // Parameter: FailNoEbfData
    // Default is TO NOT fail the algorithm if ebf data is missing
    declareProperty("FailNoEbfData",    m_failNoEbfData      = false);
    // Parameter: ActiveFilterList
    // This contains the list of filters which are "active" (participate in accept/reject events)
    declareProperty("ActiveFilterList", m_activeFilters);
    // Paramter: FilterList
    // This contains the list of filters which will be configured and run by this algorithm
    declareProperty("FilterList",       m_filterList);

    // Set up default list of filters to configure for running 
    // This should not normally be changed by JO parameters! 
    std::vector<std::string> filterList;
    filterList.push_back("GammaFilter");
    filterList.push_back("MIPFilter");
    filterList.push_back("HIPFilter");
    filterList.push_back("DGNFilter");
    filterList.push_back("FilterTrack");
    // Temporarily remove these (but leave here to remind me to put back in!)
    //filterList.push_back("TkrOutput");
    //filterList.push_back("CalOutput");
    //filterList.push_back("GemOutput");
    m_filterList = filterList;

    // Set up the active filter vector default list
    m_activeFilters.clear();
    m_activeFilters.push_back(OnboardFilterTds::ObfFilterStatus::GammaFilter);
    m_activeFilters.push_back(OnboardFilterTds::ObfFilterStatus::HIPFilter);

    // Initialize the tool name map 
    m_idToToolNameMap[GAMMA_DB_SCHEMA] = "GammaFilterTool";
    m_idToToolNameMap[HIP_DB_SCHEMA]   = "HIPFilterTool";
    m_idToToolNameMap[MIP_DB_SCHEMA]   = "MIPFilterTool";
    m_idToToolNameMap[DGN_DB_SCHEMA]   = "DGNFilterTool";
}
/* --------------------------------------------------------------------- */


StatusCode OnboardFilter::initialize()
{
    // Some shenanigans to get a persistent stream reference to pass to obfInterface...
    MsgStream* logPtr = new MsgStream(msgSvc(), name());
    MsgStream& log    = (*logPtr);

    StatusCode sc     = StatusCode::SUCCESS;

    setProperties();

    log << MSG::INFO << "OnboardFilter initialize method called" << endreq;

    // Get an instance of the filter interface
    m_obfInterface = ObfInterface::instance();

    // Retrieve (and initialize) the FSWAuxLibsTool which will load pedestal, gain and geometry libraries
    IFilterTool* toolPtr = 0;
    if (StatusCode scTool = toolSvc()->retrieveTool("FSWAuxLibsTool", toolPtr) == StatusCode::FAILURE)
    {
        log << MSG::ERROR << "Failed to load the FSW Auxiliary libraries" << endreq;
        return scTool;
    }
        
    // Recover MootSvc
    if (StatusCode sc = service("MootSvc", m_mootSvc, true) == StatusCode::FAILURE)
    {
        // Let the world there was no moot found
        log << MSG::INFO << "Moot service not found, using default configurations" << endreq;
    }

    // Check if MootSvc was not found but we expect to use it
    if (!m_mootSvc && m_mootConfig.value())
    {
        // Anders suggests that if we asked for moot and its not there then we should "crash"
        log << MSG::ERROR << "UseMootConfig set true but no moot service found, exiting..." << endreq;
        return StatusCode::FAILURE;
    }

    // If MootSvc is present then use it to set UseMootConfig
    if (m_mootSvc)
    {
        if (m_mootSvc->noMoot()) m_mootConfig = false;
        else                     m_mootConfig = true;
    }

    // If we are not using Moot then go ahead and initialize now
    if (!m_mootConfig.value()) sc = initFilters();
  
    return sc;
}

StatusCode OnboardFilter::initFilters()
{
    // Some shenanigans to get a persistent stream reference to pass to obfInterface...
    MsgStream log(msgSvc(), name());

    log << MSG::INFO << "Initializing Filter Settings" << endreq;

    // If using moot to configure for the filter configuration then do here
    if (m_mootConfig.value())
    {
        // Get back the list of active filters
        std::vector<CalibData::MootFilterCfg> filterCfgVec;
        unsigned int filterCnt = m_mootSvc->getActiveFilters(filterCfgVec);

        // Clear the active filter list and re-populate from Moot...
        m_activeFilters.clear();

        // Loop through the available moot configurations. 
        for(std::vector<CalibData::MootFilterCfg>::const_iterator filterIter = filterCfgVec.begin();
            filterIter != filterCfgVec.end(); filterIter++)
        {
            log << MSG::INFO << "Moot has filter " <<  filterIter->getName() << " as active" << endreq;
            m_activeFilters.push_back(filterIter->getSchemaId());
        }
    }

    // Loop through the list of filters to configure
    IFilterTool*                    toolPtr    = 0;
    int                             nFilters   = 0;
    const std::vector<std::string>& filterList = m_filterList;
    for(std::vector<std::string>::const_iterator filterIter = filterList.begin(); filterIter != filterList.end(); filterIter++)
    {
        std::string filterTool = *filterIter + "Tool";

        if (StatusCode sc = toolSvc()->retrieveTool(filterTool, toolPtr, this) == StatusCode::FAILURE)
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

    // Ok, if here we are initialized!
    m_initialized = true;
  
    return StatusCode::SUCCESS;
}

StatusCode OnboardFilter::execute()
{
    MsgStream log(msgSvc(), name());

    // If we are using moot then we don't actually initialize filters until first event
    if (!m_initialized)
    {
        if (StatusCode sc = initFilters() != StatusCode::SUCCESS)
        {
            log << MSG::ERROR << "Failed to initialized the filters" << endreq;
            return sc;
        }
    }

    // Recover the meta data in order to check the run mode
    SmartDataPtr<LsfEvent::MetaEvent> metaEventTds(eventSvc(), "/Event/MetaEvent");

    // If running pipeline then we'll have metaEvent data. If we have that then need to check
    // for mode changes on the active filters
    if (metaEventTds) 
    {
        log << MSG::DEBUG << "Found MetaEvent data, checking run mode" << endreq;

        lsfData::DatagramInfo datagram    = metaEventTds->datagram( );
        enums::Lsf::Mode      mode        = datagram.mode();
        unsigned int          modeChanges = datagram.modeChanges( );  //since start of the run

        if (mode != m_curMode)
        {
            log << MSG::INFO << "Detected a mode change from MetaEvent datagram, changing from " << m_curMode 
                << " to " << mode << endreq;

            // If moot is controlling the configuration, then go through here to set the modes
            if (m_mootConfig.value())
            {
                // Look up from MootSvc the list of active filters (in case it changes) 
                std::vector<CalibData::MootFilterCfg> filterCfgVec;
                unsigned int filterCnt = m_mootSvc->getActiveFilters(filterCfgVec);

                // Clear the active filter list and re-populate from Moot...
                m_activeFilters.clear();

                // Filter Tool pointer...
                IFilterTool* toolPtr = 0;

                // Loop through the available moot configurations. 
                for(std::vector<CalibData::MootFilterCfg>::const_iterator filterIter = filterCfgVec.begin();
                    filterIter != filterCfgVec.end(); filterIter++)
                {
                    log << MSG::INFO << "Moot has filter " <<  filterIter->getName() << " as active" << endreq;
                    m_activeFilters.push_back(filterIter->getSchemaId());

                    // Use the schema id to retrieve the tool name to change the mode for 
                    IdToNameMap::iterator nameIter = m_idToToolNameMap.find(filterIter->getSchemaId());

                    // Ok, this just can't happen, right?
                    if (nameIter == m_idToToolNameMap.end())
                    {
                        log << MSG::ERROR << "Cannot translate Moot schema id " << filterIter->getSchemaId() << endreq;
                        return StatusCode::FAILURE;
                    }

                    std::string filterTool = nameIter->second;

                    // Look up the tool and check we found it... just in case...
                    if (StatusCode sc = toolSvc()->retrieveTool(filterTool, toolPtr, this) == StatusCode::FAILURE)
                    {
                        log << MSG::ERROR << "Failed to find the " << filterTool << " tool" << endreq;
                        return sc;
                    }

                    // Finally... set the new mode
                    toolPtr->setMode(mode);
                }
            }
            // If no moot, then set the mode for all the filters we have set up
            else
            {
                // Loop through the list of filters to configure
                IFilterTool* toolPtr    = 0;
                int          nFilters   = 0;
                const std::vector<std::string>& filterList = m_filterList;
                for(std::vector<std::string>::const_iterator filterIter = filterList.begin(); filterIter != filterList.end(); filterIter++)
                {
                    std::string filterTool = *filterIter + "Tool";

                    if (StatusCode sc = toolSvc()->retrieveTool(filterTool, toolPtr, this) == StatusCode::FAILURE)
                    {
                        log << MSG::ERROR << "Failed to initialize the " << *filterIter << " tool" << endreq;
                        return sc;
                    }

                    // Dump the initialized configuration
                    toolPtr->setMode(mode);
                }
            }

            m_curMode = mode;
        }
    }

    // Keep track of number of events
    m_events++;

    // Back to event processing, first check for the ebf data
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
    OnboardFilterTds::ObfFilterStatus *obfStatus = new OnboardFilterTds::ObfFilterStatus;
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
        for(ActiveFilterVec::iterator filtItr = m_activeFilters.begin(); filtItr != m_activeFilters.end(); filtItr++)
        {
            // Retrieve enum
            OnboardFilterTds::ObfFilterStatus::FilterKeys key = (OnboardFilterTds::ObfFilterStatus::FilterKeys)(*filtItr);

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

