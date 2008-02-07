/** @file GammaCfgPrms.h
*
* @class GammaCfgPrms
*
* @brief Class to allow modification of Gamma Filter parameters via Job Options 
*
* @authors T. Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/GammaCfgPrms.h,v 1.2 2008/01/09 20:57:53 usher Exp $
*/

#ifndef __GammaCfgPrms_H
#define __GammaCfgPrms_H

#include "IFilterCfgPrms.h"

class GammaFilterCfgPrms : virtual public IFilterCfgPrms
{
public:
    GammaFilterCfgPrms() : m_Zbottom_Emin(0xFFFFFFFF), m_Tkr_ZeroEmin(0xFFFFFFFF) {}
    virtual ~GammaFilterCfgPrms() {}

    // This defines the method called for end of event processing
    virtual void setCfgPrms(void* cfgPrms);

    // Set the parameters (done via job options file)
    void set_Zbottom_Emin(unsigned int prm) {m_Zbottom_Emin = prm;}
    void set_Tkr_ZeroEmin(unsigned int prm) {m_Tkr_ZeroEmin = prm;}

private:
    unsigned int m_Zbottom_Emin;
    unsigned int m_Tkr_ZeroEmin;
};

#endif // __ObfInterface_H
