#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "MIPFilterLibsB1-0-8.h"

#include "XFC_DB/MFC_DB_schema.h"
#include "XFC_DB/MIP_DB_instance.h"

MIPFilterLibsB1_0_8::MIPFilterLibsB1_0_8()
{
    // Release for this setup 
    m_release        = "B1-0-8";

    // The Filter library name
    m_libName        = "mfc";

    // Path to this library
    m_libPath        = "$(OBFXFCBINDIR)/mfc";

    // Path to the master configuration file
    m_configBasePath = "$(OBFXFC_DBBINDIR)";

    // Name of the master configuration file
    m_masterName     = "mip_master";

    // Schema id for this filter
    m_schemaId       = MIP_DB_SCHEMA;

    // Master configuration instance id
    m_masterInstance = MIP_DB_INSTANCE_K_MASTER;

    // Schema id to file names
    // This maps the configuration id to a corresponding file name for the library to load
    // If the configurations change then this needs to be updated. 
    m_idToFile.clear();

    m_idToFile[SchemaPair(MIP_DB_SCHEMA,MIP_DB_INSTANCE_K_MASTER)]             = "mip_master";
    m_idToFile[SchemaPair(MIP_DB_SCHEMA,MIP_DB_INSTANCE_K_ALL_AXIS)]           = "mip_all_axis";
    m_idToFile[SchemaPair(MIP_DB_SCHEMA,MIP_DB_INSTANCE_K_EFF_OFF_AXIS)]       = "mip_eff_off_axis";
    m_idToFile[SchemaPair(MIP_DB_SCHEMA,MIP_DB_INSTANCE_K_EFF_TAPER_OFF_AXIS)] = "mip_eff_taper_off_axis";
    m_idToFile[SchemaPair(MIP_DB_SCHEMA,MIP_DB_INSTANCE_K_OFF_AXIS)]           = "mip_off_axis";

    m_dummy = "No valid id found";

    // Provide a mechanism to convert an instance typedef to a string (for convenience)
    m_instanceIdToStringMap.clear();

    m_instanceIdToStringMap[MIP_DB_INSTANCE_K_MASTER]             = "MIP_DB_INSTANCE_K_MASTER";
    m_instanceIdToStringMap[MIP_DB_INSTANCE_K_ALL_AXIS]           = "MIP_DB_INSTANCE_K_ALL_AXIS";
    m_instanceIdToStringMap[MIP_DB_INSTANCE_K_EFF_OFF_AXIS]       = "MIP_DB_INSTANCE_K_EFF_OFF_AXIS";
    m_instanceIdToStringMap[MIP_DB_INSTANCE_K_EFF_TAPER_OFF_AXIS] = "MIP_DB_INSTANCE_K_EFF_TAPER_OFF_AXIS";
    m_instanceIdToStringMap[MIP_DB_INSTANCE_K_OFF_AXIS]           = "MIP_DB_INSTANCE_K_OFF_AXIS";

    // go the other way
    m_instanceStringToIdMap.clear();

    m_instanceStringToIdMap["MIP_DB_INSTANCE_K_MASTER"]             = MIP_DB_INSTANCE_K_MASTER;
    m_instanceStringToIdMap["MIP_DB_INSTANCE_K_ALL_AXIS"]           = MIP_DB_INSTANCE_K_ALL_AXIS;
    m_instanceStringToIdMap["MIP_DB_INSTANCE_K_EFF_OFF_AXIS"]       = MIP_DB_INSTANCE_K_EFF_OFF_AXIS;
    m_instanceStringToIdMap["MIP_DB_INSTANCE_K_EFF_TAPER_OFF_AXIS"] = MIP_DB_INSTANCE_K_EFF_TAPER_OFF_AXIS;
    m_instanceStringToIdMap["MIP_DB_INSTANCE_K_OFF_AXIS"]           = MIP_DB_INSTANCE_K_OFF_AXIS;

    // Set EFC_DB_Schema filter id and cnt to "bad" values
    m_schema.filter.id  = 0xFFFF;
    m_schema.filter.cnt = 0;
}

MIPFilterLibsB1_0_8::~MIPFilterLibsB1_0_8()
{
}

// Given instance id typedef, return the string version of it
const std::string& MIPFilterLibsB1_0_8::getInstanceIdString(unsigned short int id) const
{
    std::map<unsigned short int, std::string>::const_iterator idIter = m_instanceIdToStringMap.find(id);

    if (idIter != m_instanceIdToStringMap.end()) return idIter->second;

    return m_dummy;
}

// Other way, given instance id string, return the typedef
const unsigned short int MIPFilterLibsB1_0_8::getInstanceId(const std::string& id) const
{
    unsigned short int typeId = -1;

    std::map<std::string,unsigned short int>::const_iterator idIter = m_instanceStringToIdMap.find(id);

    if (idIter != m_instanceStringToIdMap.end()) typeId = idIter->second;

    return typeId;
}
