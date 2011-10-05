/**  @file HIPFilterTool.cxx
    @brief implementation of class HIPFilterTool
    
  $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/HIPFilterTool.cxx,v 1.19 2011/04/19 17:32:02 usher Exp $  
*/

#include "IFilterTool.h"

#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/GaudiException.h" 
#include "GaudiKernel/IDataProviderSvc.h"

// Moot stuff for discerning filter configurations
#include "CalibData/Moot/MootData.h"
#include "MootSvc/IMootSvc.h"

#include "Event/TopLevel/Event.h"
#include "Event/TopLevel/EventModel.h"
#include "OnboardFilterTds/ObfFilterStatus.h"

#include "facilities/Util.h"

// Interface to EDS package here
#include "ObfInterface.h"

// FSW includes go here
#ifdef OBF_B1_1_3
#include "FSWHeaders/CDM_pubdefs.h"
#endif
#ifdef OBF_B3_0_0
#include "CDM/CDM_pubdefs.h"
#endif

#include "EFC_DB/EFC_DB_schema.h"
#include "EFC_DB/EH_ids.h"
#include "XFC_DB/MFC_DB_schema.h"
#include "XFC_DB/MIP_DB_instance.h"
#include "XFC/MFC_status.h"

// Contains all info for a particular filter's release
#ifdef OBF_B3_0_0
#include "EFC/EFC.h"
//#include "EFC/../src/EFC_samplerDef.h"
#include "HIPFilterLibsB3-0-0.h"
#endif
#ifdef OBF_B1_1_3
#include "FSWHeaders/EFC.h"
// FSW include but made local due to keyword usage
//#include "FSWHeaders/EFC_sampler.h"
#include "HIPFilterLibsB1-1-3.h"
#endif

// Useful stuff! 
#include <map>
#include <stdexcept>
#include <sstream>

/** @class HIPFilterTool
    @brief Manages the Diagnostic Filter
    @author Tracy Usher
*/
class HIPFilterTool : public AlgTool, virtual public IFilterTool
{
public:

    // Standard Gaudi Tool constructor
    HIPFilterTool(const std::string& type, const std::string& name, const IInterface* parent);

    // After building it, destroy it
    ~HIPFilterTool();

    /// @brief Intialization of the tool
    StatusCode initialize();

    /// @brief Finalize method for the tool
    StatusCode finalize();

    // Set Mode for a given filter
    virtual void setMode(unsigned int mode);

    // This defines the method called for end of event processing
    virtual void eoeProcessing(EDS_fwIxb* ixb);

    // This for end of run processing
    virtual void eorProcessing();

    // Dump out the running configuration
    virtual void dumpConfiguration();

private:

    // Private function to load FSW libraries
    bool loadLibrary (std::string libraryName, std::string libraryPath="", int verbosity=0);

    //****** This section for defining JO parameters
    // This is somewhat useless but if set will be passed to the CDM utility to print info
    IntegerProperty   m_verbosity;

    // Configuring the Diagnostic Filter
    bool              m_leakAllEvents;   // If true then GSW running of diagnostic Filter will leak all events

    // Which filter configuration to run
    StringProperty    m_configToRun;

    //****** This section for controlling implementation of Gamma Filter
    // Filter ID returned from EDS_fw after initialization
    int               m_handlerId;

    // Current mode and configuration
    unsigned short    m_curConfig;
    unsigned short    m_curMode;

    //****** This section contains various useful member variables
    // Counters to keep track of bit frequency during a given run
    int               m_statusBits[32]; //array to count # of times each veto bit was set

    // Pointer to the filter library class with release specific information
    IFilterLibs*      m_filterLibs;

    /// Pointer to the Gaudi data provider service
    IDataProviderSvc* m_dataSvc;

    /// MootSvc for filter configurations
    IMootSvc*         m_mootSvc;
};

static ToolFactory<HIPFilterTool> s_factory;
const IToolFactory& HIPFilterToolFactory = s_factory;
//------------------------------------------------------------------------

HIPFilterTool::HIPFilterTool(const std::string& type, 
                                 const std::string& name, 
                                 const IInterface* parent) :
                                 AlgTool(type, name, parent)
                               , m_curConfig(0)
                               , m_curMode(EFC_DB_MODE_K_NORMAL)
                               , m_mootSvc(0)
{
    //Declare the additional interface
    declareInterface<IFilterTool>(this);

    // declare properties with setProperties calls
    // Paramter: LeakAllEvents
    // Default is TO "leak" (pass status/filter information) all events (DEPRECATED)
    declareProperty("LeakAllEvents", m_leakAllEvents = false);
    // Parameter: Configuration
    // Overrides the default configuration given in the Master Configuration file
    declareProperty("Configuration", m_configToRun   = "");

    declareProperty("verbosity",     m_verbosity     = 0);
    
    // zero our counters
    memset(m_statusBits, 0, 32*sizeof(int));

    return;
}
//------------------------------------------------------------------------
HIPFilterTool::~HIPFilterTool()
{
}

StatusCode HIPFilterTool::initialize()
{
    StatusCode sc   = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());

    // Set the properties
    setProperties();

    //Locate and store a pointer to the data service
    if( (sc = service("EventDataSvc", m_dataSvc)).isFailure() ) 
    {
        log << MSG::ERROR << "Service [EventDataSvc] not found" << endreq;
        return sc;
    }

    // Attempt to retrieve the JO parameter "UseMootSvc" from OnboardFilter
    bool useMootSvc = false;
    try
    {
        if (IProperty* parentProp = dynamic_cast<IProperty*>(const_cast<IInterface*>(parent())))
        {
            SimplePropertyRef<bool> useMoot("UseMootConfig", useMootSvc);
            StatusCode sc = parentProp->getProperty(&useMoot);
        }
    }
    // If the above fails then it throws an exception. I think that can't happen here, but if it 
    // does then we'll proceed ahead without Moot...
    catch(...) {}

    // If OnboardFilter says we are using Moot then try to look it up
    if (useMootSvc)
    {
        if (StatusCode sc = service("MootSvc", m_mootSvc) == StatusCode::FAILURE)
        {
            log << MSG::ERROR << "Unable to retrieve MootSvc" << endreq;
            return sc;
        }
    }

    try
    {
        // Get ObfInterface pointer
        ObfInterface* obf = ObfInterface::instance();
#ifdef OBF_B3_0_0
        m_filterLibs = new HIPFilterLibsB3_0_0();
#endif
#ifdef OBF_B1_1_3
        m_filterLibs = new HIPFilterLibsB1_1_3();
#endif

        const EFC_DB_Schema& master = obf->loadFilterLibs(m_filterLibs, m_verbosity);

        // Check to see what mode we want to run... (if a different one requested via JO parameter)
        if (m_configToRun.value() != "")
        {
            unsigned short int configId = m_filterLibs->getInstanceId(m_configToRun.value());

            if (configId != m_filterLibs->getMasterConfiguration().filter.mode2cfg[m_curMode]) 
                m_filterLibs->getMasterConfiguration().filter.mode2cfg[m_curMode] = configId;
        }

        // Retrieve the configuration to use for normal mode
        unsigned char configToRun = master.filter.mode2cfg[EFC_DB_MODE_K_NORMAL];

        m_handlerId = obf->setupFilter(&master, configToRun);

        if (m_handlerId == -100)
        {
            log << MSG::ERROR << "Failed to initialize Diagnostic Filter" << endreq;
            return StatusCode::FAILURE;
        }

        // Bit mask for this filter
        unsigned int target = obf->getFilterTargetMask(master.filter.id);

        // Are we using moot and is this an active filter?
        bool activeFilter = false;

        if (m_mootSvc)
        {
            std::vector<CalibData::MootFilterCfg> filterCfgVec;
            unsigned int filterCnt = m_mootSvc->getActiveFilters(filterCfgVec);
            
            for(std::vector<CalibData::MootFilterCfg>::const_iterator filterIter = filterCfgVec.begin();
                filterIter != filterCfgVec.end(); filterIter++)
            {
                if (filterIter->getSchemaId() == m_filterLibs->FilterSchema())
                {
                    activeFilter = true;
                    log << MSG::INFO << "Moot has filter " <<  filterIter->getName() << " as active" << endreq;
                    break;
                }
            }
        }

        // Loop through and associate configurations to modes and enable the filter for that mode
        for (int modeIdx = 0; modeIdx < EFC_DB_MODE_K_CNT; modeIdx++)
        {
            unsigned int configuration = m_filterLibs->getMasterConfiguration().filter.mode2cfg[modeIdx];

            // If MootSvc configured and filter is active then attempt to retrieve the information from moot
            if (activeFilter)
            {
                std::string filterName = "";
                CalibData::MootFilterCfg* mootCfg = m_mootSvc->getActiveFilter(modeIdx, m_handlerId, filterName);

                // Returned configuration for this handler and mode 
                if (mootCfg)
                {
                    configuration = mootCfg->getInstanceId();
                    log << MSG::INFO << "Moot: mode " << modeIdx << " associated with configuration:\n" << mootCfg->getSrcPath() << endreq;
                }
            }

            obf->associateConfigToMode(target, modeIdx, configuration);
        }

        // Enable the filter
        obf->enableDisableFilter(target, target);

        // Call setMode to do the rest
        setMode(m_curMode);

        // Set the Gamma Filter output routine
        obf->setEovOutputCallBack(this);
    }
    catch(ObfInterface::ObfException& obfException)
    {
        log << MSG::ERROR << obfException.m_what << endreq;
        return StatusCode::FAILURE;
    }

    return sc;
}

StatusCode HIPFilterTool::finalize ()
{
    StatusCode  status = StatusCode::SUCCESS;
    
    return status;
}

// Set Mode and Configuration for a given filter
void HIPFilterTool::setMode(unsigned int mode)
{
    // Output what we are doing...
    MsgStream log(msgSvc(), name());

    std::string modeDesc[] = {"EFC_DB_MODE_K_NORMAL",
                              "EFC_DB_MODE_K_TOO",
                              "EFC_DB_MODE_K_ARR",
                              "EFC_DB_MODE_K_RSVD3",
                              "EFC_DB_MODE_K_RSVD4",
                              "EFC_DB_MODE_K_RSVD5",
                              "EFC_DB_MODE_K_RSVD6",
                              "EFC_DB_MODE_K_RSVD7" };

    log << MSG::INFO << "Received request to change mode from " << modeDesc[m_curMode] << " to " << modeDesc[mode] << endreq;

    // Get ObfInterface pointer
    ObfInterface* obf = ObfInterface::instance();

    // Schema id
    unsigned short int masterId = m_filterLibs->getMasterConfiguration().filter.id;

    // Bit mask for this filter
    unsigned int target = obf->getFilterTargetMask(masterId);

    // Set the default mode to run
    obf->selectFiltermode(target, mode);

    // If we are "leaking" all events then modify here
    // Note: this is standard mode of running for GSW version of obf
    if (m_leakAllEvents)
    {
        // Modify the veto mask is requested (this means we are running "pass through" mode)
        EFC_sampler* sampler = (EFC_sampler*)obf->getFilterPrm(masterId, EFC_OBJECT_K_SAMPLER);

        // Set filter to leak all events
        ////sampler->prescale.prescalers[0].refresh = 1;
    }

    return;
}

// This routine for dumping to log file the configuration being run
void HIPFilterTool::dumpConfiguration()
{
    MsgStream log(msgSvc(), name());

    log << MSG::INFO << "HIP Filter Setup - FSW release " << m_filterLibs->FlightSoftwareRelease() << "\n"
        << "   Using configuration: " << m_filterLibs->getInstanceIdString(m_filterLibs->getMasterConfiguration().filter.mode2cfg[0]) << endreq;

    return;
}

// This defines the method called for end of event processing
void HIPFilterTool::eoeProcessing(EDS_fwIxb* ixb)
{
    // Retrieve the Gamma Filter Status Word
    EDS_rsdDsc*   rsdDsc       = ixb->rsd.dscs + m_handlerId;
    unsigned char sb           = rsdDsc->sb;
    unsigned int* dscPtr       = (unsigned int*)rsdDsc->ptr;
    unsigned int  statusWord   = *dscPtr++;

    // Retrieve the output status TDS container object
    SmartDataPtr<OnboardFilterTds::ObfFilterStatus> obfFilterStatus(m_dataSvc,"/Event/Filter/ObfFilterStatus");

    // Create a new HFC status TDS sub object
    OnboardFilterTds::ObfHipStatus* hfcStat = new OnboardFilterTds::ObfHipStatus(rsdDsc->id, statusWord, sb, 0);

    // Add it to the TDS object
    obfFilterStatus->addFilterStatus(OnboardFilterTds::ObfFilterStatus::HIPFilter, hfcStat);

    // Accumulate the status bit hits
    for(int ib = 0; ib < 32; ib++) if (statusWord & 1 << ib) m_statusBits[ib]++;

    return;
}

// This for end of run processing
void HIPFilterTool::eorProcessing()
{
    MsgStream log(msgSvc(), name());

    // Output the bit frequency table
    log << MSG::INFO << "-- HIP Filter Status Word bit frequency table -- \n";

    for(int ib = 0; ib < 32; ib++)
    {
        log << "    " << m_filterLibs->getStatWordDesc(ib) << " = " << m_statusBits[ib] << "\n";
    }

    log  << endreq;

    return;
}
