/** @file ObfInterface.h

* @class ObfInterface
*
* @brief A class to interface Gaudi level OnboardFilter to the actual fsw version
*
* last modified 12/04/2006
*
* @authors T. Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/ObfInterface.h,v 1.8 2008/04/09 20:40:21 usher Exp $
*/

#ifndef __ObfInterface_H
#define __ObfInterface_H

#include <string>
#include <map>
#include <vector>
#include <exception>

// Forward declarations
typedef struct _EDS_fw        EDS_fw;
typedef struct _EFC           EFC;
typedef struct _EFC_DB_Schema EFC_DB_Schema;

namespace EbfWriterTds {
    class Ebf;
}

class EOVCallBackParams;
class IFilterTool;
class IFilterLibs;

// The class to interface to FSW version of Onboard Filter
class ObfInterface
{
public:
    // Something to return a message in an exception thrown from this class
    class ObfException : public std::exception 
    {
    public:
        ObfException(std::string error) : m_what(error) {}
       ~ObfException() throw() {}
        virtual const char *what() const throw() {return m_what.c_str();}
        std::string m_what;
    };

    // Retrieve an instance of this class
    static ObfInterface* instance();

    ///@name access methods
    /// Set up a filter specified by its name
    int  setupFilter(const EFC_DB_Schema* schema,
                     unsigned short int   configIndex);

    /// Set up the specific passthrough filter
    bool setupPassThrough(void* prm);

    /// Set a call back routine for end of event output processing
    void setEovOutputCallBack(IFilterTool* outRtn);

    /// This will cause the filters to execute upon the given event
    /// Results will appear in the provided TDS output objects
    unsigned int filterEvent(EbfWriterTds::Ebf* ebfData);

    /// Return a pointer to a given filter's parameter block of the requested type
    /// (must be typed by the user)
    void* getFilterPrm(unsigned short filterSchemaId, int type);

    ///@name other methods
    /// Load shareable libraries
    bool loadLibrary(std::string libraryName, std::string libraryPath = "", int verbosity = 0);

    ///@name other methods
    /// Load shareable libraries
    const EFC_DB_Schema& loadFilterLibs(IFilterLibs* filterLibs, int verbosity = 0);
    
    // Output status of counters
    void dumpCounters();

private:

    // Private functions
    // constructor
    ObfInterface();

    // destructor
    virtual ~ObfInterface();

    // Pointer to me
    static ObfInterface* m_instance;

    // Verbosity for output
    int                  m_verbosity;

    // Keep track of "priority" for each filter
    int                  m_priority;

    // pointers to FSW structures
    EDS_fw              *m_edsFw;

    // Keep track of initialized filters
    typedef std::map<unsigned short int, EFC*> FilterMap;
    FilterMap            m_filterMap;

    // Map file name to EH_id enum value
    typedef std::map<unsigned short int, unsigned int> SchemaToEnumMap;
    SchemaToEnumMap      m_schemaToEnum;

    // Map to control the list of filter output routines
    EOVCallBackParams*   m_callBack;

    // Counters, run type data, etc.
    int                  m_eventCount;
    int                  m_eventProcessed;
    int                  m_eventBad;

    int                  m_levels;
};

#endif // __ObfInterface_H
