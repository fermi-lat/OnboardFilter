/**  @file DGNFilterTool.cxx
    @brief implementation of class DGNFilterTool
    
  $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/DGNFilterTool.cxx,v 1.0 2008/02/08 21:32:11 usher Exp $  
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
#include "CDM/CDM_pubdefs.h"
#include "EFC_DB/EFC_DB_schema.h"
#include "EFC_DB/EH_ids.h"
#include "XFC_DB/DFC_DB_schema.h"
#include "XFC_DB/DGN_DB_instance.h"
#include "XFC/DFC_status.h"

#include "EFC/EFC.h"
#include "LSE/LFR_key.h"

// FSW include but made local do to keyword usage
#include "FSWHeaders/EFC_sampler.h"

// Contains all info for a particular filter's release
#include "DGNFilterLibsB1-0-8.h"

// Useful stuff! 
#include <map>
#include <stdexcept>
#include <sstream>

/** @class DGNFilterTool
    @brief Manages the Diagnostic Filter
    @author Tracy Usher
*/
class DGNFilterTool : public AlgTool, virtual public IFilterTool
{
public:

    // Standard Gaudi Tool constructor
    DGNFilterTool(const std::string& type, const std::string& name, const IInterface* parent);

    // After building it, destroy it
    ~DGNFilterTool();

    /// @brief Intialization of the tool
    StatusCode initialize();

    /// @brief Finalize method for the tool
    StatusCode finalize();

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

static ToolFactory<DGNFilterTool> s_factory;
const IToolFactory& DGNFilterToolFactory = s_factory;
//------------------------------------------------------------------------

DGNFilterTool::DGNFilterTool(const std::string& type, 
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
DGNFilterTool::~DGNFilterTool()
{
}

StatusCode DGNFilterTool::initialize()
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

        m_filterLibs = new DGNFilterLibsB1_0_8();
        const EFC_DB_Schema& master = obf->loadFilterLibs(m_filterLibs, m_verbosity);

        // Retrieve the mode to configure for normal running
        unsigned char modeToRun = master.filter.mode2cfg[0];

        m_filterId = obf->setupFilter(&master, modeToRun);

        if (m_filterId == -100)
        {
            log << MSG::ERROR << "Failed to initialize Diagnostic Filter" << endreq;
            return StatusCode::FAILURE;
        }

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

StatusCode DGNFilterTool::finalize ()
{
    StatusCode  status = StatusCode::SUCCESS;
    
    return status;
}

// This routine for dumping to log file the configuration being run
void DGNFilterTool::dumpConfiguration()
{
    MsgStream log(msgSvc(), name());

    log << MSG::INFO << "Diagnostic Filter Setup - FSW release " << m_filterLibs->FlightSoftwareRelease() << "\n"
        << "   Using configuration: " << m_filterLibs->getInstanceIdString(m_filterLibs->getMasterConfiguration().filter.mode2cfg[0]) << endreq;

    return;
}

// This defines the method called for end of event processing
void DGNFilterTool::eoeProcessing(EDS_fwIxb* ixb)
{
    // Retrieve the Gamma Filter Status Word
    EDS_rsdDsc*   rsdDsc     = ixb->rsd.dscs + m_filterId;
    unsigned char sb         = rsdDsc->sb;
    unsigned int* dscPtr     = (unsigned int*)rsdDsc->ptr;
    unsigned int  statusWord = *dscPtr++;

    // If we are running pass through mode then we need to manually set the veto bit
    // in the event their was a veto
    if (m_leakAllEvents)
    {
        if (statusWord & DFC_STATUS_M_VETO_DEF)
        {
            statusWord |= DFC_STATUS_M_VETOED;
            sb         |= EDS_RSD_SB_M_VETOED;
        }
    }

    // Retrieve the output status TDS container object
    SmartDataPtr<OnboardFilterTds::ObfFilterStatus> obfFilterStatus(m_dataSvc,"/Event/Filter/ObfFilterStatus");

    // Create a new DFC Status TDS sub object
    OnboardFilterTds::ObfDFCStatus* dfcStat = new OnboardFilterTds::ObfDFCStatus(rsdDsc->id, statusWord, sb);

    // Add it to the TDS object
    obfFilterStatus->addFilterStatus(OnboardFilterTds::ObfFilterStatus::DFCFilter, dfcStat);

    // Increment counters accordingly
    if((statusWord & DFC_STATUS_M_STAGE_GEM) != 0)      m_statusBits[0]++;

    if((statusWord & DFC_STATUS_M_GEM_CLASSES) != 0)    m_vetoBits[0]++;

    if((statusWord & DFC_STATUS_M_VETOED) != 0)         m_vetoBits[16]++;

    return;
}

// This for end of run processing
void DGNFilterTool::eorProcessing()
{
    MsgStream log(msgSvc(), name());

    // Output the bit frequency table
    log << MSG::INFO << "-- DFC Filter bit frequency table -- \n" 
        << "    Status Bit                         Value\n"
        << "    DFC_STATUS_M_STAGE_GEM             " << m_statusBits[0] << "\n"       
    
        << "    Veto Bit Summary" << "\n"
        << "    Trigger Name                           Count\n" << "\n"
        << "    DFC_STATUS_M_GEM_CLASSES           " << m_vetoBits[0] << "\n"
        << "    DFC_STATUS_M_VETOED                " << m_vetoBits[16] << "\n"
        << endreq;

    return;
}
