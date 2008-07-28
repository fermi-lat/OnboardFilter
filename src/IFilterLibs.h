/** @file IFilterLibs.h
* @class IFilterLibs
*
* @brief Pure virtual interface class for Filter configuration library information
*
* @authors T. Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/IFilterLibs.h,v 1.1 2008/04/25 23:21:52 usher Exp $
*/

#ifndef __IFilterLibs_H
#define __IFilterLibs_H

#include <string>
#include <vector>
#include <map>

typedef std::pair<unsigned short int, unsigned short int> SchemaPair;
typedef std::map<SchemaPair, std::string>                 IdToFileMap;

#ifndef EFC_DB_Schema
typedef struct _EFC_DB_Schema EFC_DB_Schema;
#endif

class IFilterLibs
{
public:
    virtual ~IFilterLibs() {}

    // Flight Software Release associated with these libraries
    virtual const std::string FlightSoftwareRelease() const = 0;

    // Schema id associated to this filter
    virtual const unsigned short int FilterSchema() const = 0;

    // Library containing the specific filter implamentation, name and path
    // Note that the path to the filter library is different from the path 
    // to the filter configurations
    virtual const std::string FilterLibName() const = 0;
    virtual const std::string FilterLibPath() const = 0;

    // Base path to the configuration libraries
    virtual const std::string ConfigBasePath() const = 0;

    // Master configuration file for this filter
    virtual const std::string MasterConfigName() const = 0;

    // Master configuration instance
    virtual const unsigned short int MasterConfigInstance() const = 0;

    // Reference to map relating schema-instance info to config file info
    virtual const IdToFileMap& getIdToFileMap() const = 0;

    // Given instance id typedef, return the string version of it
    virtual const std::string& getInstanceIdString(unsigned short int id) const = 0;

    // Other way, given instance id string, return the typedef
    virtual const unsigned short int getInstanceId(const std::string& id) const = 0;

    // Return string description of a given bit in status word
    virtual const std::string& getStatWordDesc(int idx) const = 0;

    // Create a copy of the master configuration file that can be modified
    virtual void setMasterConfiguration(const EFC_DB_Schema* schema) = 0;

    // Return a reference to the local copy (which can be modified)
    virtual EFC_DB_Schema& getMasterConfiguration() = 0;
};

#endif // __IFilterCfgPrms_H
