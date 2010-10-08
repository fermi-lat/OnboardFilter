/**  @file GemOutputTool.cxx
    @brief implementation of class GemOutputTool
    
  $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/GemOutputTool.cxx,v 1.4.300.1 2010/09/18 03:51:22 heather Exp $  
*/

#include "IFilterTool.h"

#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/GaudiException.h" 
#include "GaudiKernel/IDataProviderSvc.h"

#include "Event/TopLevel/Event.h"
#include "Event/TopLevel/EventModel.h"
#include "OnboardFilterTds/FilterStatus.h"
#include "OnboardFilterTds/ObfFilterStatus.h"

#include "facilities/Util.h"

#include "trackProj.h"
#include "GrbTrack.h"
#include "OnboardFilterTds/FilterStatus.h"
#include "OnboardFilterTds/Obf_TFC_prjs.h"

// Interface to EDS package here
#include "ObfInterface.h"

// FSW includes go here
#include "EFC/EFC_edsFw.h"
#include "EDS/EBF_dir.h"
#include "EDS/EBF_cid.h"
#include "EDS/EBF_gem.h"

// Useful stuff! 
#include <map>
#include <stdexcept>
#include <sstream>
#include <stdexcept>

/** @class GemOutputTool
    @brief Manages the Gamma Filter
    @author Tracy Usher
*/
class GemOutputTool : public AlgTool, virtual public IFilterTool
{
public:

    // Standard Gaudi Tool constructor
    GemOutputTool(const std::string& type, const std::string& name, const IInterface* parent);

    // After building it, destroy it
    ~GemOutputTool();

    /// @brief Intialization of the tool
    StatusCode initialize();

    /// @brief Finalize method for the tool
    StatusCode finalize();

    // Set Mode for a given filter
    void setMode(unsigned int mode) {return;}

    // This defines the method called for end of event processing
    virtual void eoeProcessing(EDS_fwIxb* ixb);

    // This for end of run processing
    virtual void eorProcessing();

    // Dump out the running configuration
    void dumpConfiguration();

private:

    //****** This section for defining JO parameters
    // This is somewhat useless but if set will be passed to the CDM utility to print info

    //****** This section contains various useful member variables
    /// Pointer to the Gaudi data provider service
    IDataProviderSvc* m_dataSvc;
};

//static ToolFactory<GemOutputTool> s_factory;
//const IToolFactory& GemOutputToolFactory = s_factory;
DECLARE_TOOL_FACTORY(GemOutputTool);
//------------------------------------------------------------------------

GemOutputTool::GemOutputTool(const std::string& type, 
                                 const std::string& name, 
                                 const IInterface* parent) :
                                 AlgTool(type, name, parent)
{
    //Declare the additional interface
    declareInterface<IFilterTool>(this);

    // declare properties with setProperties calls
    //declareProperty("FillTowerHits",   m_towerHits = true);

    return;
}
//------------------------------------------------------------------------
GemOutputTool::~GemOutputTool()
{
}

StatusCode GemOutputTool::initialize()
{
    StatusCode sc   = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());

    // Set the properties
    setProperties();

    //Locate and store a pointer to the data service
    if( (sc = service("EventDataSvc", m_dataSvc)).isFailure() ) 
    {
        log << MSG::ERROR << "Service [EventDataSvc] not found" << endreq;
        return sc;
    }

    try
    {
        // Get ObfInterface pointer
        ObfInterface* obf = ObfInterface::instance();

        // Register this as an output routine
        obf->setEovOutputCallBack(this);
    }
    catch(ObfInterface::ObfException& obfException)
    {
        log << MSG::ERROR << obfException.m_what << endreq;
        return StatusCode::FAILURE;
    }

    return sc;
}

StatusCode GemOutputTool::finalize ()
{
    StatusCode  status = StatusCode::SUCCESS;
    
    return status;
}

// This routine for dumping to log file the configuration being run
void GemOutputTool::dumpConfiguration()
{
    MsgStream log(msgSvc(), name());

    return;
}

// This defines the method called for end of event processing
void GemOutputTool::eoeProcessing(EDS_fwIxb* ixb)
{
    // Retrieve the old FilterStatus output TDS object as it needed to be already created to get this far
    SmartDataPtr<OnboardFilterTds::FilterStatus> filterStatus(m_dataSvc,"/Event/Filter/FilterStatus");

    if (!filterStatus)
    {
        throw std::runtime_error("GemOutputTool cannot find FilterStatus in the TDS");
    }

//  Get a pointer to the directory
    EDS_fwEvt      *evt =  &ixb->blk.evt;
    const EBF_dir  *dir = evt->dir;

//  Get the GEM info
    const EBF_gem *gem;
    gem = (const EBF_gem *)dir->ctbs[EBF_CID_K_GEM].ctb->dat;

    filterStatus->setGemThrTkr(gem->thrTkr);
    filterStatus->setGemCalHiLo(gem->calHiLo);
    filterStatus->setGemCondsumCno(gem->condsumCno);
    filterStatus->setGemAcd_vetoes_XZ(gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XZ]);
    filterStatus->setGemAcd_vetoes_YZ(gem->acd.vetoes[EBF_GEM_ACD_VETO_K_YZ]);
    filterStatus->setGemAcd_vetoes_XY(gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XY]);
    filterStatus->setGemAcd_vetoes_RU(gem->acd.vetoes[EBF_GEM_ACD_VETO_K_RU]);
    filterStatus->setGemLivetime(gem->livetime);
    filterStatus->setGemTrgtime(gem->trgtime);
    filterStatus->setGemPpstime(gem->ppstime);
    filterStatus->setGemDiscarded(gem->discarded);
    filterStatus->setGemPrescaled(gem->prescaled);

    // This appears to be missing?
    //m_filterStatus->setGemSent(myFilterInfo.sent);

    unsigned int acd_top = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XY];
    unsigned int acd_x   = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_YZ];
    unsigned int acd_y   = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XZ];

    filterStatus->setAcdMap(acd_y,acd_x,acd_top);

    return;
}

// This for end of run processing
void GemOutputTool::eorProcessing()
{
    MsgStream log(msgSvc(), name());

    return;
}
