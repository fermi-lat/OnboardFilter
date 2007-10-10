
#include "ObfInterface.h" 
#include "OutputRtn.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "EbfWriter/Ebf.h"
#include "OnboardFilterTds/FilterStatus.h"

#include "EFC_DB/EFC_DB_schema.h"
#include "GFC_DB/GFC_DB_schema.h"
#include "GFC_DB/GFC_DB_instance.h"
#include "XFC_DB/MFC_DB_schema.h"
#include "XFC_DB/MFC_DB_instance.h"
#include "XFC_DB/HFC_DB_schema.h"
#include "XFC_DB/HFC_DB_instance.h"
#include "XFC_DB/DFC_DB_schema.h"
#include "XFC_DB/DFC_DB_instance.h"

#include "EFC/EFC.h"
#include "EFC/EFC_edsFw.h"

#include "CDM/CDM_pubdefs.h"
#include "CMX/CMX_lookupPub.h"

#include "EDS/io/EBF_evts.h"
#include "EDS/EBF_edw.h"
#include "EDS/LCBV.h"

#include "GaudiKernel/MsgStream.h"

/* ---------------------------------------------------------------------- */

// Forward declarations
// This is the callback for extracting the results of running filters
static void         extractFilterInfo (EOVCallBackParams* callBackParams, EDS_fwIxb *ixb);
// This provides an event "free" routine to EDS_fw 
static unsigned int dummyFreeRtn (void* prm, EBF_pkt *from, EBF_pkt *to);
// This provides an event flushing routine
static int          myOutputFlush (void *unused, int reason);
// Enables us to run the filters in pass-through mode
static int          passThrough (void *unused, unsigned int nbytes, EBF_pkt *pkt, EBF_siv siv, EDS_fwIxb ixb);

/* ---------------------------------------------------------------------- */
// Local struct definition for veto mask (must be better way...)
typedef struct _EFC_samplerPrescaleCounter
{
  unsigned int countdown;   /*!< Current countdown value                  */
  unsigned int   refresh;   /*!< Refresh value                            */
} EFC_samplerPrescaleCounter;

typedef struct _EFC_samplerPrescaleCtx
{
    EFC_samplerPrescaleCounter
                         in;    /*!< Input prescaler                      */
    EFC_samplerPrescaleCounter
                        out;    /*!< Output prescaler                     */
    EFC_classes     enabled;    /*!< The classes of enabled prescalers    */
    EFC_samplerPrescaleCounter
              prescalers[32];   /*!< The prescales for each status bit
                                     an additional one for the output 
                                     prescaling                           */
} EFC_samplerPrescaleCtx;

typedef struct _EFC_samplerClasses
{
    EFC_classes   defined; /*!< The set of defined class of bits          */
    EFC_classes   enabled; /*!< The set so enabled class of bits          */
} EFC_samplerClasses;

typedef struct _EFC_sampler
{
    EFC_samplerPrescaleCtx  prescale; /*!< The prescale context           */
    EFC_samplerClasses       classes; /*!< The status word classes        */
} EFC_sampler;

/* ---------------------------------------------------------------------- */
// Local utility class for handling output call back routines at end of event
class EOVCallBackParams
{
public:
    EOVCallBackParams() : m_callBackParm(0) {m_callBackVec.clear();}
    ~EOVCallBackParams() {}

    void*              m_callBackParm;
//    FilterTdsPointers* m_tdsPointers;
    OutputRtnVec       m_callBackVec;
};


ObfInterface::ObfInterface(MsgStream& log, const std::string& filePath, void* callBackParm, int verbosity) : 
              m_log(log), m_libraryPath(filePath), m_eventCount(0), 
              m_eventProcessed(0), m_eventBad(0), m_levels(0), m_verbosity(verbosity)
{
    // Call back routine control
    m_callBack = new EOVCallBackParams();
    //m_callBack->m_tdsPointers = m_tdsPointers;
    m_callBack->m_callBackParm = callBackParm;

    // Set up the map of allowed schema's 
    m_schemaMap.clear();

    m_schemaMap["GammaFilter"] = SchemaPair(GFC_DB_SCHEMA, GFC_DB_INSTANCE_K_MASTER);
    m_schemaMap["MipFilter"]   = SchemaPair(MFC_DB_SCHEMA, MFC_DB_INSTANCE_K_MASTER);
    m_schemaMap["HFCFilter"]   = SchemaPair(HFC_DB_SCHEMA, HFC_DB_INSTANCE_K_MASTER);
    m_schemaMap["DFCFilter"]   = SchemaPair(DFC_DB_SCHEMA, DFC_DB_INSTANCE_K_MASTER);

    // Schema id to file names
    m_idToFile.clear();

    m_idToFile[SchemaPair(GFC_DB_SCHEMA,GFC_DB_INSTANCE_K_MASTER)]    = "gfc_db_master";
    m_idToFile[SchemaPair(GFC_DB_SCHEMA,GFC_DB_INSTANCE_K_NORMAL)]    = "gfc_db_normal";
    m_idToFile[SchemaPair(GFC_DB_SCHEMA,GFC_DB_INSTANCE_K_DEFAULT)]   = "gfc_db_default";

    m_idToFile[SchemaPair(MFC_DB_SCHEMA,MFC_DB_INSTANCE_K_MASTER)]    = "mfc_db_master";
    m_idToFile[SchemaPair(MFC_DB_SCHEMA,MFC_DB_INSTANCE_K_NORMAL)]    = "mfc_db_normal";
//    m_idToFile[SchemaPair(MFC_DB_SCHEMA,MFC_DB_INSTANCE_K_DEFAULT)]   = "mfc_db_default";

    m_idToFile[SchemaPair(HFC_DB_SCHEMA,HFC_DB_INSTANCE_K_MASTER)]    = "hfc_db_master";
    m_idToFile[SchemaPair(HFC_DB_SCHEMA,HFC_DB_INSTANCE_K_NORMAL)]    = "hfc_db_normal";
    m_idToFile[SchemaPair(HFC_DB_SCHEMA,HFC_DB_INSTANCE_K_DEFAULT)]   = "hfc_db_default";
  
    m_idToFile[SchemaPair(DFC_DB_SCHEMA,DFC_DB_INSTANCE_K_MASTER)]    = "dfc_db_master";
    m_idToFile[SchemaPair(DFC_DB_SCHEMA,DFC_DB_INSTANCE_K_GEM)]       = "dfc_db_gem";
    m_idToFile[SchemaPair(DFC_DB_SCHEMA,DFC_DB_INSTANCE_K_TKR)]       = "dfc_db_tkr";
    m_idToFile[SchemaPair(DFC_DB_SCHEMA,DFC_DB_INSTANCE_K_GROUND_HI)] = "dfc_db_ground_hi";
    m_idToFile[SchemaPair(DFC_DB_SCHEMA,DFC_DB_INSTANCE_K_PRIMITIVE)] = "dfc_db_primitive";

    // EFC library already loaded (we link to it)
    // Load the other filter libraries
    loadLibrary ("hfc", m_verbosity);
    loadLibrary ("mfc", m_verbosity);
    loadLibrary ("dfc", m_verbosity);
/*
    // do we need this?
    std::string eds_db_File = m_FileNamePath + delim + "eds_db" + fType;
    loadLib (eds_db_File.c_str(),verbose,0);
*/
    // Load the Gleam geometry for fsw
    loadLibrary ("ggf_db", m_verbosity);

    /* Allocate and initialize the Event Data Services framework */
    /* - NOTE: these are C structures not C++ classes */
    m_edsFw = (EDS_fw*) malloc(EDS_fwSizeof ());

    EDS_fwConstruct (m_edsFw, NULL, NULL, NULL);
    EDS_fwFreeSet   (m_edsFw, (LCBV_pktsRngFreeCb)dummyFreeRtn, NULL);  

    // Set up the output handler here (actual call back routines will be set up)

    // Register the "post" routine to handle end of event processing
    EDS_fwPostRegister   (m_edsFw,
                          EDS_FW_K_POST_0,
                         (EDS_fwPostStartRtn)NULL,
                         (EDS_fwPostWriteRtn)extractFilterInfo,
                         (EDS_fwPostNotifyRtn)NULL,
                         (EDS_fwPostFlushRtn)myOutputFlush,
                          m_callBack);

    EDS_fwPostChange   (m_edsFw, EDS_FW_M_POST_0,  EDS_FW_M_POST_0  );

    // Clear vector of filter pointers
    m_filterVec.clear();

    return;
}

ObfInterface::~ObfInterface()
{
    free(m_edsFw);

    return;
}

int ObfInterface::setupFilter(const std::string& filterName, int priority, unsigned vetoMask, bool modifyVetoMask)
{
    int filterId = -100;

    // Associate the filter name to a schema, if none then exit
    SchemaMap::iterator schemaIter = m_schemaMap.find(filterName);

    if (schemaIter != m_schemaMap.end())
    {
        SchemaPair schemaPair = schemaIter->second;

        //Look up the file name associated with this schema
        std::string fileName = m_idToFile[schemaPair];

        // This is the master file name, load this library
        loadLibrary(fileName, m_verbosity);

        /* Find the Gamma filter and get it ready for action */
        const EFC_DB_Schema* schema = EFC_lookup (schemaPair.first, schemaPair.second);

        // Load any dependent libraries called for by the master configuration
        for (int idx = 0; idx < schema->cnt; idx++)
        {
            SchemaPair pair(schemaPair.first, schema->instances[idx]);
            
            fileName = m_idToFile[pair];

            if (fileName == "")
            {
                m_log << MSG::ERROR << "Unrecognized Schema for dependent library: " << fileName << endreq;
                return filterId;
            }

            loadLibrary(fileName, m_verbosity);
        }

        // Get a pointer to the "services" structure 
        const char*                            fnd      = schema->eds.get;
        EDS_DB_handlerServicesGet              get      = (const EDS_DB_HandlerConstructServices*(*)(void*)) CMX_lookupSymbol(fnd);
        const EDS_DB_HandlerConstructServices *services = (EDS_DB_HandlerConstructServices *)get (0);

        // Allocate memory for the Filter and then initialize it
        EFC* filter = (EFC *) malloc(services->sizeOf (schema, NULL));
        const EDS_fwHandlerServices* gfcService = services->construct (filter, schema, NULL, m_edsFw);
        EFC_report (filter, -1);

        /* Register and enable the filter */
        filterId = EDS_fwHandlerServicesRegister (m_edsFw,  priority, gfcService, filter);

        // Modify the veto mask is requested (this means we are running "pass through" mode)
        if (modifyVetoMask)
        {
            EFC_sampler* sampler = (EFC_sampler*)EFC_get(filter, EFC_OBJECT_K_SAMPLER);

            sampler->classes.enabled.all = vetoMask;
        }

        // Keep track of the pointer for deletion at end of processing
        m_filterVec.push_back(filter);

        // enable the filter
        EDS_fwHandlerChange(m_edsFw, EDS_FW_MASK(filterId), EDS_FW_MASK(filterId) );
    }
    else
    {
        m_log << MSG::ERROR << "Unrecognized Filter Name: " << filterName << endreq;
    }

    return filterId;
}

void ObfInterface::setEovOutputCallBack(OutputRtn* outRtn)
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
             -1,
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
bool ObfInterface::loadLibrary (const std::string& libraryName, int verbosity)
{
    if (verbosity > 0) printf (" Loading: %s", libraryName.c_str());

    // Ugly but effective...
    #ifdef WIN32
        std::string fType = ".dll";
        std::string delim = "/";
    #else
        std::string fType = ".so";
        std::string delim = "/lib";
    #endif

    // Full file name with path
    std::string fullFileName = m_libraryPath + delim + libraryName + fType;

    // call CDM to load the library
    // Note that currently (12/4/06) cal_db will be zero even when library loads
    CDM_Database* cal_db = CDM_loadDatabase (fullFileName.c_str(), 0);

    if (verbosity > 0) printf (cal_db == NULL ? " (FAILED)\n\n" : " (succeeded)\n\n");

    return cal_db != NULL;
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
    if(length==0)
    {
        m_log << MSG::WARNING << "Event has no EBF data. Ignoring" << endreq;

        return filterStatus;
    }

    // The data variable points to the head of our EBF_ptks 
    // What follows here will create the EBF_pkts object in C++ (as opposed to C)
    unsigned int* dataPtr = (unsigned int*)data;

    // The union of the long long and ps
    EBF_pkts_u    pktsu;

    // Size comes from the length returned above
    pktsu.ps.size = length;
    pktsu.ps.pkt  = (EBF_pkt*)(dataPtr);

    // Now convert to a EBF_pkts struct
    EBF_pkts      pkts = pktsu.ll;

    // Get a warm and fuzzy feeling by doing a simple check...
    int numPkts = EBF_pktsCount (pkts);
    if (numPkts < 1)
    {
        m_eventBad++;

        m_log << MSG::WARNING << "eventbad " << m_eventBad << " count " << m_eventCount<< endreq;

        return filterStatus;
    }

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
            m_log << MSG::WARNING << "Wrong Pkt Protot! ebw.bf.proto=" << ebw.bf.proto << " count " << m_eventCount<< endreq;
            break;
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
    EDS_fwPostFlush (m_edsFw, EDS_FW_M_POST_1, 0xee);

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
//    using namespace std;
   
//    printf("ObfInterface::finalize: events total %d; processed %d; bad %d\n",
//         eventCount,eventProcessed,eventBad);
    m_log << MSG::INFO << "ObfInterface::finalize: events total " << m_eventCount <<
           "; processed " << m_eventCount << "; bad " << m_eventBad << endreq;
//    m_log << MSG::INFO << "Rejected " << m_rejected << " triggers using mask " << m_mask  << endreq;
    m_log << MSG::INFO;

    // No EFC_deconstruct to call 
    free(m_edsFw);

    for(std::vector<EFC*>::iterator filterIter = m_filterVec.begin(); filterIter != m_filterVec.end(); filterIter++)
    {
        EFC* efc = *filterIter;
        free(efc);
    }

    // loop through the call back vector for End of Run processing
    OutputRtnVec& callBackVec = m_callBack->m_callBackVec;
    for(OutputRtnVec::iterator callBackIter = callBackVec.begin(); callBackIter != callBackVec.end(); callBackIter++)
    {
        (*callBackIter)->eorProcessing(m_log);
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
        (*callBackIter)->eovProcessing(callBack->m_callBackParm, ixb);
        }
        catch(...)
        {
            int j = 0;
        }
    }
    return;
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
int myOutputFlush (void *unused, int reason)
{
//  printf ("flush Stream Id: %8u   Cnt: %8u  Reason: %d\n",
//          stream->id, stream->cnt++, reason);
  return 0;
}
