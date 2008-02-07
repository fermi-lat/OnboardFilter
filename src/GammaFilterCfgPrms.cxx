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
#include "EFC/src/GFC_def.h"


void GammaFilterCfgPrms::setCfgPrms(void* cfgPrms)
{
    GFC_cfg* prms = ((GFC*)cfgPrms)->cfg;
    

    if (m_Zbottom_Emin != 0xFFFFFFFF) prms->prms.zbottom.emin    = m_Zbottom_Emin;
    if (m_Tkr_ZeroEmin != 0xFFFFFFFF) prms->prms.tkr.zeroTkrEmin = m_Tkr_ZeroEmin;
}
