#ifndef ONBOARDFILTERTDS_H
#define ONBOARDFILTERTDS_H


/* ---------------------------------------------------------------------- *//*!

   \file  OnboardFilterTDS.h
   \brief Structure of the onboard filter TDS
   \author R. Hughes

\verbatim
    CVS $Id
\endverbatim
                                                                          */
/* ---------------------------------------------------------------------- */

#include "EFC/TFC_projectionDef.h"
#include "OnboardFilterTds/LogInfoDef.h"



#ifdef __cplusplus
extern "C" {
#endif


  struct towerLayer{
    unsigned char cnt;
    TFC_strip *beg;
  };
  struct towerRecord{
    unsigned char lcnt[2];
    int layerMaps[2];
    struct towerLayer layers[36];
  };
  
  
  typedef struct _TDS_Info{
    int tcids;
    int acd_xz;
    int acd_yz;
    int acd_xy;
    int acdStatus[16];
    //TFC_projections prjs[16];
    TFC_projections prjs;
    EDR_tkr         tkr;
//    int logMap[16][8];
    int xCnt[16];
    int yCnt[16];
    int layerEnergy[8];
    int numLogsHit;
    LogInfo logData[16*8*12];    // 16 towers * 8 layers * 12 logs
    int xy00[16];
    int xy11[16];
    int xy22[16];
    int xy33[16];
    int xcapture[16];
    int ycapture[16];
    struct towerRecord hits[16];
    int tmsk;
    int status;
    unsigned int stageEnergy;
  }TDS_Info;
  TDS_Info TDS_variables;

  int TDS_layers[16];


#ifdef __cplusplus
}
#endif


#endif


