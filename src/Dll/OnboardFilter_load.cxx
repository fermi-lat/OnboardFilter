/** 
* @file OnboardFilter_load.cxx
* @brief This is needed for forcing the linker to load all components
* of the library.
*
*  $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/Dll/OnboardFilter_load.cxx,v 1.8 2006/06/23 15:44:59 burnett Exp $
*/

#include "GaudiKernel/DeclareFactoryEntries.h"

// There should be one entry for each component included in 
// the library for this package.
DECLARE_FACTORY_ENTRIES(OnboardFilter) {
    DECLARE_ALGORITHM(OnboardFilter);
    DECLARE_TOOL(GammaFilterTool);
    DECLARE_TOOL(FSWAuxLibsTool);
    DECLARE_TOOL(MIPFilterTool);
    DECLARE_TOOL(HIPFilterTool);
    DECLARE_TOOL(DGNFilterTool);
    DECLARE_TOOL(FilterTrackTool);
    DECLARE_TOOL(TkrOutputTool);
    DECLARE_TOOL(CalOutputTool);
    DECLARE_TOOL(GemOutputTool);
} 
