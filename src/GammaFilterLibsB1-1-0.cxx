#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "GammaFilterLibsB1-1-0.h"

#include "GFC_DB/GFC_DB_V2_schema.h"
#include "GFC_DB/GAMMA_DB_instance.h"

GammaFilterLibsB1_1_0::GammaFilterLibsB1_1_0()
{
    // Release for this setup 
    m_release        = "B1-1-0";

    // The Filter library name
    m_libName        = "gfc";

    // Path to this library
    m_libPath        = "$(OBFEFCBINDIR)/gfc";

    // Path to the master configuration file
    m_configBasePath = "$(OBFGFC_DBBINDIR)";

    // Name of the master configuration file
    m_masterName     = "gamma_master";

    // Schema id for this filter
    m_schemaId       = GAMMA_DB_SCHEMA;

    // Master configuration instance id
    m_masterInstance = GAMMA_DB_INSTANCE_K_MASTER;

    // Schema id to file names
    // This maps the configuration id to a corresponding file name for the library to load
    // If the configurations change then this needs to be updated. 
    m_idToFile.clear();

    m_idToFile[SchemaPair(GAMMA_DB_SCHEMA,GAMMA_DB_INSTANCE_K_MASTER)]          = "gamma_master";
    m_idToFile[SchemaPair(GAMMA_DB_SCHEMA,GAMMA_DB_INSTANCE_K_NORMAL)]          = "gamma_normal";
    m_idToFile[SchemaPair(GAMMA_DB_SCHEMA,GAMMA_DB_INSTANCE_K_NORMAL_LEAK)]     = "gamma_normal_leak";
    m_idToFile[SchemaPair(GAMMA_DB_SCHEMA,GAMMA_DB_INSTANCE_K_CALIB)]           = "gamma_calib";
    m_idToFile[SchemaPair(GAMMA_DB_SCHEMA,GAMMA_DB_INSTANCE_K_CALIB_LEAK)]      = "gamma_calib_leak";
    m_idToFile[SchemaPair(GAMMA_DB_SCHEMA,GAMMA_DB_INSTANCE_K_CALIB_A)]         = "gamma_calib_a";
    m_idToFile[SchemaPair(GAMMA_DB_SCHEMA,GAMMA_DB_INSTANCE_K_CALIB_A_LEAK)]    = "gamma_calib_a_leak";
    m_idToFile[SchemaPair(GAMMA_DB_SCHEMA,GAMMA_DB_INSTANCE_K_EFF)]             = "gamma_eff";
    m_idToFile[SchemaPair(GAMMA_DB_SCHEMA,GAMMA_DB_INSTANCE_K_EFF_TAPER)]       = "gamma_eff_taper";
    m_idToFile[SchemaPair(GAMMA_DB_SCHEMA,GAMMA_DB_INSTANCE_K_NORMAL_LEAK_21A)] = "gamma_normal_leak_21a";
    m_idToFile[SchemaPair(GAMMA_DB_SCHEMA,GAMMA_DB_INSTANCE_K_LOWE_VETO)]       = "gamma_lowe_veto";

    m_dummy = "No valid id found";

    // Provide a mechanism to convert an instance typedef to a string (for convenience)
    m_instanceIdToStringMap.clear();

    m_instanceIdToStringMap[GAMMA_DB_INSTANCE_K_MASTER]          = "GAMMA_DB_INSTANCE_K_MASTER";
    m_instanceIdToStringMap[GAMMA_DB_INSTANCE_K_NORMAL]          = "GAMMA_DB_INSTANCE_K_NORMAL";
    m_instanceIdToStringMap[GAMMA_DB_INSTANCE_K_NORMAL_LEAK]     = "GAMMA_DB_INSTANCE_K_NORMAL_LEAK";
    m_instanceIdToStringMap[GAMMA_DB_INSTANCE_K_CALIB]           = "GAMMA_DB_INSTANCE_K_CALIB";
    m_instanceIdToStringMap[GAMMA_DB_INSTANCE_K_CALIB_LEAK]      = "GAMMA_DB_INSTANCE_K_CALIB_LEAK";
    m_instanceIdToStringMap[GAMMA_DB_INSTANCE_K_CALIB_A]         = "GAMMA_DB_INSTANCE_K_CALIB_A";
    m_instanceIdToStringMap[GAMMA_DB_INSTANCE_K_CALIB_A_LEAK]    = "GAMMA_DB_INSTANCE_K_CALIB_A_LEAK";
    m_instanceIdToStringMap[GAMMA_DB_INSTANCE_K_EFF]             = "GAMMA_DB_INSTANCE_K_EFF";
    m_instanceIdToStringMap[GAMMA_DB_INSTANCE_K_EFF_TAPER]       = "GAMMA_DB_INSTANCE_K_EFF_TAPER";
    m_instanceIdToStringMap[GAMMA_DB_INSTANCE_K_NORMAL_LEAK_21A] = "GAMMA_DB_INSTANCE_K_NORMAL_LEAK_21A";
    m_instanceIdToStringMap[GAMMA_DB_INSTANCE_K_NORMAL_LEAK_17A] = "GAMMA_DB_INSTANCE_K_NORMAL_LEAK_17A";
    m_instanceIdToStringMap[GAMMA_DB_INSTANCE_K_LOWE_VETO]       = "GAMMA_DB_INSTANCE_K_LOWE_VETO";

    // go the other way
    m_instanceStringToIdMap.clear();

    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_MASTER"]          = GAMMA_DB_INSTANCE_K_MASTER;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_NORMAL"]          = GAMMA_DB_INSTANCE_K_NORMAL;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_NORMAL_LEAK"]     = GAMMA_DB_INSTANCE_K_NORMAL_LEAK;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_CALIB"]           = GAMMA_DB_INSTANCE_K_CALIB;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_CALIB_LEAK"]      = GAMMA_DB_INSTANCE_K_CALIB_LEAK;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_CALIB_A"]         = GAMMA_DB_INSTANCE_K_CALIB_A;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_CALIB_A_LEAK"]    = GAMMA_DB_INSTANCE_K_CALIB_A_LEAK;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_EFF"]             = GAMMA_DB_INSTANCE_K_EFF;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_EFF_TAPER"]       = GAMMA_DB_INSTANCE_K_EFF_TAPER;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_NORMAL_LEAK_21A"] = GAMMA_DB_INSTANCE_K_NORMAL_LEAK_21A;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_NORMAL_LEAK_17A"] = GAMMA_DB_INSTANCE_K_NORMAL_LEAK_17A;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_LOWE_VETO"]       = GAMMA_DB_INSTANCE_K_LOWE_VETO;

    // Set EFC_DB_Schema filter id and cnt to "bad" values
    m_schema.filter.id  = 0xFFFF;
    m_schema.filter.cnt = 0;
}

GammaFilterLibsB1_1_0::~GammaFilterLibsB1_1_0()
{
}

// Given instance id typedef, return the string version of it
const std::string& GammaFilterLibsB1_1_0::getInstanceIdString(unsigned short int id) const
{
    std::map<unsigned short int, std::string>::const_iterator idIter = m_instanceIdToStringMap.find(id);

    if (idIter != m_instanceIdToStringMap.end()) return idIter->second;

    return m_dummy;
}

// Other way, given instance id string, return the typedef
const unsigned short int GammaFilterLibsB1_1_0::getInstanceId(const std::string& id) const
{
    unsigned short int typeId = -1;

    std::map<std::string,unsigned short int>::const_iterator idIter = m_instanceStringToIdMap.find(id);

    if (idIter != m_instanceStringToIdMap.end()) typeId = idIter->second;

    return typeId;
}
