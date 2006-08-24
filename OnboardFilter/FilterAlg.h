/**
 * @class FilterAlg
 * @brief Algorithm that reproduces OnboardFilter logic
 * @author David Wren - dnwren@milkyway.gsfc.nasa.gov
 * $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/FilterAlg.h,v 1.0 2004/04/05
 */

#ifndef _FILTER_ALG_H_
#define _FILTER_ALG_H_

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Property.h"
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

#include "Event/TopLevel/EventModel.h"
#include "Event/MonteCarlo/McParticle.h"
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

#include "Event/Digi/AcdDigi.h"
#include "EDS/ECR_cal.h"

#include <string>

#include "OnboardFilterTds/FilterStatus.h"
#include "OnboardFilterTds/FilterAlgTds.h"

//#include "FilterAlg_acd.h"

extern const struct _TFC_geometry *TFC_Geos[];

//using namespace OnboardFilterTds;

class FilterAlg : public Algorithm{
public:
   FilterAlg(const std::string &name, ISvcLocator *pSvcLocator);
   StatusCode initialize();
   StatusCode execute();
   StatusCode finalize();


   /// access to the Glast Detector Service to read in geometry constants from XML files
   IGlastDetSvc *m_glastDetSvc;

   bool m_throttle;
   int m_triggered_towers, m_number_triggered, m_active_towers;
   int m_trigger_word, m_maskTop, m_maskX, m_maskY;

   int m_CAL_LO;
   int m_CAL_HI;
   static const int m_THROTTLE_SET = 1;

   int m_veto, m_tmsk;
   int m_acd_X, m_acd_Y, m_acd_Front;
   int m_rowMaskX, m_rowMaskY;
   float m_energy;
   int m_tcids, m_coincidences_3[16], m_coincidences_2[16];
   //m_coincidences[] has a list of coincidences for each tower.
   //It is numbered from lsb to msb, with layer 0 at the top.  If
   //a bit is set for a layer, that means a 3-in-a-row starts in
   //that layer (6 out of 6 layers hit)
   const float* m_layerEnergy;
   const int* m_xy00;
   const int* m_xy11;
   const int* m_xy22;
   const int* m_xy33;
   const int* m_xcapture;
   const int* m_ycapture;

   unsigned int m_FilterStatus_HI, m_FilterStatus_LO;

   int m_MaskFront[65];
   int m_MaskY[65];
   int m_MaskX[65];

   unsigned int m_vetoword;

   int m_NOCALLO_FILTER_TILE_VETO;
   int m_SPLASH_0_VETO;
   int m_E0_TILE_VETO;
   int m_E350_FILTER_TILE_VETO;
   int m_SPLASH_1_VETO;
   int m_TOP_VETO;
   int m_SIDE_VETO;
   int m_EL0_ETOT_01_VETO;
   int m_EL0_ETOT_90_VETO;
   int m_ZBOTTOM_VETO;
   int m_TKR_TOP_VETO;
   int m_TKR_ROW01_VETO;
   int m_TKR_ROW23_VETO;
   int m_TKR_EQ_0_VETO;
   int m_TKR_SKIRT_VETO;
   int m_TKR_LT_2_ELO_VETO;

   int m_count;//for debugging purposes

   //for jobOptions
   int m_evaluateCal1;
   int m_usegleamacdvetoes;
   int m_evaluateAcdUncomment1;
   int m_useFilterProjecting;
   int m_useGleamTileGeometry;
   int m_PrjColMatch;


  int num_NOCALLO_FILTER_TILE_VETO;
  int num_SPLASH_0_VETO           ;
  int num_E0_TILE_VETO            ;
  int num_E350_FILTER_TILE_VETO  ;
  int num_SPLASH_1_VETO           ;
  int num_TOP_VETO                ;
  int num_SIDE_VETO               ;
  int num_EL0_ETOT_01_VETO        ;
  int num_EL0_ETOT_90_VETO        ;
  int num_ZBOTTOM_VETO            ;
  int num_TKR_TOP_VETO            ;
  int num_TKR_ROW01_VETO         ;
  int num_TKR_ROW23_VETO          ;
  int num_TKR_EQ_0_VETO           ;
  int num_TKR_SKIRT_VETO          ;
  int num_TKR_LT_2_ELO_VETO        ;
   int num_ANY_VETO;

};

#endif
