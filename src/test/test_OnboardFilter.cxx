// $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/test/test_OnboardFilter.cxx,v 1.7 2004/12/17 23:24:05 usher Exp $
// Include files
// Gaudi system includes
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Algorithm.h"

// TDS class declarations: input data, and McParticle tree

#include "Event/TopLevel/EventModel.h"

// Define the class here instead of in a header file: 
//  not needed anywhere but here!
//----------------------------------------------------
/** 
* test_OnboardFilter
*
* @brief  A miminal test of OnboardFilter, using as few other packages as possible
*
* @author Tracy Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/test/test_OnboardFilter.cxx,v 1.7 2004/12/17 23:24:05 usher Exp $
*/

class test_OnboardFilter : public Algorithm {
public:
    test_OnboardFilter(const std::string& name, ISvcLocator* pSvcLocator);
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();
    
private: 
    //! number of times called
    int m_count; 
    //! the GlastDetSvc used for access to detector info
};
//------------------------------------------------------------------------

// necessary to define a Factory for this algorithm
// expect that the xxx_load.cxx file contains a call     
//     DLL_DECL_ALGORITHM( test_OnboardFilter );

static const AlgFactory<test_OnboardFilter>  Factory;
const IAlgFactory& test_OnboardFilterFactory = Factory;

//------------------------------------------------------------------------
//! ctor
test_OnboardFilter::test_OnboardFilter(const std::string& name, 
                             ISvcLocator* pSvcLocator)
:Algorithm(name, pSvcLocator)
,m_count(0)
{
}

//------------------------------------------------------------------------
//! set parameters and attach to various perhaps useful services.
StatusCode test_OnboardFilter::initialize(){
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "initialize" << endreq;
    
    return sc;
}

//------------------------------------------------------------------------
//! process an event
StatusCode test_OnboardFilter::execute()
{
    
    // First stab a a test program
    // can be fleshed out as required
    
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream   log( msgSvc(), name() );
    log << MSG::INFO << endreq <<  "Call " << ++m_count << ": " ;
    
    // First, the collection of TkrDigis is retrieved from the TDS
    //SmartDataPtr<Event::TkrDigiCol> digiCol(eventSvc(),
    //    EventModel::Digi::TkrDigiCol );
    
    //if (digiCol == 0) {
    //    log << "no TkrDigiCol found" << endreq;
    //    sc = StatusCode::FAILURE;
    //    return sc;
    //} else {
    //    log << digiCol->size() << " TKR digis found " << endreq;
    //}
    
    return sc;
}

//------------------------------------------------------------------------
//! clean up, summarize
StatusCode test_OnboardFilter::finalize(){
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    //log  << MSG::INFO << m_count << " call(s)." << endreq;
    
    return sc;
}


