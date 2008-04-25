/** @file HIPFilterLibsB1-0-8.h
*
* @class HIPFilterLibsB1-0-8
*
* @brief Class to allow modification of Heavy Ion Filter parameters via Job Options 
*
* @authors T. Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/HIPFilterLibsB1-0-8.h,v 1.1 2008/02/07 19:36:42 usher Exp $
*/

#ifndef __HIPFilterLibsB1_0_8_H
#define __HIPFilterLibsB1_0_8_H

#include "IFilterLibs.h"
#include "EFC_DB/EFC_DB_schema.h"

class HIPFilterLibsB1_0_8 : virtual public IFilterLibs
{
public:
    HIPFilterLibsB1_0_8();
    virtual ~HIPFilterLibsB1_0_8();

    // Flight Software Release associated with these libraries
    virtual const std::string FlightSoftwareRelease() const       {return m_release;}

    // Schema id associated to this filter
    virtual const unsigned short int FilterSchema() const         {return m_schemaId;}

    // Library containing the specific filter implamentation, name and path
    virtual const std::string FilterLibName() const               {return m_libName;}
    virtual const std::string FilterLibPath() const               {return m_libPath;}

    // Base path to the configuration libraries
    virtual const std::string ConfigBasePath() const              {return m_configBasePath;}

    // Master configuration file for this filter
    virtual const std::string MasterConfigName() const            {return m_masterName;}

    // Master configuration instance
    virtual const unsigned short int MasterConfigInstance() const {return m_masterInstance;}

    // Reference to map relating schema-instance info to config file info
    virtual const IdToFileMap& getIdToFileMap() const             {return m_idToFile;}

    // Given instance id typedef, return the string version of it
    virtual const std::string& getInstanceIdString(unsigned short int id) const;

    // Other way, given instance id string, return the typedef
    virtual const unsigned short int getInstanceId(const std::string& id) const;

    // Create a copy of the master configuration file that can be modified
    virtual void setMasterConfiguration(const EFC_DB_Schema* schema) {m_schema = *schema;}

    // Return a reference to the local copy (which can be modified)
    virtual EFC_DB_Schema& getMasterConfiguration()                  {return m_schema;}

private:
    std::string        m_release;
    std::string        m_libName;
    std::string        m_libPath;
    std::string        m_masterName;
    std::string        m_configBasePath;
    unsigned short int m_schemaId;
    unsigned short int m_masterInstance;
    IdToFileMap        m_idToFile;

    std::string                               m_dummy;
    std::map<unsigned short int, std::string> m_instanceIdToStringMap;
    std::map<std::string, unsigned short int> m_instanceStringToIdMap;

    EFC_DB_Schema      m_schema;
};

#endif // __ObfInterface_H
