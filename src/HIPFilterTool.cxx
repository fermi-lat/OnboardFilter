/**  @file HIPFilterTool.cxx
    @brief implementation of class HIPFilterTool
    
  $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/HIPFilterTool.cxx,v 1.2 2008/05/16 18:00:42 usher Exp $  
*/

#include "IFilterTool.h"

#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/GaudiException.h" 
#include "GaudiKernel/IDataProviderSvc.h"

#include "Event/TopLevel/Event.h"
#include "Event/TopLevel/EventModel.h"
#include "OnboardFilterTds/ObfFilterStatus.h"

#include "facilities/Util.h"

// Interface to EDS package here
#include "ObfInterface.h"

// FSW includes go here
#include "FSWHeaders/CDM_pubdefs.h"
#include "EFC_DB/EFC_DB_schema.h"
#include "EFC_DB/EH_ids.h"
#include "XFC_DB/MFC_DB_schema.h"
#include "XFC_DB/MIP_DB_instance.h"
#include "XFC/MFC_status.h"

#include "EFC/EFC.h"
#include "LSE/LFR_key.h"

// FSW include but made local do to keyword usage
#include "FSWHeaders/EFC_sampler.h"

// Contains all info for a particular filter's release
#include "HIPFilterLibsB1-0-8.h"

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

    // Set Mode and Configuration for a given filter
    void setModeAndConfig(unsigned int mode, unsigned int config);

    // Dump out the running configuration
    void dumpConfiguration();

    // This defines the method called for end of event processing
    virtual void eoeProcessing(EDS_fwIxb* ixb);

    // This for end of run processing
    virtual void eorProcessing();

private:

    // Private function to load FSW libraries
    bool loadLibrary (std::string libraryName, std::string libraryPath="", int verbosity=0);

    //****** This section for defining JO parameters
    // This is somewhat useless but if set will be passed to the CDM utility to print info
    IntegerProperty   m_verbosity;

    // Configuring the Diagnostic Filter
    bool              m_leakAllEvents;   // If true then GSW running of diagnostic Filter will leak all events

    //****** This section for controlling implementation of Gamma Filter
    // Filter ID returned from EDS_fw after initialization
    int               m_filterId;

    //****** This section contains various useful member variables
    // Counters to keep track of bit frequency during a given run
    int               m_vetoBits[17];   //array to count # of times each veto bit was set
    int               m_statusBits[15]; //array to count # of times each veto bit was set

    // Pointer to the filter library class with release specific information
    IFilterLibs*      m_filterLibs;

    /// Pointer to the Gaudi data provider service
    IDataProviderSvc* m_dataSvc;
};

static ToolFactory<HIPFilterTool> s_factory;
const IToolFactory& HIPFilterToolFactory = s_factory;
//------------------------------------------------------------------------

HIPFilterTool::HIPFilterTool(const std::string& type, 
                                 const std::string& name, 
                                 const IInterface* parent) :
                                 AlgTool(type, name, parent)
{
    //Declare the additional interface
    declareInterface<IFilterTool>(this);

    // declare properties with setProperties calls
    declareProperty("LeakAllEvents", m_leakAllEvents = false);
    
    // zero our counters
    memset(m_vetoBits,   0, 17*sizeof(int));
    memset(m_statusBits, 0, 15*sizeof(int));

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

    try
    {
        // Get ObfInterface pointer
        ObfInterface* obf = ObfInterface::instance();

        m_filterLibs = new HIPFilterLibsB1_0_8();
        const EFC_DB_Schema& master = obf->loadFilterLibs(m_filterLibs, m_verbosity);

        // Retrieve the configuration to use for normal mode
        unsigned char configToRun = master.filter.mode2cfg[EFC_DB_MODE_K_NORMAL];

        m_filterId = obf->setupFilter(&master, configToRun);

        if (m_filterId == -100)
        {
            log << MSG::ERROR << "Failed to initialize Diagnostic Filter" << endreq;
            return StatusCode::FAILURE;
        }

        // Bit mask for this filter
        unsigned int target = obf->getFilterTargetMask(master.filter.id);

        // Loop through and associate configurations to modes and enable the filter for that mode
        for (int modeIdx = 0; modeIdx < EFC_DB_MODE_K_CNT; modeIdx++)
        {
            unsigned int configuration = m_filterLibs->getMasterConfiguration().filter.mode2cfg[modeIdx];

            obf->associateConfigToMode(target, modeIdx, configuration);
            obf->enableDisableFilter(target, target);
        }

        // Set the default mode to run
        obf->selectFiltermode(target, EFC_DB_MODE_K_NORMAL);

        // If we are "leaking" all events then modify here
        // Note: this is standard mode of running for GSW version of obf
        if (m_leakAllEvents)
        {
            // Modify the veto mask is requested (this means we are running "pass through" mode)
            EFC_sampler* sampler = (EFC_sampler*)obf->getFilterPrm(master.filter.id, EFC_OBJECT_K_SAMPLER);

            // Set filter to leak all events
            sampler->prescale.prescalers[0].refresh = 1;
        }

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
void HIPFilterTool::setModeAndConfig(unsigned int mode, unsigned int config)
{
    // Get ObfInterface pointer
    ObfInterface* obf = ObfInterface::instance();

    // Bit mask for this filter
    unsigned int target = m_filterLibs->getMasterConfiguration().filter.id;

    // Associate the configuration to the mode (and vice versa)
    obf->associateConfigToMode(target, mode, config);

    // Set the default mode to run
    obf->selectFiltermode(target, mode);

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
    EDS_rsdDsc*   rsdDsc       = ixb->rsd.dscs + m_filterId;
    unsigned char sb           = rsdDsc->sb;
    unsigned int* dscPtr       = (unsigned int*)rsdDsc->ptr;
    unsigned int  statusWord   = *dscPtr++;

    // If we are running pass through mode then we need to manually set the veto bit
    // in the event their was a veto
    if (m_leakAllEvents)
    {
        if (statusWord & HFC_STATUS_M_VETO_DEF)
        {
            statusWord |= HFC_STATUS_M_VETOED;
            sb         |= EDS_RSD_SB_M_VETOED;
        }
    }

    // Retrieve the output status TDS container object
    SmartDataPtr<OnboardFilterTds::ObfFilterStatus> obfFilterStatus(m_dataSvc,"/Event/Filter/ObfFilterStatus");

    // Create a new HFC status TDS sub object
    OnboardFilterTds::ObfHFCStatus* hfcStat = new OnboardFilterTds::ObfHFCStatus(rsdDsc->id, statusWord, sb);

    // Add it to the TDS object
    obfFilterStatus->addFilterStatus(OnboardFilterTds::ObfFilterStatus::HFCFilter, hfcStat);

    // Increment counters accordingly
    if((statusWord & HFC_STATUS_M_STAGE_GEM) != 0)      m_statusBits[0]++;
    if((statusWord & HFC_STATUS_M_STAGE_DIR) != 0)      m_statusBits[1]++;
    if((statusWord & HFC_STATUS_M_STAGE_CAL) != 0)      m_statusBits[2]++;
    if((statusWord & HFC_STATUS_M_STAGE_CAL_ECHK) != 0) m_statusBits[3]++;
    if((statusWord & HFC_STATUS_M_STAGE_CAL_LCHK) != 0) m_statusBits[4]++;
    if((statusWord & HFC_STATUS_M_MULTI_PKT) != 0)      m_statusBits[5]++;
    if((statusWord & HFC_STATUS_M_ERR_CAL) != 0)        m_statusBits[6]++;
    if((statusWord & HFC_STATUS_M_ERR_CTB) != 0)        m_statusBits[7]++;
    if((statusWord & HFC_STATUS_M_ERR_DIR) != 0)        m_statusBits[8]++;

    if((statusWord & HFC_STATUS_M_LYR_ENERGY) != 0)     m_vetoBits[0]++;
    if((statusWord & HFC_STATUS_M_LYR_COUNTS) != 0)     m_vetoBits[1]++;
    if((statusWord & HFC_STATUS_M_GEM_NOTKR) != 0)      m_vetoBits[2]++;
    if((statusWord & HFC_STATUS_M_GEM_NOCALLO) != 0)    m_vetoBits[3]++;
    if((statusWord & HFC_STATUS_M_GEM_NOCNO) != 0)      m_vetoBits[4]++;

    if((statusWord & HFC_STATUS_M_VETOED) != 0)         m_vetoBits[16]++;

    return;
}

// This for end of run processing
void HIPFilterTool::eorProcessing()
{
    MsgStream log(msgSvc(), name());

    // Output the bit frequency table
    log << MSG::INFO << "-- HFC Filter bit frequency table -- \n" 
        << "    Status Bit                         Value\n"
        << "    HFC_STATUS_M_STAGE_GEM             " << m_statusBits[0] << "\n"       
        << "    HFC_STATUS_M_STAGE_DIR             " << m_statusBits[1] << "\n"       
        << "    HFC_STATUS_M_STAGE_CAL             " << m_statusBits[2] << "\n"      
        << "    HFC_STATUS_M_STAGE_CAL_ECHK        " << m_statusBits[3] << "\n"
        << "    HFC_STATUS_M_STAGE_CAL_LCHK        " << m_statusBits[4] << "\n"
        << "    HFC_STATUS_M_MULTI_PKT             " << m_statusBits[5] << "\n"
        << "    HFC_STATUS_M_ERR_CAL               " << m_statusBits[6] << "\n"
        << "    HFC_STATUS_M_ERR_CTB               " << m_statusBits[7] << "\n"
        << "    HFC_STATUS_M_ERR_DIR               " << m_statusBits[8] << "\n"
    
        << "    Veto Bit Summary" << "\n"
        << "    Trigger Name                           Count\n" << "\n"
        << "    HFC_STATUS_M_LYR_ENERGY            " << m_vetoBits[0] << "\n"
        << "    HFC_STATUS_M_LYR_COUNTS            " << m_vetoBits[1] << "\n"
        << "    HFC_STATUS_M_GEM_NOTKR             " << m_vetoBits[2] << "\n"
        << "    HFC_STATUS_M_GEM_NOCALLO           " << m_vetoBits[3] << "\n"
        << "    HFC_STATUS_M_NOCNO                 " << m_vetoBits[4] << "\n"
        << "    HFC_STATUS_M_VETOED                " << m_vetoBits[16] << "\n"
        << endreq;

    return;
}
