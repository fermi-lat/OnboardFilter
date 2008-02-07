/** @file IFilterCfgPrms.h

* @class IFilterCfgPrms
*
* @brief Pure virtual interface class for filter configuration parameter control
*
* @authors T. Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/IFilterCfgPrms.h,v 1.2 2008/01/09 20:57:53 usher Exp $
*/

#ifndef __IFilterCfgPrms_H
#define __IFilterCfgPrms_H

class IFilterCfgPrms 
{
public:
    virtual ~IFilterCfgPrms() {}

    // This defines the method called for end of event processing
    virtual void setCfgPrms(void* cfgPrms) = 0;
};

#endif // __IFilterCfgPrms_H
