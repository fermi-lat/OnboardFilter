/** 
* @file OnboardFilter_load.cxx
* @brief This is needed for forcing the linker to load all components
* of the library.
*
*  $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/Dll/OnboardFilter_load.cxx,v 1.7 2005/09/06 16:42:29 burnett Exp $
*/

#include "GaudiKernel/DeclareFactoryEntries.h"

// There should be one entry for each component included in 
// the library for this package.
DECLARE_FACTORY_ENTRIES(OnboardFilter) {
    DECLARE_ALGORITHM(OnboardFilter);
    DECLARE_ALGORITHM(FilterTracks);
	DECLARE_ALGORITHM(FilterAlg);
    DECLARE_TOOL(FilterDisplay);
} 
