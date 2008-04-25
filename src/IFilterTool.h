/** @file IBbgndTupleSelectTool.h

    @brief declaration of the IBkgndSelectTool class

$Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/IFilterTool.h,v 1.0 2007/11/09 19:06:19 usher Exp $

*/

#ifndef IFilterTool_h
#define IFilterTool_h

#include "GaudiKernel/IAlgTool.h"

#include <string>
#include <vector>

// Forward declarations
#ifndef EDS_fwIxb 
    typedef struct _EDS_fwIxb EDS_fwIxb;
#endif

/** @class IFilterTool
    @brief Provides interface to the GSW tool to instantiate and control the various onboard filters
    @author Tracy Usher
*/

static const InterfaceID IID_IFilterTool("IFilterTool", 1 , 0);

class IFilterTool : virtual public IAlgTool
{
public:

    // Retrieve interface ID
    static const InterfaceID& interfaceID() { return IID_IFilterTool; }

    // This defines the method called for end of event processing
    virtual void eoeProcessing(EDS_fwIxb* ixb) = 0;

    // This for end of run processing
    virtual void eorProcessing() = 0;

    // Dump out the running configuration
    virtual void dumpConfiguration() = 0;
};

// This for end of event and end of run processing
typedef std::vector<IFilterTool*> OutputRtnVec;

#endif
