#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "DGNFilterLibsB1-0-8.h"

#include "XFC_DB/DFC_DB_schema.h"
#include "XFC_DB/DGN_DB_instance.h"

DGNFilterLibsB1_0_8::DGNFilterLibsB1_0_8()
{
    // Release for this setup 
    m_release        = "B1-0-8";

    // The Filter library name
    m_libName        = "dfc";

    // Path to this library
    m_libPath        = "$(OBFXFCBINDIR)/dfc";

    // Path to the master configuration file
    m_configBasePath = "$(OBFXFC_DBBINDIR)";

    // Name of the master configuration file
    m_masterName     = "dgn_master";

    // Schema id for this filter
    m_schemaId       = DGN_DB_SCHEMA;

    // Master configuration instance id
    m_masterInstance = DGN_DB_INSTANCE_K_MASTER;

    // Schema id to file names
    // This maps the configuration id to a corresponding file name for the library to load
    // If the configurations change then this needs to be updated. 
    m_idToFile.clear();
  
    m_idToFile[SchemaPair(DGN_DB_SCHEMA,DGN_DB_INSTANCE_K_MASTER)]    = "dgn_master";
    m_idToFile[SchemaPair(DGN_DB_SCHEMA,DGN_DB_INSTANCE_K_GEM)]       = "dgn_gem";
    m_idToFile[SchemaPair(DGN_DB_SCHEMA,DGN_DB_INSTANCE_K_GEM_500)]   = "dgn_gem_500";
    m_idToFile[SchemaPair(DGN_DB_SCHEMA,DGN_DB_INSTANCE_K_GEM_1000)]  = "dgn_gem_1000";
    m_idToFile[SchemaPair(DGN_DB_SCHEMA,DGN_DB_INSTANCE_K_TKR)]       = "dgn_tkr";
    m_idToFile[SchemaPair(DGN_DB_SCHEMA,DGN_DB_INSTANCE_K_GROUND_HI)] = "dgn_ground_hi";
    m_idToFile[SchemaPair(DGN_DB_SCHEMA,DGN_DB_INSTANCE_K_PRIMITIVE)] = "dgn_primitive";

    m_dummy = "No valid id found";

    // Provide a mechanism to convert an instance typedef to a string (for convenience)
    m_instanceIdToStringMap.clear();

    m_instanceIdToStringMap[DGN_DB_INSTANCE_K_MASTER]    = "DGN_DB_INSTANCE_K_MASTER";
    m_instanceIdToStringMap[DGN_DB_INSTANCE_K_GEM]       = "DGN_DB_INSTANCE_K_GEM";
    m_instanceIdToStringMap[DGN_DB_INSTANCE_K_GEM_500]   = "DGN_DB_INSTANCE_K_GEM_500";
    m_instanceIdToStringMap[DGN_DB_INSTANCE_K_GEM_1000]  = "DGN_DB_INSTANCE_K_GEM_1000";
    m_instanceIdToStringMap[DGN_DB_INSTANCE_K_TKR]       = "DGN_DB_INSTANCE_K_TKR";
    m_instanceIdToStringMap[DGN_DB_INSTANCE_K_GROUND_HI] = "DGN_DB_INSTANCE_K_GROUND_HI";
    m_instanceIdToStringMap[DGN_DB_INSTANCE_K_PRIMITIVE] = "DGN_DB_INSTANCE_K_PRIMITIVE";

    // go the other way
    m_instanceStringToIdMap.clear();

    m_instanceStringToIdMap["DGN_DB_INSTANCE_K_MASTER"]    = DGN_DB_INSTANCE_K_MASTER;
    m_instanceStringToIdMap["DGN_DB_INSTANCE_K_GEM"]       = DGN_DB_INSTANCE_K_GEM;
    m_instanceStringToIdMap["DGN_DB_INSTANCE_K_GEM_500"]   = DGN_DB_INSTANCE_K_GEM_500;
    m_instanceStringToIdMap["DGN_DB_INSTANCE_K_GEM_1000"]  = DGN_DB_INSTANCE_K_GEM_1000;
    m_instanceStringToIdMap["DGN_DB_INSTANCE_K_TKR"]       = DGN_DB_INSTANCE_K_TKR;
    m_instanceStringToIdMap["DGN_DB_INSTANCE_K_GROUND_HI"] = DGN_DB_INSTANCE_K_GROUND_HI;
    m_instanceStringToIdMap["DGN_DB_INSTANCE_K_PRIMITIVE"] = DGN_DB_INSTANCE_K_PRIMITIVE;

    // Set EFC_DB_Schema filter id and cnt to "bad" values
    m_schema.filter.id  = 0xFFFF;
    m_schema.filter.cnt = 0;
}

DGNFilterLibsB1_0_8::~DGNFilterLibsB1_0_8()
{
}

// Given instance id typedef, return the string version of it
const std::string& DGNFilterLibsB1_0_8::getInstanceIdString(unsigned short int id) const
{
    std::map<unsigned short int, std::string>::const_iterator idIter = m_instanceIdToStringMap.find(id);

    if (idIter != m_instanceIdToStringMap.end()) return idIter->second;

    return m_dummy;
}

// Other way, given instance id string, return the typedef
const unsigned short int DGNFilterLibsB1_0_8::getInstanceId(const std::string& id) const
{
    unsigned short int typeId = -1;

    std::map<std::string,unsigned short int>::const_iterator idIter = m_instanceStringToIdMap.find(id);

    if (idIter != m_instanceStringToIdMap.end()) typeId = idIter->second;

    return typeId;
}
