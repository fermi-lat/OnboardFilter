#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "GammaFilterLibsB3-0-0.h"

#include "GFC_DB/GFC_DB_V3_schema.h"
#include "GFC_DB/GAMMA_DB_instance.h"

GammaFilterLibsB3_0_0::GammaFilterLibsB3_0_0()
{
    // Release for this setup 
    m_release        = "B3-0-0";

    // The Filter library name
    //    m_libName        = "gfc";
    m_libName        = "ggfc";

    // Path to this library
    m_libPath        = "$(OBFEFCBINDIR)/ggfc";

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
    m_idToFile[SchemaPair(GAMMA_DB_SCHEMA,GAMMA_DB_INSTANCE_K_NORMAL_LEAK_21A)] = "gamma_normal_leak21a";
    m_idToFile[SchemaPair(GAMMA_DB_SCHEMA,GAMMA_DB_INSTANCE_K_NORMAL_10GEV)]    = "gamma_normal_10GeV";
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
    m_instanceIdToStringMap[GAMMA_DB_INSTANCE_K_NORMAL_10GEV]    = "GAMMA_DB_INSTANCE_K_NORMAL_10GEV";
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
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_NORMAL_10GEV"]    = GAMMA_DB_INSTANCE_K_NORMAL_10GEV;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_NORMAL_LEAK_21A"] = GAMMA_DB_INSTANCE_K_NORMAL_LEAK_21A;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_NORMAL_LEAK_17A"] = GAMMA_DB_INSTANCE_K_NORMAL_LEAK_17A;
    m_instanceStringToIdMap["GAMMA_DB_INSTANCE_K_LOWE_VETO"]       = GAMMA_DB_INSTANCE_K_LOWE_VETO;

    // Set EFC_DB_Schema filter id and cnt to "bad" values
    m_schema.filter.id  = 0xFFFF;
    m_schema.filter.cnt = 0;

    // Strings for output at end of processing

    // Descriptor for each bit
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_GEM_THROTTLE       ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_GEM_TKR            ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_GEM_CALLO          ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_GEM_CALHI          ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_GEM_CNO            ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_ACD_TOP            ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_ACD_SIDE           ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_ACD_SIDE_FILTER    ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_TKR_EQ_1           ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_TKR_GE_2           ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_RSVD_10            ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_TKR_LT_2_ELO       ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_TKR_SKIRT          ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_TKR_EQ_0           ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_TKR_ROW2           ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_TKR_ROW01          ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_TKR_TOP            ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_SIDE               ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_TOP                ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_ZBOTTOM            ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_ERR_CTB            ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_SPLASH_1           ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_E350_FILTER_TILE   ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_E0_TILE            ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_EL0_ETOT_HI        ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_EL0_ETOT_LO        ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_LO_ENERGY          ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_HI_ENERGY          ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_SPLASH_0           ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_NOCALLO_FILTER_TILE");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_NON_PHYSICS        ");
    m_statusWordStringVec.push_back("GFC_V3_STATUS_M_VETOED             ");

}

GammaFilterLibsB3_0_0::~GammaFilterLibsB3_0_0()
{
}

// Given instance id typedef, return the string version of it
const std::string& GammaFilterLibsB3_0_0::getInstanceIdString(unsigned short int id) const
{
    std::map<unsigned short int, std::string>::const_iterator idIter = m_instanceIdToStringMap.find(id);

    if (idIter != m_instanceIdToStringMap.end()) return idIter->second;

    return m_dummy;
}

// Other way, given instance id string, return the typedef
const unsigned short int GammaFilterLibsB3_0_0::getInstanceId(const std::string& id) const
{
    unsigned short int typeId = -1;

    std::map<std::string,unsigned short int>::const_iterator idIter = m_instanceStringToIdMap.find(id);

    if (idIter != m_instanceStringToIdMap.end()) typeId = idIter->second;

    return typeId;
}

// Return string description of a given bit in status word
const std::string& GammaFilterLibsB3_0_0::getStatWordDesc(int idx) const
{
    if (idx < 0 || idx >= m_statusWordStringVec.size()) return "";

    return m_statusWordStringVec[idx];
}
