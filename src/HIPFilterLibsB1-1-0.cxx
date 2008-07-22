#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "HIPFilterLibsB1-1-0.h"

#include "XFC_DB/HFC_DB_schema.h"
#include "XFC_DB/HIP_DB_instance.h"

HIPFilterLibsB1_1_0::HIPFilterLibsB1_1_0()
{
    // Release for this setup 
    m_release        = "B1-1-0";

    // The Filter library name
    m_libName        = "hfc";

    // Path to this library
    m_libPath        = "$(OBFXFCBINDIR)/hfc";

    // Path to the master configuration file
    m_configBasePath = "$(OBFXFC_DBBINDIR)";

    // Name of the master configuration file
    m_masterName     = "hip_master";

    // Schema id for this filter
    m_schemaId       = HIP_DB_SCHEMA;

    // Master configuration instance id
    m_masterInstance = HIP_DB_INSTANCE_K_MASTER;

    // Schema id to file names
    // This maps the configuration id to a corresponding file name for the library to load
    // If the configurations change then this needs to be updated. 
    m_idToFile.clear();

    m_idToFile[SchemaPair(HIP_DB_SCHEMA,HIP_DB_INSTANCE_K_MASTER)]              = "hip_master";
    m_idToFile[SchemaPair(HIP_DB_SCHEMA,HIP_DB_INSTANCE_K_EFF)]                 = "hip_eff";
    m_idToFile[SchemaPair(HIP_DB_SCHEMA,HIP_DB_INSTANCE_K_EFF_TAPER)]           = "hip_eff_taper";
    m_idToFile[SchemaPair(HIP_DB_SCHEMA,HIP_DB_INSTANCE_K_LEO)]                 = "hip_leo";
    m_idToFile[SchemaPair(HIP_DB_SCHEMA,HIP_DB_INSTANCE_K_NORMAL)]              = "hip_normal";

    m_dummy = "No valid id found";

    // Provide a mechanism to convert an instance typedef to a string (for convenience)
    m_instanceIdToStringMap.clear();

    m_instanceIdToStringMap[HIP_DB_INSTANCE_K_MASTER]    = "HIP_DB_INSTANCE_K_MASTER";
    m_instanceIdToStringMap[HIP_DB_INSTANCE_K_EFF]       = "HIP_DB_INSTANCE_K_EFF";
    m_instanceIdToStringMap[HIP_DB_INSTANCE_K_EFF_TAPER] = "HIP_DB_INSTANCE_K_EFF_TAPER";
    m_instanceIdToStringMap[HIP_DB_INSTANCE_K_LEO]       = "HIP_DB_INSTANCE_K_LEO";
    m_instanceIdToStringMap[HIP_DB_INSTANCE_K_NORMAL]    = "HIP_DB_INSTANCE_K_NORMAL";

    // go the other way
    m_instanceStringToIdMap.clear();

    m_instanceStringToIdMap["HIP_DB_INSTANCE_K_MASTER"]    = HIP_DB_INSTANCE_K_MASTER;
    m_instanceStringToIdMap["HIP_DB_INSTANCE_K_EFF"]       = HIP_DB_INSTANCE_K_EFF;
    m_instanceStringToIdMap["HIP_DB_INSTANCE_K_EFF_TAPER"] = HIP_DB_INSTANCE_K_EFF_TAPER;
    m_instanceStringToIdMap["HIP_DB_INSTANCE_K_LEO"]       = HIP_DB_INSTANCE_K_LEO;
    m_instanceStringToIdMap["HIP_DB_INSTANCE_K_NORMAL"]    = HIP_DB_INSTANCE_K_NORMAL;

    // Set EFC_DB_Schema filter id and cnt to "bad" values
    m_schema.filter.id  = 0xFFFF;
    m_schema.filter.cnt = 0;
}

HIPFilterLibsB1_1_0::~HIPFilterLibsB1_1_0()
{
}

// Given instance id typedef, return the string version of it
const std::string& HIPFilterLibsB1_1_0::getInstanceIdString(unsigned short int id) const
{
    std::map<unsigned short int, std::string>::const_iterator idIter = m_instanceIdToStringMap.find(id);

    if (idIter != m_instanceIdToStringMap.end()) return idIter->second;

    return m_dummy;
}

// Other way, given instance id string, return the typedef
const unsigned short int HIPFilterLibsB1_1_0::getInstanceId(const std::string& id) const
{
    unsigned short int typeId = -1;

    std::map<std::string,unsigned short int>::const_iterator idIter = m_instanceStringToIdMap.find(id);

    if (idIter != m_instanceStringToIdMap.end()) typeId = idIter->second;

    return typeId;
}
