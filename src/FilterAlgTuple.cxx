
/** @file FilterAlgTuple.cxx
@brief Declaration and implementation of FilterAlgTuple

$Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/FilterAlgTuple.cxx,v 1.7 2007/03/14 22:44:59 lsrea Exp $

*/
#include "ntupleWriterSvc/INTupleWriterSvc.h"


#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "OnboardFilterTds/FilterStatus.h"
#include "OnboardFilterTds/ObfFilterStatus.h"
#include "OnboardFilterTds/FilterAlgTds.h"

#include <string>
#include <cmath>
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** @class FilterAlgTuple
@brief generate tuple stuff for the Onboard Filter
*/

namespace{
    bool disabled = true;
}
class FilterAlgTuple : public Algorithm {

public:
    FilterAlgTuple(const std::string& name, ISvcLocator* pSvcLocator); 

    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

private:


    StringProperty m_eventTreeName;

    INTupleWriterSvc* m_rootTupleSvc;;
    double m_statusHi, m_statusLo,m_separation;
    double m_filterAlgStatus;
    double m_filtxdir,m_filtydir ,m_filtzdir;
    float m_energy;
    double m_slopeYZ,m_slopeXZ;
    int m_xHits, m_yHits;

    int m_gamStatus;
    int m_cnoStatus;
    int m_mipStatus;
    int m_dfcStatus;

    int m_warnNoFilterStatus;   // count WARNINGs: no FilterStatus found
 
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static const AlgFactory<FilterAlgTuple>  Factory;
const IAlgFactory& FilterAlgTupleFactory = Factory;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FilterAlgTuple::FilterAlgTuple(const std::string& name, ISvcLocator* pSvcLocator) :
Algorithm(name, pSvcLocator),  m_rootTupleSvc(0)
{

    declareProperty("EventTreeName",     m_eventTreeName="MeritTuple");


}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
StatusCode FilterAlgTuple::initialize() {

    StatusCode  sc = StatusCode::SUCCESS;
    if(disabled) return sc;
    
    MsgStream log(msgSvc(), name());

    // Use the Job options service to get the Algorithm's parameters
    setProperties();
    m_warnNoFilterStatus = 0;   // Zero counter for warnings

    // get a pointer to RootTupleSvc 
    if( (sc = service("RootTupleSvc", m_rootTupleSvc, true) ). isFailure() ) {
        log << MSG::ERROR << " failed to get the RootTupleSvc" << endreq;
        return sc;
    }

    /** @page MeritTuple
  	       
      @section Filter FilterAlg Variables

      @verbatim

      Name                          Bit  Explanation
      
   DFC_V_STATUS_ACD                   0  ACD was analyzed                
   DFC_V_STATUS_DIR           	      1	 DIR was decoded                 
   DFC_V_STATUS_ATF            	      2	 ACD/TKR veto was analyzed       
   DFC_V_STATUS_CAL1          	      3  CAL was analyzed, phase 1       
   DFC_V_STATUS_TKR           	      4  TKR finding was done            
   DFC_V_STATUS_ACD_TOP     	      5  ACD top  tile struck            
   DFC_V_STATUS_ACD_SIDE     	      6  ACD side tile struck            
   DFC_V_STATUS_ACD_SIDE_FILTER       7	 ACD      filter tile struck     
   DFC_V_STATUS_TKR_POSSIBLE          8	 Possible track                  
   DFC_V_STATUS_TKR_TRIGGER           9	 Have a 3-in-a-row trigger       
   DFC_V_STATUS_CAL_LO         	     10  Cal Lo Trigger                  
   DFC_V_STATUS_CAL_HI         	     11  Cal Hi Trigger                  
   DFC_V_STATUS_TKR_EQ_1       	     12  Exactly 1 track                 
   DFC_V_STATUS_TKR_GE_2             13	 Greater or equal 2 tracks       
   DFC_V_STATUS_TKR_THROTTLE         14	 Throttle bit set                

   DFC_V_STATUS_TKR_LT_2_ELO         15  Low energy, no 2 track evidence   
   DFC_V_STATUS_TKR_SKIRT            16  Event into the skirt region     
   DFC_V_STATUS_TKR_EQ_0             17  No tracks                       
   DFC_V_STATUS_TKR_ROW2       	     18  Track Row 2 match               
   DFC_V_STATUS_TKR_ROW01            19  Track Row 0 or 1 match          
   DFC_V_STATUS_TKR_TOP              20  Track Top match                 
   DFC_V_STATUS_ZBOTTOM        	     21  No tracks into CAL with energy  
   DFC_V_STATUS_EL0_ETOT_90    	     22  E layer 0/ETOT > .90            
   DFC_V_STATUS_EL0_ETOT_01          23  E layer 0/ETOT < .01            
   DFC_V_STATUS_SIDE                 24	 Event has a side face veto      
   DFC_V_STATUS_TOP            	     25	 Event has a top  face veto      
   DFC_V_STATUS_SPLASH_1       	     26  Event has a splash veto         
   DFC_V_STATUS_E350_FILTER_TILE     27  Event <350Mev  + filter tiles   
   DFC_V_STATUS_E0_TILE              28  Event 0 energy + tile hit       
   DFC_V_STATUS_SPLASH_0             29  Event has a splash veto         
   DFC_V_STATUS_NOCALLO_FILTER_TILE  30  No CAL LO trigger + filter tile 
   DFC_V_STATUS_VETOED               31  Any veto                        
@endverbatim

<table>
  <tr><th> Variable <th> Type <th> Description
  <tr><td>  FilterAlgStatus
  <td>D<td>    Status generated by FilterAlg 
  <tr><td>  FilterStatus_HI
  <td>D<td>    bits 15-31 of the filter status word (17 bits) 
  <tr><td>  FilterStatus_LO
  <td>D<td>    bits  0-14 of the filter status word (15 bits)
  <tr><td>  FilterXhits
  <td>D<td>    number of hits on best track XZ slope
  <tr><td>  FilterYhits
  <td>D<td>    number of hits on best track YZ slope
  <tr><td>  FilterXZslope
  <td>D<td>    XZ slope
  <tr><td>  FilterYZslope
  <td>D<td>    YZ slope
  <tr><td>  FilterEnergy
  <td>D<td>    Energy as determined by onboard alg
  <tr><td>  FilterAngSep   
  <td>D<td>    Filter status separation
</table> 

*/

    m_rootTupleSvc->addItem(m_eventTreeName, "FilterStatus_HI", &m_statusHi);
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterStatus_LO", &m_statusLo );
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterAlgStatus", &m_filterAlgStatus );
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterAngSep",    &m_separation );
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterEnergy",    &m_energy );
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterXhits",     &m_xHits );
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterYhits",     &m_yHits );
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterSlopeYZ",   &m_slopeYZ );
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterSlopeXZ",   &m_slopeXZ );
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterXDir",      &m_filtxdir );
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterYDir",      &m_filtydir );
    m_rootTupleSvc->addItem(m_eventTreeName, "FilterZDir",      &m_filtzdir );

    m_rootTupleSvc->addItem(m_eventTreeName, "ObfGamStatus",    &m_gamStatus);
    m_rootTupleSvc->addItem(m_eventTreeName, "ObfCnoStatus",    &m_cnoStatus);
    m_rootTupleSvc->addItem(m_eventTreeName, "ObfMipStatus",    &m_mipStatus);
    m_rootTupleSvc->addItem(m_eventTreeName, "ObfDfcStatus",    &m_dfcStatus);

    return sc;
}

//------------------------------------------------------------------------------
StatusCode FilterAlgTuple::execute() {

    StatusCode  sc = StatusCode::SUCCESS;
    if(disabled) return sc;

    MsgStream log(msgSvc(), name());

    // Old school output
    SmartDataPtr<OnboardFilterTds::FilterStatus> filterStatus(eventSvc(), "/Event/Filter/FilterStatus");
    if( filterStatus ){
        m_statusHi=filterStatus->getHigh();
        m_statusLo=filterStatus->getLow();
        m_separation=filterStatus->getSeparation();
	m_energy=filterStatus->getCalEnergy();
	m_xHits = 0;
	m_yHits = 0;
        m_filtxdir=m_filtydir=m_filtzdir=0;
	double slopeXZ = 0.0;
	double slopeYZ = 0.0;
	double intXZ = 0.0;
	double intYZ = 0.0;
	filterStatus->getBestTrack(m_xHits,m_yHits,slopeXZ,slopeYZ,intXZ,intYZ);
	if(m_xHits>0&&m_yHits>0){
        float alpha = atan2(slopeYZ,slopeXZ);
	  if(alpha < 0) {
	    alpha = alpha+2.0*3.1415;
	  }
	  float m_slope = sqrt(pow(slopeXZ,2) + pow(slopeYZ,2));
	  float beta = atan(m_slope);
	  m_filtxdir = cos(alpha)*sin(beta);
	  m_filtydir = sin(alpha)*sin(beta);
	  m_filtzdir = cos(beta);
	}
	m_slopeYZ=slopeYZ;
	m_slopeXZ=slopeXZ;
    }else {
        m_statusHi = m_statusLo = 0;

        m_warnNoFilterStatus++;
        if (   m_warnNoFilterStatus <= 10 ) {
            log << MSG::WARNING << "FilterStatus not found" ;
            if ( m_warnNoFilterStatus == 10 ) {
                log << " -- Further WARNINGs on missing FilterStatus are suppressed"; }
            log  << endreq;
        }
    }

    // Beyond old school
    SmartDataPtr<FilterAlgTds::FilterAlgData> filterAlgStatus(eventSvc(),"/Event/Filter/FilterAlgData");
    if(filterAlgStatus){
        m_filterAlgStatus=(double)filterAlgStatus->getVetoWord();
    }

    // ultra modern method
    SmartDataPtr<OnboardFilterTds::ObfFilterStatus> obfStatus(eventSvc(), "/Event/Filter/ObfFilterStatus");

    if (obfStatus)
    {
        // Pointer to our retrieved objects
        const OnboardFilterTds::IObfStatus* obfResult = 0;

        // Start with Gamma Filter
        obfResult   = obfStatus->getFilterStatus(OnboardFilterTds::ObfFilterStatus::GammaFilter);
        m_gamStatus = obfResult ? obfResult->getStatus32() : -1;

        // Get the CNO (HFC) Filter
        obfResult   = obfStatus->getFilterStatus(OnboardFilterTds::ObfFilterStatus::CNOFilter);
        m_cnoStatus = obfResult ? obfResult->getStatus32() : -1;

        // Get the MIP filter
        obfResult   = obfStatus->getFilterStatus(OnboardFilterTds::ObfFilterStatus::MipFilter);
        m_mipStatus = obfResult ? obfResult->getStatus32() : -1;

        // Get the Diagnostic filter
        obfResult   = obfStatus->getFilterStatus(OnboardFilterTds::ObfFilterStatus::DFCFilter);
        m_dfcStatus = obfResult ? obfResult->getStatus32() : -1;
    }

    return sc;
}
//------------------------------------------------------------------------------
StatusCode FilterAlgTuple::finalize() {

    if(disabled) return StatusCode::SUCCESS;

    MsgStream log(msgSvc(), name());
    log << MSG::INFO ;
    log << endreq;
    if(m_warnNoFilterStatus>0)
        log << MSG::INFO << "Number of warnings (FilterStatus not found): "<< m_warnNoFilterStatus << endreq;

    setFinalized(); //  prevent being called again

    return StatusCode::SUCCESS;
}
