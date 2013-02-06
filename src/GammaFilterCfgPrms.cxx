#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "GammaFilterCfgPrms.h"

//#include "EFC/EFC_edsFw.h"
//#include "EDS/EBF_dir.h"
//#include "EDS/EBF_cid.h"
//#include "EDS/EBF_tkr.h"
//#include "EDS/EDR_cal.h"
//#include "EDS/EDR_tkrUnpack.h"
//#include "EDS/FFS.h"
#include "EFC_DB/EFC_DB_sampler.h"
#if defined(OBF_B3_0_0) || defined(OBF_B3_1_0) || defined(OBF_B3_1_1) || defined(OBF_B3_1_3)
#include "EFC/GFC_def.h"
#else
#include "EFC/../src/GFC_def.h"
#endif


void GammaFilterCfgPrms::setCfgPrms(void* cfgPrms)
{
    GFC_cfg* prms = ((GFC*)cfgPrms)->cfg;
    
    if (m_Acd_TopSideEmax       != 0xFFFFFFFF) prms->prms.acd.topSideEmax       = m_Acd_TopSideEmax;
    if (m_Acd_TopSideFilterEmax != 0xFFFFFFFF) prms->prms.acd.topSideFilterEmax = m_Acd_TopSideFilterEmax;
    if (m_Acd_SplashEmax        != 0xFFFFFFFF) prms->prms.acd.splashEmax        = m_Acd_TopSideFilterEmax;
    if (m_Acd_SplashCount       != 0xFFFFFFFF) prms->prms.acd.splashCount       = m_Acd_TopSideFilterEmax;

    if (m_Atf_Emax              != 0xFFFFFFFF) prms->prms.atf.emax              = m_Atf_Emax;

    if (m_Zbottom_Emin          != 0xFFFFFFFF) prms->prms.zbottom.emin          = m_Zbottom_Emin;

    if (m_Cal_Epass             != 0xFFFFFFFF) prms->prms.cal.epass             = m_Cal_Epass;
    if (m_Cal_Emin              != 0xFFFFFFFF) prms->prms.cal.emin              = m_Cal_Emin;
    if (m_Cal_Emax              != 0xFFFFFFFF) prms->prms.cal.emax              = m_Cal_Emax;
    if (m_Cal_Layer0RatioLo     != 0xFFFFFFFF) prms->prms.cal.layer0RatioLo     = m_Cal_Layer0RatioLo;
    if (m_Cal_Layer0RatioHi     != 0xFFFFFFFF) prms->prms.cal.layer0RatioHi     = m_Cal_Layer0RatioHi;

    if (m_Tkr_Row2Emax          != 0xFFFFFFFF)   prms->prms.tkr.row2Emax        = m_Tkr_Row2Emax;
    if (m_Tkr_Row01Emax         != 0xFFFFFFFF)   prms->prms.tkr.row01Emax       = m_Tkr_Row01Emax;
    if (m_Tkr_TopEmax           != 0xFFFFFFFF)   prms->prms.tkr.topEmax         = m_Tkr_TopEmax;
    if (m_Tkr_ZeroTkrEmin       != 0xFFFFFFFF)   prms->prms.tkr.zeroTkrEmin     = m_Tkr_ZeroTkrEmin;
    if (m_Tkr_TwoTkrEmax        != 0xFFFFFFFF)   prms->prms.tkr.twoTkrEmax      = m_Tkr_TwoTkrEmax;
    if (m_Tkr_SkirtEmax         != 0xFFFFFFFF)   prms->prms.tkr.skirtEmax       = m_Tkr_SkirtEmax;
}
