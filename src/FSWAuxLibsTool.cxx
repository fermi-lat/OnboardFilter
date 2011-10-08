/**  @file FSWAuxLibsTool.cxx
    @brief implementation of class FSWAuxLibsTool
    
  $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/FSWAuxLibsTool.cxx,v 1.11 2011/10/05 21:50:05 jrb Exp $  
*/

#include "IFilterTool.h"

#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/GaudiException.h" 

#include "facilities/Util.h"

// Interface to EDS package here
#include "ObfInterface.h"

// FSW includes go here
#ifdef OBF_1_1_3
#include "FSWHeaders/CDM_pubdefs.h"
#endif

#ifdef OBF_3_0_0
#include "CDM/CDM_pubdefs.h"
#endif

// Useful stuff! 
#include <stdexcept>
#include <sstream>

/** @class FSWAuxLibsTool
    @brief Manages the Gamma Filter
    @author Tracy Usher
*/
class FSWAuxLibsTool : public AlgTool, virtual public IFilterTool
{
public:

    // Standard Gaudi Tool constructor
    FSWAuxLibsTool(const std::string& type, const std::string& name, const IInterface* parent);

    // After building it, destroy it
    ~FSWAuxLibsTool();

    /// @brief Intialization of the tool
    StatusCode initialize();

    /// @brief Finalize method for the tool
    StatusCode finalize();

    // Set Mode for a given filter
    void setMode(unsigned int mode) {return;}

    // This defines the method called for end of event processing
    virtual void eoeProcessing(EDS_fwIxb* ixb);

    // This for end of run processing
    virtual void eorProcessing();

    // Dump out the running configuration
    void dumpConfiguration();

private:

    // Private function to load FSW libraries
    bool loadLibrary (std::string libraryName, std::string libraryPath="", int verbosity=0);

    //****** This section for defining JO parameters
    // File name for peds/gains
    std::string  m_PathName_Pedestals;
    std::string  m_FileName_Pedestals;
    std::string  m_PathName_Gains;
    std::string  m_FileName_Gains;

    //****** This section contains various useful member variables
};

static ToolFactory<FSWAuxLibsTool> s_factory;
const IToolFactory& FSWAuxLibsToolFactory = s_factory;
//------------------------------------------------------------------------

FSWAuxLibsTool::FSWAuxLibsTool(const std::string& type, 
                                 const std::string& name, 
                                 const IInterface* parent) :
                                 AlgTool(type, name, parent)
{
    //Declare the additional interface
    declareInterface<IFilterTool>(this);

    // As of April 21, 2008 there are no bits turned off in Gleam version of Gamma Filter
    unsigned gamBitsToIgnore = 0;

    // declare properties with setProperties calls
    // See the file EFC/src/GFC_def.h for the definition of these variables
    // ****DO NOT CHANGE unless you know what you are doing ! *****
    //#ifndef SCons
    declareProperty("PathNamePeds",     m_PathName_Pedestals = "$(OBFCOP_DBBINDIR)/cal_db_pedestals");
    //#else
    //    declareProperty("PathNamePeds",     m_PathName_Pedestals = "$(OBFLDPATH)");
    //#endif

    declareProperty("FileNamePeds",     m_FileName_Pedestals = "cal_db_pedestals");
    declareProperty("PathNameGains",    m_PathName_Gains     = "$(OBFCOG_DBBINDIR)/cal_db_gains");

    declareProperty("FileNameGains",    m_FileName_Gains     = "cal_db_gains");

    return;
}
//------------------------------------------------------------------------
FSWAuxLibsTool::~FSWAuxLibsTool()
{
}

StatusCode FSWAuxLibsTool::initialize()
{
    StatusCode sc   = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());

    // Set the properties
    setProperties();

    try
    {
        // Get ObfInterface pointer
        ObfInterface* obf = ObfInterface::instance();

        // Load the correct calibration libraries
        std::string calPedFile = m_FileName_Pedestals;
        std::string calPedPath = m_PathName_Pedestals;         // + "/" + calPedFile;

        log << MSG::INFO << "Loading CAL pedestal file: " << calPedFile << ", path: " << calPedPath << endreq;

        obf->loadLibrary(calPedFile, calPedPath);
    
        std::string calGainFile = m_FileName_Gains;
        std::string calGainPath = m_PathName_Gains;            // + "/" + calGainFile;

        log << MSG::INFO << "Loading CAL gain file: " << calGainFile << ", path: " << calGainPath << endreq;

        obf->loadLibrary(calGainFile, calGainPath);

        // Load the Gleam geometry for fsw
        obf->loadLibrary ("geo_db_data", "$(OBFGGF_DBBINDIR)/geo_db_data");
    }
    catch(ObfInterface::ObfException& obfException)
    {
        log << MSG::ERROR << obfException.m_what << endreq;
        return StatusCode::FAILURE;
    }

    return sc;
}

StatusCode FSWAuxLibsTool::finalize ()
{
    StatusCode  status = StatusCode::SUCCESS;
    
    return status;
}

// This routine for dumping to log file the configuration being run
void FSWAuxLibsTool::dumpConfiguration()
{
    MsgStream log(msgSvc(), name());

    return;
}

// This defines the method called for end of event processing
void FSWAuxLibsTool::eoeProcessing(EDS_fwIxb* ixb)
{
    return;
}

// This for end of run processing
void FSWAuxLibsTool::eorProcessing()
{
    return;
}
