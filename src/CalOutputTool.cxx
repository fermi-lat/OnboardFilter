/**  @file CalOutputTool.cxx
    @brief implementation of class CalOutputTool
    
  $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/CalOutputTool.cxx,v 1.4 2008/06/11 19:23:18 usher Exp $  
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
#include "EDS/ECR_cal.h"
#include "EDS/EDR_cal.h"
#include "EDS/EBF_cal.h"
#include "EDS/EDR_calUnpack.h"
#include <PBI/FFS.ih>

// Useful stuff! 
#include <map>
#include <stdexcept>
#include <sstream>
#include <stdexcept>

/** @class CalOutputTool
    @brief Manages the Gamma Filter
    @author Tracy Usher
*/
class CalOutputTool : public AlgTool, virtual public IFilterTool
{
public:

    // Standard Gaudi Tool constructor
    CalOutputTool(const std::string& type, const std::string& name, const IInterface* parent);

    // After building it, destroy it
    ~CalOutputTool();

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

static ToolFactory<CalOutputTool> s_factory;
const IToolFactory& CalOutputToolFactory = s_factory;
//------------------------------------------------------------------------

CalOutputTool::CalOutputTool(const std::string& type, 
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
CalOutputTool::~CalOutputTool()
{
}

StatusCode CalOutputTool::initialize()
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

StatusCode CalOutputTool::finalize ()
{
    StatusCode  status = StatusCode::SUCCESS;
    
    return status;
}

// This routine for dumping to log file the configuration being run
void CalOutputTool::dumpConfiguration()
{
    MsgStream log(msgSvc(), name());

    return;
}

// This defines the method called for end of event processing
void CalOutputTool::eoeProcessing(EDS_fwIxb* ixb)
{
    // Retrieve the old FilterStatus output TDS object as it needed to be already created to get this far
    SmartDataPtr<OnboardFilterTds::FilterStatus> filterStatus(m_dataSvc,"/Event/Filter/FilterStatus");

    if (!filterStatus)
    {
        throw std::runtime_error("CalOutputTool cannot find FilterStatus in the TDS");
    }

//  Get a pointer to the directory
    EDS_fwEvt      *evt       =  &ixb->blk.evt;
    const EBF_dir  *dir       = evt->dir;
    const  ECR_cal *constants = evt->calCal;
    EDR_cal        *cal       = evt->cal;

    EDR_calUnpack (cal, dir, evt->calCal);

    // A place for data
    LogInfo logData[16*8*12];    // 16 towers * 8 layers * 12 logs

    int twrMap     = EDR_CAL_TWRMAP_JUSTIFY (cal->twrMap);
    int numLogsHit = 0;

    for (int tower=0; tower<16; tower++) 
    {
        if ((twrMap    & FFSL_mask (tower)) > 0) 
        {
            const EDR_calTower*      ctr        = &cal->twrs[tower];
            int                      layerMap   =  EDR_CAL_TOWER_LAYERMAP_JUSTIFY (ctr->layerMap);
            const int*               energy     = ctr->logEnergies;
            const EBF_calLogData_cf *logs       = (EBF_calLogData_cf *)ctr->data + 1;
            const EBF_calLogData_cf *log        = logs;
            const EBF_calLogData_bf *log_bf     = (EBF_calLogData_bf *)ctr->data + 1;
            int                      four_range = ctr->flags & 1;
            int                      counts     = ctr->layerCnts;
            const  ECR_calTower     *calTower   = constants->twrs + tower;
            int                      nlogs      = 0;
            
            //printf("OBF: tower %d counts %x\n",tower,counts);
            for (int layer=0; layer<8; layer++) 
            {
                nlogs = (counts & 0xf);
                if (nlogs > 0) 
                {
                    const EDR_calColumnMap *colMapP = ctr->colMap;
                    int logMap =  EDR_CAL_TOWER_COLMAP_JUSTIFY (colMapP[layer]);
                    int colMap = logMap;
                    for (int ibit=0; ibit<12; ibit++, colMap <<= 1) 
                    {
                        if (colMap<0) 
                        {
                            int  eA    = energy[0];
                            int  eB    = energy[1];
                            int phaN   = log->phaN;
                            int phaP   = log->phaP;
                            int logNum = layer * EDA_CAL_K_LOGS_PER_LAYER + ibit;
                            int valN   = log_bf->valN;
                            int valP   = log_bf->valP;
                            int rngN   = log_bf->rngN;
                            int rngP   = log_bf->rngP;                  

                            const ECR_calLogEnd* rN = &calTower->ranges[rngN].logs[logNum].n;
                            const ECR_calLogEnd* rP = &calTower->ranges[rngP].logs[logNum].p;

                            int ilayer;
                            if (layer < 4) 
                            {
                                ilayer = layer*2;
                            } else {
                                ilayer = layer*2 - 7;
                            }
                            logData[numLogsHit].tower  = tower;
                            logData[numLogsHit].layer  = ilayer;
                            logData[numLogsHit].column = ibit;
                            logData[numLogsHit].valN   = valN;
                            logData[numLogsHit].rangeN = rngN;
                            logData[numLogsHit].eN     = eB;
                            logData[numLogsHit].pedN   = rN->bf.pedestal;
                            logData[numLogsHit].gainN  = rN->bf.gain;
                            logData[numLogsHit].shiftN = rN->bf.shift;
                            logData[numLogsHit].valP   = valP;
                            logData[numLogsHit].rangeP = rngP;
                            logData[numLogsHit].eP     = eA;
                            logData[numLogsHit].pedP   = rP->bf.pedestal;
                            logData[numLogsHit].gainP  = rP->bf.gain;
                            logData[numLogsHit].shiftP = rP->bf.shift;
                            numLogsHit++;
                            energy  += 2;
                            log  += 1;
                            log_bf  += 1;
                        }
                    }
                }
                counts = counts>>4;
            }
        }
    }

    // Fill in to the TDS output object
    filterStatus->setLogData(numLogsHit, logData);

    return;
}

// This for end of run processing
void CalOutputTool::eorProcessing()
{
    MsgStream log(msgSvc(), name());

    return;
}
