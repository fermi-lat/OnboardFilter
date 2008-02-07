#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "CalFilterOutput.h"
#include "OnboardFilterTds/FilterStatus.h"

#include "GaudiKernel/MsgStream.h"

#include "EFC/EFC_edsFw.h"
#include "EDS/EBF_dir.h"
#include "EDS/ECR_cal.h"
#include "EDS/EDR_cal.h"
#include "EDS/EBF_cal.h"
#include "EDS/EDR_calUnpack.h"
#include "EDS/FFS.h"

// Constructor
CalFilterOutput::CalFilterOutput()
{
    return;
}
/* ---------------------------------------------------------------------- *//*!

  \fn  void extractFilterInfo(Stream *ostream, EDS_fwIxb *ixb)
                            
  \brief         Extracts info from the onboard filter algorithm. 
  \param ostream dummy var
  \param     ixb The information exchange block.

   This routine extracts both the results and the some of the intermediate
   info used by the filter in deciding if an event should be vetoed or not.
                                                                          */
/* ---------------------------------------------------------------------- */
void CalFilterOutput::eovProcessing(void* callBackParm, EDS_fwIxb* ixb)
{
    // Extract output pointers info
    ObfOutputCallBackParm* tdsPointers = reinterpret_cast<ObfOutputCallBackParm*>(callBackParm);

    // Recover pointers to the TDS objects
    OnboardFilterTds::FilterStatus* filterStatus = tdsPointers->m_filterStatus;

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
        if ((twrMap    & FFS_mask (tower)) > 0) 
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

void CalFilterOutput::eorProcessing(MsgStream& log)
{
    return;
}
