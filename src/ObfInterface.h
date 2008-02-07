/** @file ObfInterface.h

* @class ObfInterface
*
* @brief A class to interface Gaudi level OnboardFilter to the actual fsw version
*
* last modified 12/04/2006
*
* @authors T. Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/ObfInterface.h,v 1.5 2007/12/28 18:39:47 usher Exp $
*/

#ifndef __ObfInterface_H
#define __ObfInterface_H

#include <string>
#include <map>
#include <vector>

// Forward declarations
typedef struct _EBF_pkt   EBF_pkt;
typedef signed int        EBF_siv;
typedef struct _EDS_fw    EDS_fw;
typedef struct _EDS_fwIxb EDS_fwIxb;
typedef struct _EFC       EFC;

namespace EbfWriterTds {
    class Ebf;
}
namespace OnboardFilterTds {
    class TowerHits;
    class FilterStatus;
}

class MsgStream;
class ObfOutputCallBackParm;
class EOVCallBackParams;
class OutputRtn;
class IFilterCfgPrms;

// The class to interface to FSW version of Onboard Filter
class ObfInterface
{
public:
    // constructors
    ObfInterface(MsgStream& log, const std::string& filePath, ObfOutputCallBackParm* callBackParm, int verbosity = 0);

    // destructor
    virtual ~ObfInterface();

    ///@name access methods
    /// Set up a filter specified by its name
    int  setupFilter(const std::string& filterName, 
                     const std::string& configuration, 
                     IFilterCfgPrms*    filterPrms,
                     int                priority, 
                     unsigned           vetoMask, 
                     bool               modifyVetoMask);

    /// Set up the specific passthrough filter
    bool setupPassThrough(void* prm);

    /// Set a call back routine for end of event output processing
    void setEovOutputCallBack(OutputRtn* outRtn);

    /// This will cause the filters to execute upon the given event
    /// Results will appear in the provided TDS output objects
    unsigned int  filterEvent(EbfWriterTds::Ebf* ebfData);

    /// Return a pointer to a given filter's configuration block
    /// (must be typed by the user)
    void* getFilterCfgPrm(int filterId);

    ///@name other methods
    /// Load shareable libraries
    bool loadLibrary (const std::string& libraryName, int verbosity = 0);
    
    // Output status of counters
    void dumpCounters();

private:

    // Private functions
    // The path where we can find the fsw libraries
    std::string m_libraryPath;

    // Verbosity for output
    int                m_verbosity;

    // pointers to FSW structures
    EDS_fw            *m_edsFw;
    std::vector<EFC*>  m_filterVec;

    // Map input name to a schema pair for the master configuration
    typedef std::pair<unsigned short int, unsigned short int> SchemaPair;
    typedef std::map<std::string, SchemaPair>                 SchemaMap;
    SchemaMap          m_schemaMap;

    // Map master schema to a file name
    typedef std::map<SchemaPair, std::string> IdToFileMap;
    IdToFileMap        m_idToFile;

    // Map file name to EH_id enum value
    typedef std::map<std::string, unsigned int> FileToEnumMap;
    FileToEnumMap      m_fileToEnum;

    // Map between filter and its configuration
    typedef std::map<unsigned short int, void*> IdToCfgMap;

    IdToCfgMap         m_idToCfgMap;

    // Enable output to the "standard" Gaudi log stream
    MsgStream&         m_log;

    // Pointers to the output TDS classes - refreshed per event
    //FilterTdsPointers* m_tdsPointers;

    // Map to control the list of filter output routines
    EOVCallBackParams* m_callBack;

    // Counters, run type data, etc.
    int                m_eventCount;
    int                m_eventProcessed;
    int                m_eventBad;

    int                m_levels;
};

#endif // __ObfInterface_H
