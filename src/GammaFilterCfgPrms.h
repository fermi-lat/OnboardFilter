/** @file GammaCfgPrms.h
*
* @class GammaCfgPrms
*
* @brief Class to allow modification of Gamma Filter parameters via Job Options 
*
* @authors T. Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/GammaFilterCfgPrms.h,v 1.1 2008/02/07 19:36:42 usher Exp $
*/

#ifndef __GammaCfgPrms_H
#define __GammaCfgPrms_H

#include "IFilterCfgPrms.h"

class GammaFilterCfgPrms : virtual public IFilterCfgPrms
{
public:
    GammaFilterCfgPrms() :  m_Acd_TopSideEmax(0xFFFFFFFF)
                          , m_Acd_TopSideFilterEmax(0xFFFFFFFF)
                          , m_Acd_SplashEmax(0xFFFFFFFF)
                          , m_Acd_SplashCount(0xFFFFFFFF)
                          , m_Atf_Emax(0xFFFFFFFF)
                          , m_Zbottom_Emin(0xFFFFFFFF)
                          , m_Cal_Epass(0xFFFFFFFF)
                          , m_Cal_Emin(0xFFFFFFFF)
                          , m_Cal_Emax(0xFFFFFFFF)
                          , m_Cal_Layer0RatioLo(0xFFFFFFFF)
                          , m_Cal_Layer0RatioHi(0xFFFFFFFF)
                          , m_Tkr_Row2Emax(0xFFFFFFFF)
                          , m_Tkr_Row01Emax(0xFFFFFFFF)
                          , m_Tkr_TopEmax(0xFFFFFFFF)
                          , m_Tkr_ZeroTkrEmin(0xFFFFFFFF)
                          , m_Tkr_TwoTkrEmax(0xFFFFFFFF) {}
    virtual ~GammaFilterCfgPrms() {}

    // This defines the method called for end of event processing
    virtual void setCfgPrms(void* cfgPrms);

    // Set the parameters (done via job options file)
    void set_Acd_TopSideEmax       (unsigned int prm) {m_Acd_TopSideEmax       = prm;}
    void set_Acd_TopSideFilterEmax (unsigned int prm) {m_Acd_TopSideFilterEmax = prm;}
    void set_Acd_SplashEmax        (unsigned int prm) {m_Acd_TopSideFilterEmax = prm;}
    void set_Acd_SplashCount       (unsigned int prm) {m_Acd_TopSideFilterEmax = prm;}
    void set_Atf_Emax              (unsigned int prm) {m_Atf_Emax              = prm;}
    void set_Zbottom_Emin          (unsigned int prm) {m_Zbottom_Emin          = prm;}
    void set_Cal_Epass             (unsigned int prm) {m_Cal_Epass             = prm;}
    void set_Cal_Emin              (unsigned int prm) {m_Cal_Emin              = prm;}
    void set_Cal_Emax              (unsigned int prm) {m_Cal_Emax              = prm;}
    void set_Cal_Layer0RatioLo     (unsigned int prm) {m_Cal_Layer0RatioLo     = prm;}
    void set_Cal_Layer0RatioHi     (unsigned int prm) {m_Cal_Layer0RatioHi     = prm;}
    void set_Tkr_Row2Emax          (unsigned int prm) {m_Tkr_Row2Emax          = prm;}
    void set_Tkr_Row01Emax         (unsigned int prm) {m_Tkr_Row01Emax         = prm;}
    void set_Tkr_TopEmax           (unsigned int prm) {m_Tkr_TopEmax           = prm;}
    void set_Tkr_ZeroTkrEmin       (unsigned int prm) {m_Tkr_ZeroTkrEmin       = prm;}
    void set_Tkr_TwoTkrEmax        (unsigned int prm) {m_Tkr_TwoTkrEmax        = prm;}
    void set_Tkr_SkirtEmax         (unsigned int prm) {m_Tkr_SkirtEmax         = prm;}

private:
    unsigned int m_Acd_TopSideEmax;
    unsigned int m_Acd_TopSideFilterEmax;
    unsigned int m_Acd_SplashEmax;
    unsigned int m_Acd_SplashCount;

    unsigned int m_Atf_Emax;

    unsigned int m_Zbottom_Emin;

    unsigned int m_Cal_Epass;
    unsigned int m_Cal_Emin;
    unsigned int m_Cal_Emax;
    unsigned int m_Cal_Layer0RatioLo;
    unsigned int m_Cal_Layer0RatioHi;

    unsigned int m_Tkr_Row2Emax;
    unsigned int m_Tkr_Row01Emax;
    unsigned int m_Tkr_TopEmax;
    unsigned int m_Tkr_ZeroTkrEmin;
    unsigned int m_Tkr_TwoTkrEmax;
    unsigned int m_Tkr_SkirtEmax;
};

#endif // __ObfInterface_H
