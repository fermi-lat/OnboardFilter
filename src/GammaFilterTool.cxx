/**  @file GammaFilterTool.cxx
    @brief implementation of class GammaFilterTool
    
  $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/GammaFilterTool.cxx,v 1.15 2008/06/26 12:50:58 usher Exp $
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
#include "GammaFilterCfgPrms.h"

// FSW includes go here
#include "FSWHeaders/CDM_pubdefs.h"
#include "EFC_DB/EFC_DB_schema.h"
#include "EFC_DB/EH_ids.h"
#include "GFC_DB/GFC_DB_schema.h"
#include "GFC_DB/GAMMA_DB_instance.h"
#include "EFC/GFC_status.h"

#include "FSWHeaders/EFC.h"
#include "LSE/LFR_key.h"

// FSW include but made local do to keyword usage
#include "FSWHeaders/EFC_sampler.h"

// Contains all info for a particular filter's release
#include "GammaFilterLibsB1-0-8.h"

// Useful stuff! 
#include <map>
#include <stdexcept>
#include <sstream>

/** @class GammaFilterTool
    @brief Manages the Gamma Filter
    @author Tracy Usher
*/
class GammaFilterTool : public AlgTool, virtual public IFilterTool
{
public:

    // Standard Gaudi Tool constructor
    GammaFilterTool(const std::string& type, const std::string& name, const IInterface* parent);

    // After building it, destroy it
    ~GammaFilterTool();

    /// @brief Intialization of the tool
    StatusCode initialize();

    /// @brief Finalize method for the tool
    StatusCode finalize();

    // Set Mode and Configuration for a given filter
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

    // Which filter configuration to run
    StringProperty    m_configToRun;

    // Configuring the Gamma Filter
    unsigned int      m_gamBitsToIgnore; // This sets a mask of gamma filter veto bits to ignore
    bool              m_runAllStages;    // Run all stages of filter for diagnostics...
    bool              m_leakAllEvents;   // If true then GSW running of Gamma Filter will leak all events

    // Keeping track of the veto bits (necessary if running all stages)
    unsigned int      m_filterVetoMask;  // This is what the filter wanted to use

    unsigned int      m_gamBitsOriginal; // If ignoring bits, save the original state

    // Changeable parameters for the gamma filter
    // See the file EFC/src/GFC_def.h for the definition of these variables
    unsigned int      m_Acd_TopSideEmax;
    unsigned int      m_Acd_TopSideFilterEmax;
    unsigned int      m_Acd_SplashEmax;
    unsigned int      m_Acd_SplashCount;

    unsigned int      m_Atf_Emax;

    unsigned int      m_Zbottom_Emin;

    unsigned int      m_Cal_Epass;
    unsigned int      m_Cal_Emin;
    unsigned int      m_Cal_Emax;
    unsigned int      m_Cal_Layer0RatioLo;
    unsigned int      m_Cal_Layer0RatioHi;

    unsigned int      m_Tkr_Row2Emax;
    unsigned int      m_Tkr_Row01Emax;
    unsigned int      m_Tkr_TopEmax;
    unsigned int      m_Tkr_ZeroTkrEmin;
    unsigned int      m_Tkr_TwoTkrEmax;
    unsigned int      m_Tkr_SkirtEmax;

    // Filter ID returned from EDS_fw after initialization
    int               m_handlerId;

    // Current mode and configuration
    unsigned short    m_curConfig;
    unsigned short    m_curMode;

    //****** This section contains various useful member variables
    // Counters to keep track of bit frequency during a given run
    int               m_statusBits[32]; //array to count # of times each bit in status word is set

    // Pointer to the filter library class with release specific information
    IFilterLibs*      m_filterLibs;

    /// Pointer to the Gaudi data provider service
    IDataProviderSvc* m_dataSvc;

    /// MootSvc for filter configurations
    IMootSvc*         m_mootSvc;
};

static ToolFactory<GammaFilterTool> s_factory;
const IToolFactory& GammaFilterToolFactory = s_factory;
//------------------------------------------------------------------------

GammaFilterTool::GammaFilterTool(const std::string& type, 
                                 const std::string& name, 
                                 const IInterface* parent) :
                                 AlgTool(type, name, parent)
                               , m_filterVetoMask(0)
                               , m_gamBitsOriginal(0)
                               , m_curConfig(0)
                               , m_curMode(EFC_DB_MODE_K_NORMAL)
                               , m_filterLibs(0)
                               , m_mootSvc(0)
{
    //Declare the additional interface
    declareInterface<IFilterTool>(this);

    // As of April 21, 2008 there are no bits turned off in Gleam version of Gamma Filter
    unsigned gamBitsToIgnore = 0;

    // declare properties with setProperties calls
    // See the file EFC/src/GFC_def.h for the definition of these variables
    // ****DO NOT CHANGE unless you know what you are doing ! *****
    // Parameter: RunAllStages
    // Default is not run the filter in data mode, we do NOT run all stages
    declareProperty("RunAllStages",          m_runAllStages          = false);
    // Paramter: LeakAllEvents
    // Default is TO "leak" (pass status/filter information) all events (DEPRECATED)
    declareProperty("LeakAllEvents",         m_leakAllEvents         = false);
    // Parameter: Configuration
    // Overrides the default configuration given in the Master Configuration file
    declareProperty("Configuration",         m_configToRun           = "");
    // Parameter: GamFilterMask
    // Allows override of the "veto mask" the gamma filter uses to determine whether to veto an event
    declareProperty("GamFilterMask",         m_gamBitsToIgnore       = gamBitsToIgnore);

    declareProperty("Acd_TopSideEmax",       m_Acd_TopSideEmax       = 0xFFFFFFFF);
    declareProperty("Acd_TopSideFilterEmax", m_Acd_TopSideFilterEmax = 0xFFFFFFFF);
    declareProperty("Acd_SplashEmax",        m_Acd_SplashEmax        = 0xFFFFFFFF);
    declareProperty("Acd_SplashCount",       m_Acd_SplashCount       = 0xFFFFFFFF);

    declareProperty("Atf_Emax",              m_Atf_Emax              = 0xFFFFFFFF);

    declareProperty("Zbottom_Emin",          m_Zbottom_Emin          = 0xFFFFFFFF);

    declareProperty("Cal_Epass",             m_Cal_Epass             = 0xFFFFFFFF);
    declareProperty("Cal_Emin",              m_Cal_Emin              = 0xFFFFFFFF);
    declareProperty("Cal_Emax",              m_Cal_Emax              = 0xFFFFFFFF);
    declareProperty("Cal_Layer0RatioLo",     m_Cal_Layer0RatioLo     = 0xFFFFFFFF);
    declareProperty("Cal_Layer0RatioHi",     m_Cal_Layer0RatioHi     = 0xFFFFFFFF);

    declareProperty("Tkr_Row2Emax",          m_Tkr_Row2Emax          = 0xFFFFFFFF);
    declareProperty("Tkr_Row01Emax",         m_Tkr_Row01Emax         = 0xFFFFFFFF);
    declareProperty("Tkr_TopEmax",           m_Tkr_TopEmax           = 0xFFFFFFFF);
    declareProperty("Tkr_ZeroTkrEmin",       m_Tkr_ZeroTkrEmin       = 0xFFFFFFFF);
    declareProperty("Tkr_TwoTkrEmax",        m_Tkr_TwoTkrEmax        = 0xFFFFFFFF);
    declareProperty("Tkr_SkirtEmax",         m_Tkr_SkirtEmax         = 0xFFFFFFFF);

    declareProperty("verbosity",             m_verbosity             = 0);

    // zero our counters
    memset(m_statusBits, 0, 32*sizeof(int));    

    return;
}
//------------------------------------------------------------------------
GammaFilterTool::~GammaFilterTool()
{
}

StatusCode GammaFilterTool::initialize()
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

        // Create the object which contains the release specific information for the Gamma Filter
        // This includes the library containing the filter code as well as the libraries which 
        // define the running configurations. 
        m_filterLibs = new GammaFilterLibsB1_0_8();

        // Load the necessary libraries and obtain the master configuration file
        const EFC_DB_Schema& master = obf->loadFilterLibs(m_filterLibs, m_verbosity);

        // Check to see what mode we want to run... (if a different one requested via JO parameter)
        if (m_configToRun.value() != "")
        {
            unsigned short int configId = m_filterLibs->getInstanceId(m_configToRun.value());

            if (configId != m_filterLibs->getMasterConfiguration().filter.mode2cfg[m_curMode]) 
                m_filterLibs->getMasterConfiguration().filter.mode2cfg[m_curMode] = configId;
        }

        // Default value of current config
        m_curConfig = m_filterLibs->getMasterConfiguration().filter.mode2cfg[m_curMode];

        // Set up the filter including the configuration to run
        m_handlerId = obf->setupFilter(&master, m_curConfig);

        // Hmm... should replace this with a try-catch?
        if (m_handlerId == -100)
        {
            log << MSG::ERROR << "Failed to initialize Gamma Filter" << endreq;
            return StatusCode::FAILURE;
        }

        // Bit mask for this filter
        unsigned int target = obf->getFilterTargetMask(master.filter.id);

        // Are we using moot and is this an active filter?
        bool activeFilter = false;

        // If we have moot we need to get the list of active filters and see if we are one of them
        if (m_mootSvc)
        {
            std::vector<CalibData::MootFilterCfg> filterCfgVec;
            unsigned int filterCnt = m_mootSvc->getActiveFilters(filterCfgVec);
            
            // Loop through the returned list of active filters
            for(std::vector<CalibData::MootFilterCfg>::const_iterator filterIter = filterCfgVec.begin();
                filterIter != filterCfgVec.end(); filterIter++)
            {
                // Are we active?
                if (filterIter->getSchemaId() == m_filterLibs->FilterSchema())
                {
                    activeFilter = true;
                    log << MSG::INFO << "Moot has filter " <<  filterIter->getName() << " as active" << endreq;
                    break;
                }
            }
        }

        // Loop through and associate configurations to modes
        for (int modeIdx = 0; modeIdx < EFC_DB_MODE_K_CNT; modeIdx++)
        {
            // Default is the configuration from the master configuration file
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

        // Use set mode to do the rest here
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

StatusCode GammaFilterTool::finalize ()
{
    StatusCode  status = StatusCode::SUCCESS;
    
    return status;
}

// Set Mode and Configuration for a given filter
void GammaFilterTool::setMode(unsigned int mode)
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

    // Set up the object allowing one to change gamma filter parameters
    // But only if running outside of Moot
    if (!m_mootSvc)
    {
        GammaFilterCfgPrms gamParms;
        gamParms.set_Acd_TopSideEmax(m_Acd_TopSideEmax);
        gamParms.set_Acd_TopSideFilterEmax(m_Acd_TopSideFilterEmax);
        gamParms.set_Acd_SplashEmax(m_Acd_TopSideFilterEmax);
        gamParms.set_Acd_SplashCount(m_Acd_TopSideFilterEmax);
        gamParms.set_Atf_Emax(m_Atf_Emax);
        gamParms.set_Zbottom_Emin(m_Zbottom_Emin);
        gamParms.set_Cal_Epass(m_Cal_Epass);
        gamParms.set_Cal_Emin(m_Cal_Emin);
        gamParms.set_Cal_Emax(m_Cal_Emax);
        gamParms.set_Cal_Layer0RatioLo(m_Cal_Layer0RatioLo);
        gamParms.set_Cal_Layer0RatioHi(m_Cal_Layer0RatioHi);
        gamParms.set_Tkr_Row2Emax(m_Tkr_Row2Emax);
        gamParms.set_Tkr_Row01Emax(m_Tkr_Row01Emax);
        gamParms.set_Tkr_TopEmax(m_Tkr_TopEmax);
        gamParms.set_Tkr_ZeroTkrEmin(m_Tkr_ZeroTkrEmin);
        gamParms.set_Tkr_TwoTkrEmax(m_Tkr_TwoTkrEmax);
        gamParms.set_Tkr_SkirtEmax(m_Tkr_SkirtEmax);

        // Get the Gamma Filter parameter block
        void* gammaCfgPrms = obf->getFilterPrm(masterId, EFC_OBJECT_K_FILTER_PRM);

        // This will modify any configuration parameters that are not 0xFFFFFFFF
        gamParms.setCfgPrms(gammaCfgPrms);
    }

    // If we are disabling vetoes, or if we are trying to run all stages of filter, then modify here
    if (m_runAllStages || m_gamBitsToIgnore)
    {
        // Modify the veto mask is requested (this means we are running "pass through" mode)
        EFC_sampler* sampler = (EFC_sampler*)obf->getFilterPrm(masterId, EFC_OBJECT_K_SAMPLER);

        // If running all stages of the filter then get the veto mask (since we'll need to apply it in the end)
        if (m_runAllStages)
        {
            // What did the filter want to use as a veto mask?
            m_filterVetoMask = sampler->classes.enabled.all;

            // Set the filter's veto mask to zero so it won't reject events
            sampler->classes.enabled.all = 0;
        }
        // Otherwise, modify the bits to ignore in the filter's veto mask
        else sampler->classes.enabled.all &= ~m_gamBitsToIgnore;
    }

    // And, of course, reset the mode
    m_curMode = mode;

    return;
}

// This routine for dumping to log file the configuration being run
void GammaFilterTool::dumpConfiguration()
{
    MsgStream log(msgSvc(), name());

    unsigned short mode2cfg = m_filterLibs->getMasterConfiguration().filter.mode2cfg[0];
    std::string    config   = m_filterLibs->getInstanceIdString(mode2cfg);

    log << MSG::INFO << "Gamma Filter Setup - FSW release " << m_filterLibs->FlightSoftwareRelease() << "\n"
        << "   Using configuration: " << config.data() << " for mode = 0" << endreq;

    return;
}

// This defines the method called for end of event processing
void GammaFilterTool::eoeProcessing(EDS_fwIxb* ixb)
{
    // Retrieve the Gamma Filter Status Word
    EDS_rsdDsc*   rsdDsc        = ixb->rsd.dscs + m_handlerId;
    unsigned char sb            = rsdDsc->sb;
    unsigned int* dscPtr        = (unsigned int*)rsdDsc->ptr;
    unsigned int  oldStatusWord = *dscPtr++;
    unsigned int  statusWord    = oldStatusWord;
    unsigned int  energy        = *dscPtr;

    // If we are leaking all events then we need to manually set the veto bit
    // in the event their was a veto
    if (m_runAllStages)
    {
        // Remove any undesired veto bits from the "old school" status word **** 
        oldStatusWord = ~m_gamBitsToIgnore & oldStatusWord;

        // This attempts to emulate the hi energy pass after the fact, allowing us to 
        // continue running the full Gamma Filter processing... The prescription is as 
        // defined by Patrck Smith in his confluence discussion page at 
        // https://confluence.slac.stanford.edu/display/DC2/2007/12/12/Onboard+Filter+-+20GeV+Threshold+verification
        // Any coding errors are mine (TU 12/12/07)
        // This is now implemented in FSW B1-0-8 so remove from here
        //bool hiEPass =   (oldStatusWord & GFC_STATUS_M_HI_ENERGY) // if this bit is set then we want to pass the event
        //             && !(oldStatusWord & (GFC_STATUS_M_SPLASH_0 | GFC_STATUS_M_NOCALLO_FILTER_TILE)); // as long as these bits aren't set

        // If any unmasked veto bits are set then set the general event vetoed bit 
        if (oldStatusWord & m_filterVetoMask) // && !hiEPass)
        {
            oldStatusWord |= GFC_STATUS_M_VETOED;
            statusWord    |= GFC_STATUS_M_VETOED;
            sb            |= EDS_RSD_SB_M_VETOED;
        }
    }

    // Retrieve the output status TDS container object
    SmartDataPtr<OnboardFilterTds::ObfFilterStatus> obfFilterStatus(m_dataSvc,"/Event/Filter/ObfFilterStatus");

    // Create a new Gamma Status TDS sub object
    OnboardFilterTds::ObfGammaStatus* gamStat = new OnboardFilterTds::ObfGammaStatus(rsdDsc->id, statusWord, sb, 0, energy);

    // Add it to the TDS object
    obfFilterStatus->addFilterStatus(OnboardFilterTds::ObfFilterStatus::GammaFilter, gamStat);

    // Accumulate the status bit hits
    for(int ib = 0; ib < 32; ib++) if (statusWord & 1 << ib) m_statusBits[ib]++;

    return;
}

// This for end of run processing
void GammaFilterTool::eorProcessing()
{
    MsgStream log(msgSvc(), name());

    // Descriptor for each bit
    std::string statusBitDesc[] = {"GFC_STATUS_M_GEM_THROTTLE       ",
                                   "GFC_STATUS_M_GEM_TKR            ",
                                   "GFC_STATUS_M_GEM_CALLO          ",
                                   "GFC_STATUS_M_GEM_CALHI          ",
                                   "GFC_STATUS_M_GEM_CNO            ",
                                   "GFC_STATUS_M_ACD_TOP            ",
                                   "GFC_STATUS_M_ACD_SIDE           ",
                                   "GFC_STATUS_M_ACD_SIDE_FILTER    ",
                                   "GFC_STATUS_M_TKR_EQ_1           ",
                                   "GFC_STATUS_M_TKR_GE_2           ",
                                   "GFC_STATUS_M_HI_ENERGY          ",
                                   "GFC_STATUS_M_RSVD_11            ",
                                   "GFC_STATUS_M_RSVD_12            ",
                                   "GFC_STATUS_M_RSVD_13            ",
                                   "GFC_STATUS_M_ERR_CTB            ",
                                   "GFC_STATUS_M_TKR_LT_2_ELO       ",
                                   "GFC_STATUS_M_TKR_SKIRT          ",
                                   "GFC_STATUS_M_TKR_EQ_0           ",
                                   "GFC_STATUS_M_TKR_ROW2           ",
                                   "GFC_STATUS_M_TKR_ROW01          ",
                                   "GFC_STATUS_M_TKR_TOP            ",
                                   "GFC_STATUS_M_ZBOTTOM            ",
                                   "GFC_STATUS_M_EL0_ETOT_HI        ",
                                   "GFC_STATUS_M_EL0_ETOT_LO        ",
                                   "GFC_STATUS_M_SIDE               ",
                                   "GFC_STATUS_M_TOP                ",
                                   "GFC_STATUS_M_SPLASH_1           ",
                                   "GFC_STATUS_M_E350_FILTER_TILE   ",
                                   "GFC_STATUS_M_E0_TILE            ",
                                   "GFC_STATUS_M_SPLASH_0           ",
                                   "GFC_STATUS_M_NOCALLO_FILTER_TILE",
                                   "GFC_STATUS_M_VETOED             " };

    // Output the bit frequency table
    log << MSG::INFO << "-- Gamma Filter Status Word bit frequency table -- \n";

    for(int ib = 0; ib < 32; ib++)
    {
        log << "    " << statusBitDesc[ib] << " = " << m_statusBits[ib] << "\n";
    }

    log  << endreq;

    return;
}
