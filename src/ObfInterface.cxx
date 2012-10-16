
#include "ObfInterface.h" 

#include "IFilterTool.h"
#include "IFilterCfgPrms.h"
#include "IFilterLibs.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sstream>

#include "EbfWriter/Ebf.h"

#include "EFC_DB/EFC_DB_schema.h"
#include "GFC_DB/GAMMA_DB_instance.h"
#include "XFC_DB/MFC_DB_schema.h"
#include "XFC_DB/MIP_DB_instance.h"
#include "XFC_DB/HFC_DB_schema.h"
#include "XFC_DB/HIP_DB_instance.h"
#include "XFC_DB/DFC_DB_schema.h"
#include "XFC_DB/DGN_DB_instance.h"

#ifdef OBF_B1_1_3
#include "FSWHeaders/EFC.h"
#endif
#if defined(OBF_B3_0_0) || defined(OBF_B3_1_0)
#include "EFC/EFC.h"
#include "EFC/EFR_key.h"
#include "CDM/CDM_pubdefs.h"
#endif

#ifdef OBF_B1_1_3
#include "LSE/LFR_key.h"
#include "FSWHeaders/CDM_pubdefs.h"
#endif

#include "CAB/CAB_lookupPub.h"

#include "EDS/io/EBF_evts.h"
#include "EDS/EBF_edw.h"
#include "EDS/LCBV.h"

#include "GaudiKernel/MsgStream.h"
#include "facilities/Util.h"

/* ---------------------------------------------------------------------- */

// Forward declarations
// This is the callback for extracting the results of running filters
static void         extractFilterInfo (EOVCallBackParams* callBackParams, EDS_fwIxb *ixb);
// Dummy notify routine
static int          dummyNotify(void *prm, unsigned int mode, unsigned int active, const unsigned char cfgs[32], void *rsdTbl);
// This provides an event "free" routine to EDS_fw 
static unsigned int dummyFreeRtn (void* prm, EBF_pkt *from, EBF_pkt *to);
// This provides an event flushing routine
static int          myOutputFlush (void *unused, int reason);
// Enables us to run the filters in pass-through mode
static int          passThrough (void *unused, unsigned int nbytes, EBF_pkt *pkt, EBF_siv siv, EDS_fwIxb ixb);

/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
// Local utility class for handling output call back routines at end of event
class EOVCallBackParams
{
public:
//    EOVCallBackParams() : m_statParms(0), m_callBackParm(0) {m_callBackVec.clear();}
    EOVCallBackParams() : m_statParms(0) {m_callBackVec.clear();}
    ~EOVCallBackParams() {}

    std::ostringstream     m_defaultStream;
    void*                  m_statParms;
    OutputRtnVec           m_callBackVec;
};

ObfInterface* ObfInterface::m_instance = 0;

ObfInterface* ObfInterface::instance()
{
    if (!m_instance) m_instance = new ObfInterface();
    return m_instance;
}

ObfInterface::ObfInterface() : m_eventCount(0), m_eventProcessed(0), m_eventBad(0), m_levels(0), m_verbosity(0)
{
    // Call back routine control
    m_callBack = new EOVCallBackParams();

    m_schemaToEnum[GAMMA_DB_SCHEMA] = EH_ID_K_GAMMA;
    m_schemaToEnum[MIP_DB_SCHEMA]   = EH_ID_K_MIP;
    m_schemaToEnum[HIP_DB_SCHEMA]   = EH_ID_K_HIP;
    m_schemaToEnum[DGN_DB_SCHEMA]   = EH_ID_K_DGN;

    // Provide a mechanism to convert an instance typedef to a string (for convenience)
    m_modeEnumToStringMap.clear();

    m_modeEnumToStringMap[EFC_DB_MODE_K_NORMAL]   = "EFC_DB_MODE_K_NORMAL";
    m_modeEnumToStringMap[EFC_DB_MODE_K_TOO]      = "EFC_DB_MODE_K_TOO";
    m_modeEnumToStringMap[EFC_DB_MODE_K_ARR]      = "EFC_DB_MODE_K_ARR";
    m_modeEnumToStringMap[EFC_DB_MODE_K_RSVD3]    = "EFC_DB_MODE_K_RSVD3";
    m_modeEnumToStringMap[EFC_DB_MODE_K_RSVD4]    = "EFC_DB_MODE_K_RSVD4";
    m_modeEnumToStringMap[EFC_DB_MODE_K_RSVD5]    = "EFC_DB_MODE_K_RSVD5";
    m_modeEnumToStringMap[EFC_DB_MODE_K_RSVD6]    = "EFC_DB_MODE_K_RSVD6";
    m_modeEnumToStringMap[EFC_DB_MODE_K_RSVD7]    = "EFC_DB_MODE_K_RSVD7";

    // go the other way
    m_modeStringToEnumMap.clear();

    m_modeStringToEnumMap["EFC_DB_MODE_K_NORMAL"] = EFC_DB_MODE_K_NORMAL;
    m_modeStringToEnumMap["EFC_DB_MODE_K_TOO"]    = EFC_DB_MODE_K_TOO;
    m_modeStringToEnumMap["EFC_DB_MODE_K_ARR"]    = EFC_DB_MODE_K_ARR;
    m_modeStringToEnumMap["EFC_DB_MODE_K_RSVD3"]  = EFC_DB_MODE_K_RSVD3;
    m_modeStringToEnumMap["EFC_DB_MODE_K_RSVD4"]  = EFC_DB_MODE_K_RSVD4;
    m_modeStringToEnumMap["EFC_DB_MODE_K_RSVD5"]  = EFC_DB_MODE_K_RSVD5;
    m_modeStringToEnumMap["EFC_DB_MODE_K_RSVD6"]  = EFC_DB_MODE_K_RSVD6;
    m_modeStringToEnumMap["EFC_DB_MODE_K_RSVD7"]  = EFC_DB_MODE_K_RSVD7;

    /* Allocate and initialize the Event Data Services framework */
    /* - NOTE: these are C structures not C++ classes */
    m_edsFw = (EDS_fw*) malloc(EDS_fwSizeof ());

    EDS_fwConstruct (m_edsFw, NULL, NULL, NULL);
    EDS_fwFreeSet   (m_edsFw, (LCBV_pktsRngFreeCb)dummyFreeRtn, NULL);  

    // Register the "post" routine to handle end of event processing
    EDS_fwPostRegister   (m_edsFw,
                          EDS_FW_K_POST_0,
                         (EDS_fwPostStartRtn)NULL,
                         (EDS_fwPostWriteRtn)extractFilterInfo,
                         (EDS_fwPostNotifyRtn)dummyNotify,
                         (EDS_fwPostFlushRtn)myOutputFlush,
                          m_callBack);

    // Post notification
    EDS_fwPostNotify(m_edsFw, EDS_FW_M_POST_0, EFC_DB_MODE_K_NORMAL);

    EDS_fwPostChange(m_edsFw, EDS_FW_M_POST_0,  EDS_FW_M_POST_0  );

    // Clear vector of filter pointers
    m_filterMap.clear();

    return;
}

ObfInterface::~ObfInterface()
{
    free(m_edsFw);

    return;
}

int ObfInterface::setupFilter(const EFC_DB_Schema* schema,
                              unsigned short int   configIndex)
{
    int filterId = -100;

    // Attempt to trap any dprintf or printf output in fsw code
    // Create a local buffer and store the current state of stdout
#ifdef _WIN32__
    char buf[100000];
    buf[0] = 0;
    FILE myFile = *stdout;

    // Redirect output to our local buffer
    setvbuf(stdout, buf, _IOFBF, 100000);
#endif

    // Retrieve the key to our filter
#if defined(OBF_B3_0_0) || defined(OBF_B3_1_0)
    unsigned int key = EFR_keyGet (CDM_findDatabase (schema->filter.id, configIndex), 0);
#endif
#ifdef OBF_B1_1_3
    unsigned int key = LFR_keyGet (CDM_findDatabase (schema->filter.id, configIndex), 0);
#endif


    // Get a pointer to the "services" structure 
    const char*                            fnd      = schema->eds.get;
    EDS_DB_handlerServicesGet              get      = (const EDS_DB_HandlerConstructServices*(*)(void*)) CMX_lookupSymbol(fnd);
    const EDS_DB_HandlerConstructServices *services = (EDS_DB_HandlerConstructServices *)get (0);

    // Look up the target mask for this filter
    unsigned int target = m_schemaToEnum[schema->filter.id];

    // Allocate memory for the Filter and then initialize it
    EFC* filter = (EFC *) malloc(services->sizeOf (schema, NULL));
    const EDS_fwHandlerServicesX1* gfcService = services->construct (filter, target, schema, key, NULL, m_edsFw);
    EFC_report (filter, -1);

    /* Register and enable the filter */
    filterId = EDS_fwHandlerServicesRegisterX1 (m_edsFw,  target, gfcService, filter);

    // Keep track of the pointer for deletion at end of processing
    m_filterMap[schema->filter.id] = filter;

    // Associate a configuration with our run mode
    //EDS_fwHandlerAssociate(m_edsFw, EDS_FW_MASK(target), EFC_DB_MODE_K_NORMAL, configIndex );
    //EDS_fwHandlerAssociate(m_edsFw, target, EFC_DB_MODE_K_NORMAL, configIndex );

    // enable the filter
    //EDS_fwHandlerChange(m_edsFw, EDS_FW_MASK(target), EDS_FW_MASK(target) );
    //EDS_fwHandlerChange(m_edsFw, target, target );

    // Select the mode we want
    //EDS_fwHandlerSelect(m_edsFw, EDS_FW_MASK(target), EFC_DB_MODE_K_NORMAL);
    //EDS_fwHandlerSelect(m_edsFw, target, EFC_DB_MODE_K_NORMAL);

    // Restore the output stream
#ifdef _WIN32__
    *stdout = myFile;
#endif

    return filterId;
}

/// Associate a given (set of) filter(s) configuration with a mode
unsigned int ObfInterface::associateConfigToMode(unsigned int targets, unsigned int mode, unsigned int configuration)
{
    // Associate a configuration with our run mode
    return EDS_fwHandlerAssociate(m_edsFw, targets, mode, configuration );
}


/// Enable/Disable filter(s)
unsigned int ObfInterface::enableDisableFilter(unsigned int targets, unsigned int mask)
{
    // enable the filter
    return EDS_fwHandlerChange(m_edsFw, targets, mask );
}

/// Set current mode for a given (set of) filter(s)
unsigned int ObfInterface::selectFiltermode(unsigned int targets, unsigned int mode)
{
    // Select the mode we want
    return EDS_fwHandlerSelect(m_edsFw, targets, mode);
}
    
/// Return a "target" mask given a filter's schema id
unsigned int ObfInterface::getFilterTargetMask(unsigned short int schemaId) const
{
    SchemaToEnumMap::const_iterator idIter = m_schemaToEnum.find(schemaId);

    if (idIter != m_schemaToEnum.end()) return EDS_FW_MASK(idIter->second);

    return 0;
}

void* ObfInterface::getFilterPrm(unsigned short filterSchemaId, int type)
{
    void* filterPrms = 0;

    FilterMap::iterator mapIter = m_filterMap.find(filterSchemaId);

    if (mapIter != m_filterMap.end()) filterPrms = EFC_get(mapIter->second, (EFC_OBJECT_K)type);

    return filterPrms;
}


//void ObfInterface::setEovOutputCallBack(OutputRtn* outRtn)
void ObfInterface::setEovOutputCallBack(IFilterTool* outRtn)
{
    if (outRtn) m_callBack->m_callBackVec.push_back(outRtn);

    return;
}

bool ObfInterface::setupPassThrough(void* prm)
{
    //  Register and enable the "passThrough" handler.  This handler is only
    //  used to set EDS_FW_FN_M_POST_0, which allows the "post" routine to get filter
    //  info from the IXB block
    int id = EDS_fwHandlerRegister (m_edsFw,
             EH_ID_K_PASS_THRU,
             EDS_FW_OBJ_M_DIR,
             EDS_FW_FN_M_DIR | EDS_FW_FN_M_POST_0,
             1000,
             (EDS_fwHandlerProcessRtn    )passThrough,
             (EDS_fwHandlerAssociateRtn  )NULL,
             (EDS_fwHandlerSelectRtn     )NULL,
             (EDS_fwHandlerFlushRtn      )NULL,
             (EDS_fwHandlerDestructRtn   )NULL,
             NULL);

    // enable the pass through
    EDS_fwHandlerChange(m_edsFw, EDS_FW_MASK(id), EDS_FW_MASK(id) );

    return true;
}
/* ---------------------------------------------------------------------- */
/* ====================================================================== */

/* ---------------------------------------------------------------------- *//*!

  \fn     static int loadLib (const char *library_name, int verbose)
  \brief  Dynamically loads the specified configuration library
  \retval 0, success
  \retval -1, failure

  \param  library_name The name of the library to load
  \param  verbose      The verbosity level
                                                                          */
/* ---------------------------------------------------------------------- */
bool ObfInterface::loadLibrary (std::string libraryName, std::string libraryPath, int verbosity)
{
  // For debugging, ignore input verbosity arg.  Always set to 1
    verbosity = 1;   
    if (verbosity > 0) printf (" Loading: %s", libraryName.c_str());

    std::string fullFileName = libraryPath != "" ? libraryPath + "/" : "";

    // Platform dependent section to paste it all together
    #ifdef WIN32
        fullFileName += libraryName + ".dll";
    #else
        fullFileName += "lib" + libraryName + ".so";
    #endif

    // Expand any environment variables that might be in the name
    facilities::Util::expandEnvVar(&fullFileName);

    // call CDM to load the library
    // Note that currently (12/4/06) cal_db will be zero even when library loads
    CDM_Database* cal_db = CDM_loadDatabase (fullFileName.c_str(), 0);

    if (verbosity > 0) printf (cal_db == NULL ? " (FAILED)\n\n" : " (succeeded)\n\n");

    return cal_db != NULL;
}

const EFC_DB_Schema& ObfInterface::loadFilterLibs(IFilterLibs* filterLibs, int verbosity)
{
    const std::string basePath = filterLibs->ConfigBasePath() + "/";

    // Load the library containing the filter code
    loadLibrary (filterLibs->FilterLibName(), filterLibs->FilterLibPath(), verbosity);

    // Locate the Gamma filter's master configuration file
    loadLibrary(filterLibs->MasterConfigName(), basePath + filterLibs->MasterConfigName(), m_verbosity);

    // Find the Master Schema for the desired filter and get it ready for action 
    const EFC_DB_Schema* masterSchema = EFC_lookup (filterLibs->FilterSchema(), filterLibs->MasterConfigInstance());

    // Make a local non-constant copy so that we can, if need be, modify things
    filterLibs->setMasterConfiguration(masterSchema);
    EFC_DB_Schema& master = filterLibs->getMasterConfiguration();

    // If no output asked for, reset to make sure we always get the status word
    if (!master.filter.rsd.nbytes)
    {
        master.filter.rsd.nbytes = 4;
    }

    // Load any dependent libraries called for by the master configuration
    for (int idx = 0; idx < master.filter.cnt; idx++)
    {
        SchemaPair pair(master.filter.id, master.filter.instances[idx]);
            
        //std::string& fileName = m_idToFile[pair];
        IdToFileMap::const_iterator idIter = filterLibs->getIdToFileMap().find(pair);
        if (idIter == filterLibs->getIdToFileMap().end())
        {
            std::stringstream errorString;
            errorString << "Unable to find file for configuration: " << pair.second;
            throw ObfException(errorString.str());
        }

        const std::string& fileName = idIter->second;
        //#ifndef SCons
        loadLibrary(fileName, basePath + fileName, verbosity);
        //#else
        //loadLibrary(fileName, filterLibs->FilterLibPath(), verbosity);
        //#endif

        // Look up the configruation
        const EFC_DB_Schema* thisSchema = EFC_lookup (filterLibs->FilterSchema(), pair.second);
        int j = 0;
    }

    return master;
}


/* ---------------------------------------------------------------------- *//*!

   \fn         int doFilter (const FilterRto *rto)
   \brief      Common routine to do the filtering
   \param rto  The run time options or control parameters, assumed to be
               already filled in.
   \return     Status
                                                                          */
/* ---------------------------------------------------------------------- */
unsigned int ObfInterface::filterEvent(EbfWriterTds::Ebf* ebfData)
{
    // Set everything on?
    unsigned int filterStatus = -1;

    // Event counter 
    m_eventCount++;
    
    // The following few lines will put the pointer to the data in 
    // into a form which can be eaten by the fsw data handler
    unsigned int  length;
    char         *data = ebfData->get(length);

    // This can't happen (flw!)
    if(length==0) throw ObfException("Warning: Event has no EBF data. Ignoring...");

    // The data variable points to the head of our EBF_ptks 
    // What follows here will create the EBF_pkts object in C++ (as opposed to C)
    unsigned int* dataPtr = (unsigned int*)data;

    // The union of the long long and ps
    EBF_pkts_u    pktsu;

    // Size comes from the length returned above
    pktsu.ps.size = length;
    pktsu.ps.pkt  = (EBF_pkt*)(dataPtr);

    // Now convert to a EBF_pkts struct
    EBF_pkts      pkts = pktsu.p;

    // Get a warm and fuzzy feeling by doing a simple check...
    /*********************************************************************
    // No longer needed
    int numPkts = EBF_pktsCount (pkts);
    if (numPkts < 1)
    {
        std::stringstream errorString;

        m_eventBad++;

        errorString << "EBF_pktsCount returns no packets in event. eventbad: " << m_eventBad << ", count: " << m_eventCount;
        //m_log << MSG::WARNING << "eventbad " << m_eventBad << " count " << m_eventCount<< endreq;
        throw ObfException(errorString.str());

        //return filterStatus;
    }
    *********************************************************************/

    m_eventProcessed++;

    /* Start the event flow (do we need this?)*/
//    ctx.result.beg = TMR_GET ();

    // Keep track of the fate of processing
    unsigned int fate;

    // Loop over packets in our event
    while (EBF__pktsSize (pkts))
    {        
        EBF_edw  edw;
        EBF_ebw  ebw;
        EBF_pkt* pkt = EBF__pktsPkt (pkts);

        edw.ui = pkt->hdr.undef[7];
        ebw.ui = pkt->ebw.ui;

        // Another warm and fuzzy cross check (ie it can't happen in simulation)
        if (ebw.bf.proto != 1)
        {
            std::stringstream errorString;

            errorString << "Wrong Pkt Proto! ebw.bf.proto=" << ebw.bf.proto << " count " << m_eventCount;

            throw ObfException(errorString.str());
        }

        /* 
         | Must pull this information out of the packets before the 
         | calling the user else if might destroy it.
        */
        pkts = EBF__pktsNext (pkts);
            
        // Call the EDS handler which will call the filters in turn
        fate   = EDS_fwHandlerProcess (m_edsFw, edw.ui, pkt);
        
        // As fate will have it...
        if (fate & LCBV_PKT_FATE_M_NO_MORE) break;
        if (fate & LCBV_PKT_FATE_M_ABORT  ) break;
    }

    /* Flush the output  */
    EDS_fwHandlerFlush (m_edsFw, EDS_FW_MASK(0), 0);
    EDS_fwPostFlush (m_edsFw, EDS_FW_M_POST_0, 0xee);

    ////m_log << m_callBack->m_defaultStream.str() << endreq;

    /* Above will have caused the post event processing to occur */
    /* In particular, this will have set teh status bits in the  */
    /* TDS_variables so we can set status bits                   */
    //filterStatus = m_tdsPointers->m_filterStatus->get();
    filterStatus = fate;

    return filterStatus;
}

/* ---------------------------------------------------------------------- */
#if EDM_USE
/* ---------------------------------------------------------------------- *//*!

  \fn     static void setMessageLevels (MessageObjLevels lvl)
  \brief  Sets the diagnostic message level for each object

  \param  lvl  The message levels for each object.

                                                                          */
/* ---------------------------------------------------------------------- */
void ObfInterface::setMessageLevels (MessageObjLevels lvl)
{
  extern EDM_level              GFC_Filter_edm;
  extern EDM_level                 TFC_Acd_edm;
  extern EDM_level               TFC_Skirt_edm;
  extern EDM_level TFC_ProjectionTowerFind_edm;

  Filter_edm                  = lvl.filter;
  GFC_Filter_edm              = lvl.efc_filter;
  TFC_ProjectionTowerFind_edm = lvl.tfc_ptf;
  TFC_Acd_edm                 = lvl.tfc_acd;
  TFC_Skirt_edm               = lvl.tfc_skirt;

  return;
}

/* ---------------------------------------------------------------------- */
#endif

void ObfInterface::dumpCounters()
{
//    m_log << MSG::INFO << "ObfInterface::finalize: events total " << m_eventCount <<
//           "; processed " << m_eventCount << "; bad " << m_eventBad << endreq;
//    m_log << MSG::INFO;

    // Ok, now dump the JJ's statistics block
    ////myOutputFlush (m_callBack, -1);
    ////m_log << m_callBack->m_defaultStream.str() << endreq;

    // No EFC_deconstruct to call 
    free(m_edsFw);

    for(FilterMap::iterator filterIter = m_filterMap.begin(); filterIter != m_filterMap.end(); filterIter++)
    {
        EFC* efc = filterIter->second;
        free(efc);
    }

    // loop through the call back vector for End of Run processing
    OutputRtnVec& callBackVec = m_callBack->m_callBackVec;
    for(OutputRtnVec::iterator callBackIter = callBackVec.begin(); callBackIter != callBackVec.end(); callBackIter++)
    {
        (*callBackIter)->eorProcessing();
    }

    return;
}


void extractFilterInfo (EOVCallBackParams* callBack, EDS_fwIxb *ixb)
{
    // loop through the call back vector 
    OutputRtnVec& callBackVec = callBack->m_callBackVec;
    for(OutputRtnVec::iterator callBackIter = callBackVec.begin(); callBackIter != callBackVec.end(); callBackIter++)
    {
        try{
        (*callBackIter)->eoeProcessing(ixb);
        }
        catch(...)
        {
            int j = 0;
        }
    }
    return;
}


int dummyNotify (void *prm, unsigned int mode, unsigned int active, const unsigned char cfgs[32], void *rsdTbl)
//                 const EDS_rsdTbl     *rsdTbl)
{
   return 0;
}

/* --------------------------------------------------------------------- *//*!

  \fn unsigned int      LCBP_pktsRngFree   (LCBP                 lcb,
                                            EBF_pkt            *from,
                                            EBF_pkt              *to)
  \brief  Dummy implementation of the corresponds LCBD routine
  \return Status

  \param   lcb  The virtual LCB device handle
  \param  from  The address to start the free at.
  \param    to  The address of the last packet to free.
                                                                         */
/* --------------------------------------------------------------------- */
unsigned int dummyFreeRtn (void* prm, EBF_pkt *from, EBF_pkt *to)
{
  return 0;
}

/* --------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \fn     static int passThrough (void        *unused,
                                  unsigned int nbytes,
                                  EBF_pkt        *pkt,
                                  EBF_siv         siv,
                                  EDS_fwIxb       ixb)
  \brief  Pass through filter routine

  \param  unused  Context variable (unused)
  \param  nbytes  Number of bytes in the packet
  \param  pkt     The event packet
  \param  siv     The state information vector
  \param  ixb    The EDS framework information exchange block
                                                                          */
/* ---------------------------------------------------------------------- */
int passThrough (void        *unused,
                 unsigned int nbytes,
                 EBF_pkt        *pkt,
                 EBF_siv         siv,
                 EDS_fwIxb       ixb)
{
  return EDS_FW_FN_M_NO_MORE | EDS_FW_FN_M_POST_0;
}


/* ---------------------------------------------------------------------- *//*!

  \fn     static int streamFlush (Stream *stream, int reason)
  \brief  Dummy stream flushing routine

  \param  stream The stream context handle
  \param  reason The reason the stream is being flushed
                                                                          */
/* ---------------------------------------------------------------------- */
//#include "statistics/GFC_statsPrint.h"

int myOutputFlush (void* callBackPrm, int reason)
{
    if (callBackPrm && reason < 0)
    {
        /*
        EOVCallBackParams* eovParms = reinterpret_cast<EOVCallBackParams*>(callBackPrm);
        GFC_stats* stats = reinterpret_cast<GFC_stats*>(eovParms->m_statParms);

        // Need to make sure "printf" sends its output to the gaudi log file
        std::streambuf* save = std::cout.rdbuf(eovParms->m_defaultStream.rdbuf());
        std::cout << "Calling statsPrint \n" << std::endl;

        char buf[20000];
        buf[0] = 0;
        FILE myFile = *stdout;

        setvbuf(stdout, buf, _IOFBF, 20000);

        GFC_statsPrint (stats, GFC_STATUS_M_VETOES);

        *stdout = myFile;

        std::cout << buf << std::endl;
        std::string outBuf(buf);
        std::cout.flush();
        std::cout.rdbuf(save);
        //*/
    }

    return 0;
}

