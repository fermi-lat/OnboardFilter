/* ---------------------------------------------------------------------- *//*!

   \file  OnboardFilter.cxx
   \brief  run filter
   \author JJRussell - russell@slac.stanford.edu

\verbatim

  CVS $Id: OnboardFilter.cxx,v 1.65 2007/03/29 19:50:46 usher Exp $
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
#include "OnboardFilterTds/FilterStatus.h"
#include "OnboardFilterTds/ObfFilterStatus.h"

#include "ObfInterface.h"
#include "GammaFilterOutput.h"
#include "MipFilterOutput.h"
#include "CNOFilterOutput.h"
#include "DFCFilterOutput.h"
#include "CalFilterOutput.h"
#include "TkrFilterOutput.h"
#include "GemFilterOutput.h"


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
    bool        m_gammaFilter;
    bool        m_CNOFilter;
    bool        m_MIPFilter;
    bool        m_DFCFilter;
    bool        m_passThrough;

    // Extra output call backs
    bool        m_calFilterInfo;
    bool        m_tkrFilterInfo;
    bool        m_gemFilterInfo;
    bool        m_tkrHitsInfo;

    int         m_mask;            //mask for setting filter to reject
    unsigned    m_gamBitsToIgnore; // This sets a mask of gamma filter veto bits to ignore
    int         m_rejected;
    int         m_noEbfData;
    bool        m_failNoEbfData;

    // Path to shareables
    std::string m_FileNamePath;

    // File name for peds/gains
    std::string m_FileName_Pedestals;
    std::string m_FileName_Gains;

    ObfInterface* m_obfInterface;

    // Call back parm for holding pointers to output classes
    FilterTdsPointers* m_tdsPointers;

    // The list of filters to use to reject events
    //typedef std::list<OnboardFilterTds::IObfStatus::FilterKeys> FilterList;
    typedef std::vector<unsigned int> FilterList;
    FilterList  m_filterList;
};



static const AlgFactory<OnboardFilter> Factory;
const IAlgFactory& OnboardFilterFactory = Factory;
//FilterInfo OnboardFilter::myFilterInfo;

OnboardFilter::OnboardFilter(const std::string& name, ISvcLocator *pSvcLocator) : Algorithm(name,pSvcLocator), 
          m_rejected(0), m_noEbfData(0)
{
    // The default gamma veto bit ignore mask
    unsigned gamBitsToIgnore = GFC_STATUS_M_TKR_LT_2_ELO
                             | GFC_STATUS_M_EL0_ETOT_90 
                             | GFC_STATUS_M_EL0_ETOT_01
                             | GFC_STATUS_M_SPLASH_1;

    // Properties for this algorithm
    declareProperty("FileNamePath",   m_FileNamePath       = "$(FLIGHTCODELIBS)");
    declareProperty("FileNamePeds",   m_FileName_Pedestals = "cal_db_pedestals");
    declareProperty("FileNameGains",  m_FileName_Gains     = "cal_db_gains");
    declareProperty("mask",           m_mask               = 0);
    declareProperty("GamFilterMask",  m_gamBitsToIgnore    = gamBitsToIgnore);
    declareProperty("PassThrough",    m_passThrough        = true);
    declareProperty("gammaFilter",    m_gammaFilter        = true);
    declareProperty("CNOFilter",      m_CNOFilter          = true);
    declareProperty("MIPFilter",      m_MIPFilter          = true);
    declareProperty("DFCFilter",      m_DFCFilter          = true);
    declareProperty("CalFilterInfo",  m_calFilterInfo      = true);
    declareProperty("TkrFilterInfo",  m_tkrFilterInfo      = true);
    declareProperty("GemFilterInfo",  m_gemFilterInfo      = true);
    declareProperty("TkrHitsInfo",    m_tkrHitsInfo        = false);
    declareProperty("FailNoEbfData",  m_failNoEbfData      = false);
    declareProperty("FilterList",     m_filterList);

    // Set up the default to filter on Gamma and CNO filters
    m_filterList.clear();
    m_filterList.push_back(OnboardFilterTds::ObfFilterStatus::GammaFilter);
    m_filterList.push_back(OnboardFilterTds::ObfFilterStatus::CNOFilter);
}
/* --------------------------------------------------------------------- */


StatusCode OnboardFilter::initialize()
{
    // Some shenanigans to get a persistent stream reference to pass to obfInterface...
    MsgStream* logPtr = new MsgStream(msgSvc(), name());
    MsgStream& log   = (*logPtr);

    setProperties();

    log << MSG::INFO << "Initializing Filter Settings" << endreq;

    // Get an instance of the local class to hold output pointers
    m_tdsPointers = new FilterTdsPointers();

    // Load the external libraries
    int ret = facilities::Util::expandEnvVar(&m_FileNamePath);

    // Get an instance of the filter interface
    m_obfInterface = new ObfInterface(log, m_FileNamePath, m_tdsPointers);

    // Load the correct calibration libraries
    std::string calPedFile = m_FileName_Pedestals;
    m_obfInterface->loadLibrary(calPedFile);
    
    std::string calGainFile = m_FileName_Gains;
    m_obfInterface->loadLibrary(calGainFile);

    // Set variable for filter priority
    int priority = 0;

    // Set up the Gamma Filter and associated output
    if (m_gammaFilter)
    {
        unsigned vetoMask = m_passThrough ? 0 : ~m_gamBitsToIgnore & GFC_STATUS_M_VETOES;

        int filterId = m_obfInterface->setupFilter("GammaFilter", priority++, vetoMask, m_passThrough);

        if (filterId == -100)
        {
            log << MSG::ERROR << "Failed to initialize Gamma Filter" << endreq;
        }

        // Set the Gamma Filter output routine
        OutputRtn* outRtn = new GammaFilterOutput(filterId, m_gamBitsToIgnore, m_passThrough);
        m_obfInterface->setEovOutputCallBack(outRtn);
    }

    // Set up the CNO (Heavy Ion) filter and associated output
    if (m_CNOFilter)
    {
        unsigned vetoMask = HFC_STATUS_M_VETO_DEF;

        int filterId = m_obfInterface->setupFilter("CNOFilter", priority++, vetoMask, false);

        if (filterId == -100)
        {
            log << MSG::ERROR << "Failed to initialize CNO Filter" << endreq;
        }

        // Set the CNO filter output routine
        OutputRtn* outRtn = new CNOFilterOutput(filterId);
        m_obfInterface->setEovOutputCallBack(outRtn);
    }

    // Set up the MIP filter and associated output
    if (m_MIPFilter)
    {
        unsigned vetoMask = MFC_STATUS_M_VETO_DEF;

        int filterId = m_obfInterface->setupFilter("MipFilter", priority++, vetoMask, false);

        if (filterId == -100)
        {
            log << MSG::ERROR << "Failed to initialize MIP Filter" << endreq;
        }

        // Set the Mip filter output routine
        OutputRtn* outRtn = new MipFilterOutput(filterId);
        m_obfInterface->setEovOutputCallBack(outRtn);
    }

    // Set up the CNO (Heavy Ion) filter and associated output
    if (m_DFCFilter)
    {
        unsigned vetoMask = DFC_STATUS_M_VETO_DEF;

        int filterId = m_obfInterface->setupFilter("DFCFilter", priority++, vetoMask, false);

        if (filterId == -100)
        {
            log << MSG::ERROR << "Failed to initialize DFC Filter" << endreq;
        }

        // Set the CNO filter output routine
        OutputRtn* outRtn = new DFCFilterOutput(filterId);
        m_obfInterface->setEovOutputCallBack(outRtn);
    }

    // Set up a "passthrough" filter which allows us to always retrieve results from 
    // the filters after they have run
    if (!m_obfInterface->setupPassThrough(0))
    {
        log << MSG::ERROR << "Failed to initialize pass through Filter" << endreq;
    }

    // Extra filter output only if filters set up
    if (priority >0)
    {
        // Calorimater info if requested
        if (m_calFilterInfo)
        {
            OutputRtn* outRtn = new CalFilterOutput();
            m_obfInterface->setEovOutputCallBack(outRtn);
        }

        // Calorimater info if requested
        if (m_tkrFilterInfo)
        {
            OutputRtn* outRtn = new TkrFilterOutput();
            m_obfInterface->setEovOutputCallBack(outRtn);
        }

        // Calorimater info if requested
        if (m_gemFilterInfo)
        {
            OutputRtn* outRtn = new GemFilterOutput();
            m_obfInterface->setEovOutputCallBack(outRtn);
        }
    }
    // Watch out for no filters set up!
    else
    {
        log << MSG::WARNING << "No filters have been requested! " << endreq;
    }
  
    return StatusCode::SUCCESS;
}

StatusCode OnboardFilter::execute()
{
    MsgStream log(msgSvc(), name());
    
    //  Make the tds objects
    OnboardFilterTds::TowerHits *hits = new OnboardFilterTds::TowerHits;
    eventSvc()->registerObject("/Event/Filter/TowerHits",hits);
    OnboardFilterTds::FilterStatus *newStatus=new OnboardFilterTds::FilterStatus;
    eventSvc()->registerObject("/Event/Filter/FilterStatus",newStatus);

    OnboardFilterTds::ObfFilterStatus *obfStatus=new OnboardFilterTds::ObfFilterStatus;
    eventSvc()->registerObject("/Event/Filter/ObfFilterStatus",obfStatus);

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

    // Store pointers in the call back object
    m_tdsPointers->m_filterStatus    = newStatus;
    m_tdsPointers->m_obfFilterStatus = obfStatus;
    m_tdsPointers->m_towerHits       = hits;

    // Call the filter
    unsigned int fate = m_obfInterface->filterEvent(ebfData);

    if (fate != 4)
    {
        log << MSG::ERROR << "Error in filter processing, fate = " << fate << endreq;
    }

    // Check to see if we are vetoing events at this stage
    if (m_mask != 0)
    {
        unsigned int combStatus = 0xFFFFFFFF;

        // Loop through the list of filters to apply
        for(FilterList::iterator listItr = m_filterList.begin(); listItr != m_filterList.end(); listItr++)
        {
            // Retrieve enum
            //OnboardFilterTds::ObfFilterStatus::FilterKeys key = *listItr;
            OnboardFilterTds::ObfFilterStatus::FilterKeys key = (OnboardFilterTds::ObfFilterStatus::FilterKeys)(*listItr);

            // Look up the information for this filter
            const OnboardFilterTds::IObfStatus* filterStat = obfStatus->getFilterStatus(key);

            // Make sure the filter ran
            if (!filterStat) continue;

            // And result into previous results
            combStatus &= filterStat->getStatus32();
        }

        // High order bit set means we reject events, assume m_mask = -1 does that job
        if (combStatus & m_mask)
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
    log << MSG::INFO << "Rejected " << m_rejected << " triggers using mask: "
        << std::hex << m_mask << std::dec << endreq;

    return StatusCode::SUCCESS;
}

