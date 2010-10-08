// $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/test/test_OnboardFilter.cxx,v 1.3.288.1 2010/09/18 03:51:22 heather Exp $
// Include files
// Gaudi system includes
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Algorithm.h"

// TDS class declarations: input data, and McParticle tree

#include "Event/TopLevel/EventModel.h"
#include "OnboardFilterTds/ObfFilterStatus.h"

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
* $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/test/test_OnboardFilter.cxx,v 1.3.288.1 2010/09/18 03:51:22 heather Exp $
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

//static const AlgFactory<test_OnboardFilter>  Factory;
//const IAlgFactory& test_OnboardFilterFactory = Factory;
DECLARE_ALGORITHM_FACTORY(test_OnboardFilter);
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
    log << MSG::INFO  <<  "Call " << ++m_count << endreq ;
    
    // First, the collection of TkrDigis is retrieved from the TDS
    SmartDataPtr<OnboardFilterTds::ObfFilterStatus> obfFilterStatus(eventSvc(),"/Event/Filter/ObfFilterStatus");

    // It better be there! 
    if (!obfFilterStatus)
    {
        log << MSG::ERROR << "No ObfFilterStatus found in the TDS! " << endreq;
        return StatusCode::FAILURE;
    }

    // Now step through and initialize the status objects one by one
    const OnboardFilterTds::IObfStatus* tdsStatus = 0;

    // We do this one by one explicitly for now. Start with the results of the gamma filter
    if ((tdsStatus = obfFilterStatus->getFilterStatus(OnboardFilterTds::ObfFilterStatus::GammaFilter)))
    {
        unsigned int status  = tdsStatus->getStatusWord();
        unsigned int summary = tdsStatus->getFiltersb();
        log << MSG::INFO << "*** Gamma Filter ***" << endreq;
        log << MSG::INFO << "    Status Word: " << std::hex << status << 
                            ", Summary Byte: " << std::hex << summary << endreq;
    }

    // MIP Filter
    if ((tdsStatus = obfFilterStatus->getFilterStatus(OnboardFilterTds::ObfFilterStatus::MIPFilter)))
    {
        unsigned int status  = tdsStatus->getStatusWord();
        unsigned int summary = tdsStatus->getFiltersb();
        log << MSG::INFO << "*** MIP Filter ***" << endreq;
        log << MSG::INFO << "    Status Word: " << std::hex << status << 
                            ", Summary Byte: " << std::hex << summary << endreq;
    }

    // HIP Filter
    if ((tdsStatus = obfFilterStatus->getFilterStatus(OnboardFilterTds::ObfFilterStatus::HIPFilter)))
    {
        unsigned int status  = tdsStatus->getStatusWord();
        unsigned int summary = tdsStatus->getFiltersb();
        log << MSG::INFO << "*** Gamma Filter ***" << endreq;
        log << MSG::INFO << "    Status Word: " << std::hex << status << 
                            ", Summary Byte: " << std::hex << summary << endreq;
    }

    // DGN Filter
    if ((tdsStatus = obfFilterStatus->getFilterStatus(OnboardFilterTds::ObfFilterStatus::DGNFilter)))
    {
        unsigned int status  = tdsStatus->getStatusWord();
        unsigned int summary = tdsStatus->getFiltersb();
        log << MSG::INFO << "*** DGN Filter ***" << endreq;
        log << MSG::INFO << "    Status Word: " << std::hex << status << 
                            ", Summary Byte: " << std::hex << summary << endreq;
    }
    
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



