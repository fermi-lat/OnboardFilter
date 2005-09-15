

#define EFC_DFILTER

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "EFC/EDM.h"
//#include "EFC_ss.h"
#include "filter_rto.h"
//#include "filter_rto.h"
#include "EFC/EFC_edsFw.h"

#include "EFC/EFC_gammaResultsPrint.h"
#include "EFC/EFC_gammaStatsPrint.h"
#include "EFC/EFC_gammaCfgPrint.h"
#include "EFC/EFC_gammaCfg.h"
#include "EFC/EFC_gamma.h"



#include "EFC/EFC_gammaStatus.h"
#include "EFC/EFC_gammaResult.h"
#include "EFC/EFC_gammaStats.h"
#include "EFC/TFC_projectionDef.h"


#if       defined (EFC_DFILTER)
#include "EFC/EFC_display.h"
#include "EFC/TFC_geometryPrint.h"
#include "EFC/TFC_projectionPrint.h"
#endif

#include "EFC/TFC_geos.h"
#include "EFC/TFC_geoIds.h"
#include "EFC_gammaResultDef.h"


#include "EDS/LCBV.h"
#include "EDS/EBF_dir.h"
#include "EDS/EBF_evt.h"
#include "EDS/EBF_pkt.h"
#include "EDS/EBF_mc.h"
#include "EDS/TMR.h"
#include "EDS/io/LCBP.h"
#include "EDS/io/EBF_stream.h"
#include "EDS/io/EBF_evts.h"
#include "EDS/EDS_fw.h"
#include "EDS/ECR_cal.h"
#include "EDS/EDR_cal.h"
#include "EDS/EDR_tkr.h"


#include "EDS/FFS.h"
#include "EDS/EBF_cid.h"
#include "EDS/EBF_ctb.h"
#include "EDS/EBF_gem.h"
#include "EDS/EBF_tkr.h"
#include "EDS/EBF_dir.h"
#include "EDS/EBF_evt.h"
#include "EDS/EDR_calUnpack.h"
#include "EDS/EDR_tkrUnpack.h"
#include "EDS/EDR_gemPrint.h"
#include "EDS/EDR_calPrint.h"
#include "EDS/EDR_tkrPrint.h"

 
#include "GaudiKernel/Algorithm.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Property.h"
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

#include "Event/TopLevel/EventModel.h"
#include "Event/MonteCarlo/McParticle.h"
#include "EbfWriter/Ebf.h"
#include "FilterAlg_skirt.h"
#include "TFC_acd.h"
#include "EDS/EDR_tkr.h"
#include "TFC_geometryDef.h"
#include "EFC/TFC_projectionDef.h"
#include "FilterAlg_projectionFind.h"

#include "OnboardFilter/FilterStatus.h"
#include "OnboardFilter/FilterAlg.h" 


/* ---------------------------------------------------------------------- *//*!

  \def    OFFSET_EVEN
  \brief  An additional offset (from the right) in a 32-bit word from
          where the set of even layers normally begin.

          In order to properly form the coincidences, a buffer area
          between the bits representing the even and odd layers must
          be maintained. This defines the beginning of the set of bits
          representing the even layers. There is also an offset defined
          for the set of bits representing the odd bits. Only one of
          these is non-zero, but the code is written in such a way that
          the non-zero one can be either.
                                                                          *//*!
  \def    OFFSET_ODD
  \brief  An additional offset (from the right) in a 32-bit word from
          where the set of odd layers begin.

          In order to properly form the coincidences, a buffer area
          between the bits representing the even and odd layers must
          be maintained. This defines the beginning of the set of bits
          representing the idd layers. There is also an offset defined
          for the set of bits representing the even bits. Only one of
          these is non-zero, but the code is written in such a way that
          the non-zero one can be either.
                                                                          */
/* ---------------------------------------------------------------------- */
#define OFFSET_EVEN 0
#define OFFSET_ODD  7    
        
#define TFC_TRG_REMAP_INIT(_trigger, _even, _odd, _e, _o)                  \
_even  =  ((((_trigger)>>(EBF_TKR_RIGHT_BIT_K_L ## _e + OFFSET_EVEN)) & 1) \
      << (EBF_TKR_K_L ## _e));                                             \
_odd   =  ((((_trigger)>>(EBF_TKR_RIGHT_BIT_K_L ## _o + OFFSET_ODD )) & 1) \
      << (EBF_TKR_K_L ## _o))

    
#define TFC_TRG_REMAP(_trigger, _even, _odd, _e, _o)                       \
_even |=  ((((_trigger)>>(EBF_TKR_RIGHT_BIT_K_L ## _e + OFFSET_EVEN)) & 1) \
      << (EBF_TKR_K_L ## _e));                                             \
_odd  |=  ((((_trigger)>>(EBF_TKR_RIGHT_BIT_K_L ## _o + OFFSET_ODD )) & 1) \
      << (EBF_TKR_K_L ## _o))
    


/* ---------------------------------------------------------------------- *//*!

  \enum   _ACD_SIDE_TILES_M
  \brief   Enumerates the bit masks used to extract the struck tiles in
           various rows for both the + and - ACD side tile planes
									  *//*!
  \typedef ACD_SIDE_TILES_M
  \brief   Typedef for enum _ACD_SIDE_TILES_M
									  */
/* ---------------------------------------------------------------------- */
typedef enum _ACD_SIDE_TILES_M
{
  ACD_SIDE_TILES_M_ROW  = (0x1f << 16) | (0x1f), 
  /*!< Primitive bit mask for 1 5 tile row of ACD side tiles              */

  ACD_SIDE_TILES_M_ROW0 = ACD_SIDE_TILES_M_ROW<<(0*5),
  /*!< Row 0 ACD side tiles, bit mask                                     */ 

  ACD_SIDE_TILES_M_ROW1 = ACD_SIDE_TILES_M_ROW<<(1*5),
  /*!< Row 1 ACD side tiles, bit mask                                     */ 

  ACD_SIDE_TILES_M_ROW2 = ACD_SIDE_TILES_M_ROW<<(2*5),
  /*!< Row 2 ACD side tiles, bit mask                                     */ 

  ACD_SIDE_TILES_M_ROW01 = ACD_SIDE_TILES_M_ROW0 | ACD_SIDE_TILES_M_ROW1,
  /*!< Rows 0 and 1 ACD side tiles, bit mask                              */ 

  ACD_SIDE_TILES_M_ROW3  = 0x80008000,
  /*!< Row  3 ACD side tiles, bit mask                                    */
}
ACD_SIDE_TILES_M; 
/* ---------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \var   const TFC_geometry *TFC_Geos[]
  \brief A list of the available geometries.

   This is used as a cheap database of the known geometries.
                                                                         */
/* --------------------------------------------------------------------- */
extern const TFC_geometry *TFC_Geos[];
/* --------------------------------------------------------------------- */




static const AlgFactory<FilterAlg> Factory;
const IAlgFactory& FilterAlgFactory = Factory;  
GammaCfgTkr          cfg;
  
FilterAlg::FilterAlg(const std::string& name, ISvcLocator* pSvcLocator):Algorithm(name, pSvcLocator){
  
  declareProperty("UseGleamAcdVetoes",     m_usegleamacdvetoes     = 0);//Set to 1 to use Gleam veto lists.
  declareProperty("evaluateCal1",          m_evaluateCal1          = 0);//Set to 1 to call this function.
  declareProperty("evaluateAcdUncomment1", m_evaluateAcdUncomment1 = 0);//Set to 1 to uncomment code
  declareProperty("UseFilterProjecting",   m_useFilterProjecting   = 1);//Set to zero to use my method.
                                                                        //Leave at 1 to use JJ's.
  declareProperty("UseGleamTileGeometry",  m_useGleamTileGeometry  = 0);//Set to 1 to use the Gleam geometry.
                                                                        //Note that this geometry does not account
                                                                        //for gaps between tiles.
  declareProperty("DoPrjColumnMatchCheck", m_PrjColMatch           = 0);//Set to 1 to use more complicated
                                                                        //projection method that checks
                                                                        //to see if the column is hit also.

  if (m_PrjColMatch){
    m_useGleamTileGeometry = 1;//force it to use the Gleam Tile Geometry, not JJ's
    m_useFilterProjecting = 0; //force it to use my projection method, not JJ's
  }
}


StatusCode FilterAlg::initialize(){
  MsgStream log(msgSvc(),name());
  log<<MSG::DEBUG<<"Initializing"<<endreq;
  
  StatusCode sc = StatusCode::SUCCESS;
  
  




    /* Find the specified geometry */
   TFC_geoId      geo_id;
    const TFC_geometry     *geo;
    int geoPrint = 0;
   geo_id   = TFC_K_GEO_ID_DEFAULT;
    geo = locateGeo (geo_id, geoPrint);
  
  m_glastDetSvc = 0;
  sc = service("GlastDetSvc", m_glastDetSvc, true);
  if (sc.isSuccess() ) {
    sc = m_glastDetSvc->queryInterface(IID_IGlastDetSvc, (void**)&m_glastDetSvc);
  }
  
  if( sc.isFailure() ) {
    log << MSG::ERROR << "FilterAlg failed to get the GlastDetSvc" << endreq;
    return sc;
  }
  
  m_number_triggered = 0;
  m_triggered_towers = 0;
  m_throttle = false;
  initMasks(m_MaskFront,m_MaskY,m_MaskX);
  
//  m_CAL_LO = 0x400;
//  m_CAL_HI = 0x800;
  m_CAL_LO = 1 << EFC_GAMMA_STATUS_V_GEM_CALLO;
  m_CAL_HI = 1 << EFC_GAMMA_STATUS_V_GEM_CALHI;
  
  m_NOCALLO_FILTER_TILE_VETO = 30;
  m_SPLASH_0_VETO            = 29;
  m_E0_TILE_VETO             = 28;
  m_E350_FILTER_TILE_VETO    = 27;
  m_SPLASH_1_VETO            = 26;
  m_TOP_VETO                 = 25;
  m_SIDE_VETO                = 24;
  m_EL0_ETOT_01_VETO         = 23;
  m_EL0_ETOT_90_VETO         = 22;
  m_ZBOTTOM_VETO             = 21;
  m_TKR_TOP_VETO             = 20;
  m_TKR_ROW01_VETO           = 19;
  m_TKR_ROW23_VETO           = 18;
  m_TKR_EQ_0_VETO            = 17;
  m_TKR_SKIRT_VETO           = 16;
  m_TKR_LT_2_ELO_VETO        = 15;
  
  m_count = 0;//for debug purposes
  num_NOCALLO_FILTER_TILE_VETO= 0;
  num_SPLASH_0_VETO= 0           ;
  num_E0_TILE_VETO= 0            ;
  num_E350_FILTER_TILE_VETO= 0  ;
  num_SPLASH_1_VETO= 0           ;
  num_TOP_VETO= 0                ;
  num_SIDE_VETO= 0               ;
  num_EL0_ETOT_01_VETO= 0        ;
  num_EL0_ETOT_90_VETO= 0        ;
  num_ZBOTTOM_VETO= 0            ;
  num_TKR_TOP_VETO= 0            ;
  num_TKR_ROW01_VETO= 0         ;
  num_TKR_ROW23_VETO= 0          ;
  num_TKR_EQ_0_VETO= 0           ;
  num_TKR_SKIRT_VETO= 0          ;
  num_TKR_LT_2_ELO_VETO = 0       ;
  num_ANY_VETO = 0;


    cfg.row2Emax           = ECR_CAL_MEV_TO_LEU (30000);
    cfg.row01Emax          = ECR_CAL_MEV_TO_LEU (10000);
    cfg.topEmax            = ECR_CAL_MEV_TO_LEU (30000);
    cfg.zeroTkrEmin        = ECR_CAL_MEV_TO_LEU (250);
    cfg.twoTkrEmax         = ECR_CAL_MEV_TO_LEU ( 5);
    cfg.skirtEmax          = ECR_CAL_MEV_TO_LEU (20);
    cfg.geometry           = geo;

  return StatusCode::SUCCESS;
}

StatusCode FilterAlg::finalize(){
  MsgStream log(msgSvc(),name());
  log<<MSG::DEBUG<<"Finalizing"<<endreq;
  printf("NOCALLO_FILTER_TILE_VETO: %d\n",num_NOCALLO_FILTER_TILE_VETO);
  printf("SPLASH_0_VETO: %d\n",num_SPLASH_0_VETO);
  printf("E0_TILE_VETO: %d\n",num_E0_TILE_VETO);
  printf("E350_FILTER_TILE_VETO: %d\n",num_E350_FILTER_TILE_VETO);
  printf("SPLASH_1_VETO: %d\n",num_SPLASH_1_VETO);
  printf("TOP_VETO: %d\n",num_TOP_VETO);
  printf("SIDE_VETO: %d\n",num_SIDE_VETO);
  printf("EL0_ETOT_01_VETO: %d\n",num_EL0_ETOT_01_VETO);
  printf("EL0_ETOT_90_VETO: %d\n",num_EL0_ETOT_90_VETO);
  printf("ZBOTTOM_VETO: %d\n",num_ZBOTTOM_VETO);
  printf("TKR_TOP_VETO: %d\n",num_TKR_TOP_VETO);
  printf("TKR_ROW01_VETO: %d\n",num_TKR_ROW01_VETO);
  printf("TKR_ROW23_VETO: %d\n",num_TKR_ROW23_VETO);
  printf("TKR_EQ_0_VETO: %d\n",num_TKR_EQ_0_VETO);
  printf("TKR_SKIRT_VETO: %d\n",num_TKR_SKIRT_VETO);
  printf("TKR_LT_2_ELO_VETO: %d\n",num_TKR_LT_2_ELO_VETO);
  printf("ANY_VETO: %d\n",num_ANY_VETO);
  return StatusCode::SUCCESS;
}

StatusCode FilterAlg::execute(){
//  printf("\n\n\n******************************Calling FilterAlg\n");
//  if (m_count < 1) return StatusCode::SUCCESS;
  MsgStream log(msgSvc(),name());
 // log<<MSG::DEBUG<<"execute()"<<endreq;
  ITkrGeometrySvc *tkrGeoSvc=NULL;
  if(service("TkrGeometrySvc",tkrGeoSvc,true).isFailure()){
    log<<MSG::ERROR<<"Couldn't set up TkrGeometrySvc!"<<endreq;
    return StatusCode::FAILURE;
  }
  
  //Create TDS object for FilterAlg
  FilterAlgTds::FilterAlgData *newFilterAlgData=new FilterAlgTds::FilterAlgData;
  eventSvc()->registerObject("/Event/Filter/FilterAlgData", newFilterAlgData);
  
  //Get filter info from TDS
  SmartDataPtr<OnboardFilterTds::FilterStatus> filter(eventSvc(),
                                                      "/Event/Filter/FilterStatus");
  if(!filter){
//    log<<MSG::ERROR<<"Unable to retrieve FilterStatus from TDS"<<endreq;
    return StatusCode::SUCCESS;
  }
  
  //fill in the variables we need
  m_FilterStatus_HI = filter->getHigh();
  m_FilterStatus_LO = filter->getLow();
  filter->getAcdMap(m_acd_Y, m_acd_X, m_acd_Front);//this order is important: Y,X,Top
  m_energy = filter->getCalEnergy();
  m_layerEnergy = filter->getLayerEnergy();
  m_tcids = filter->getTcids();
  const int *layers = filter->getLayers();
  m_tmsk = filter->getTmsk();
  m_xy00 = filter->getXY00();
  m_xy11 = filter->getXY11();
  m_xy22 = filter->getXY22();
  m_xy33 = filter->getXY33();
  m_xcapture = filter->getXcapture();
  m_ycapture = filter->getYcapture();
  
  

  unsigned int tmsk = createTowerMask(m_tcids);
  do{
      
      int tower;
      tower = FFS (tmsk) - 16;
      tmsk &= ~(0x80000000 >> (tower+16));
  }
  while (tmsk);

  //reset veto word
  m_vetoword = 0;
  
  //do some manipulations of filter data to get some
  //necessary local variables.
  CreateLocals();
  //if the user wants to use a different set of acd tile hits,
  //such as the acdDigi hits (as opposed to what OnboardFilter
  //says are hit), the user can insert that code in useAcdDigi(acd), but
  //must make sure to format m_acd_Y, m_acd_X, and m_acd_Front
  //properly.  If the user wants to use the Gleam ACD Digis, just
  //add this to the jobOptions file: FilterAlg.UseGleamAcdVetoes = 1;
  
  if (m_usegleamacdvetoes){
    SmartDataPtr<Event::AcdDigiCol> acd(eventSvc(), EventModel::Digi::AcdDigiCol);
    if( acd==0 ) log << MSG::DEBUG << "No acd digis found" << endreq;
    useAcdDigi(acd);//Important!  The threshold is different for these.  Do not
    //expect the tile list to be the same!
  }
  
  //start filtering
  CheckCal();
  evaluateAcd();
  evaluateAtf();
  evaluateZbottom();
  if (m_evaluateCal1) evaluateCal1();
  TFC_projections *m_prjs = filter->getProjections();
  EDR_tkr *m_tkr = filter->getTkr();
  unsigned int vetoes = 0;
  unsigned int status = tkrFilter(&cfg, 4.0*m_energy, m_acd_X, m_acd_Y, m_acd_Front, 
      vetoes, m_tkr, m_prjs);

//  if ((m_tcids == 0 )&&(4.0*m_energy > 250)) setVeto(m_TKR_EQ_0_VETO);
  if ((m_tcids == 0 )) setVeto(m_TKR_EQ_0_VETO);


//
// Check the status bits
   if (isVetoed(status,EFC_GAMMA_STATUS_M_TKR_LT_2_ELO)) setVeto(m_TKR_LT_2_ELO_VETO);
   if (isVetoed(status,EFC_GAMMA_STATUS_M_TKR_SKIRT)) setVeto(m_TKR_SKIRT_VETO);
   if (isVetoed(status,EFC_GAMMA_STATUS_M_TKR_EQ_0)) setVeto(m_TKR_EQ_0_VETO);
   if (isVetoed(status,EFC_GAMMA_STATUS_M_TKR_ROW2)) setVeto(m_TKR_ROW23_VETO);
   if (isVetoed(status,EFC_GAMMA_STATUS_M_TKR_ROW01)) setVeto(m_TKR_ROW01_VETO);
   if (isVetoed(status,EFC_GAMMA_STATUS_M_TKR_TOP)) setVeto(m_TKR_TOP_VETO);

  m_TKR_TOP_VETO             = 20;
  m_TKR_ROW01_VETO           = 19;
  m_TKR_ROW23_VETO           = 18;
  m_TKR_EQ_0_VETO            = 17;
  m_TKR_SKIRT_VETO           = 16;
  m_TKR_LT_2_ELO_VETO        = 15;

  
  //All Vetoes have been filled in.
  m_count++;//for debugging purposes
  //put the vetoword in the tds
  newFilterAlgData->setVetoWord(m_vetoword);

  if ((m_vetoword >> (m_NOCALLO_FILTER_TILE_VETO-15)) & 1) {
   num_NOCALLO_FILTER_TILE_VETO++;
   //printf("AlgFilterV30: NOCALLO_FILTER_TILE_VETO\n");
  }  
  if ((m_vetoword >> (m_SPLASH_0_VETO-15)) & 1)            {
   num_SPLASH_0_VETO++           ;
   //printf("AlgFilterV29: SPLASH_0_VETO\n");
  }  
  if ((m_vetoword >> (m_E0_TILE_VETO-15)) & 1)             {
   num_E0_TILE_VETO++            ;
   //printf("AlgFilterV28: E0_TILE_VETO\n");
  }  
  if ((m_vetoword >> (m_E350_FILTER_TILE_VETO-15)) & 1)    {
   num_E350_FILTER_TILE_VETO++  ;
   //printf("AlgFilterV27: E350_FILTER_TILE_VETO\n");
  }  
  if ((m_vetoword >> (m_SPLASH_1_VETO-15)) & 1)            {
   num_SPLASH_1_VETO++           ;
   //printf("AlgFilterV26: SPLASH_1_VETO\n");
  }  
  if ((m_vetoword >> (m_TOP_VETO-15)) & 1)                 {
   num_TOP_VETO++                ;
   //printf("AlgFilterV25: TOP_VETO\n");
  }  
  if ((m_vetoword >> (m_SIDE_VETO-15)) & 1)                {
   num_SIDE_VETO++               ;
   //printf("AlgFilterV24: SIDE_VETO\n");
  }  
  if ((m_vetoword >> (m_EL0_ETOT_01_VETO-15)) & 1)         {
   num_EL0_ETOT_01_VETO++        ;
   //printf("AlgFilterV23: EL0_ETOT_01_VETO\n");
  }  
  if ((m_vetoword >> (m_EL0_ETOT_90_VETO-15)) & 1)         {
   num_EL0_ETOT_90_VETO++        ;
   //printf("AlgFilterV22: EL0_ETOT_90_VETO\n");
  }  
  if ((m_vetoword >> (m_ZBOTTOM_VETO-15)) & 1)             {
   num_ZBOTTOM_VETO++            ;
   //printf("AlgFilterV21: ZBOTTOM_VETO\n");
  }  
  if ((m_vetoword >> (m_TKR_TOP_VETO-15)) & 1)             {
   num_TKR_TOP_VETO++            ;
   //printf("AlgFilterV20: TKR_TOP_VETO\n");
  }  
  if ((m_vetoword >> (m_TKR_ROW01_VETO-15)) & 1)           {
   num_TKR_ROW01_VETO++         ;
   //printf("AlgFilterV19: TKR_ROW01_VETO\n");
  }  
  if ((m_vetoword >> (m_TKR_ROW23_VETO-15)) & 1)           {
   num_TKR_ROW23_VETO++          ;
   //printf("AlgFilterV18: TKR_ROW23_VETO\n");
  }  
  if ((m_vetoword >> (m_TKR_EQ_0_VETO-15)) & 1)            {
   num_TKR_EQ_0_VETO++           ;
   //printf("AlgFilterV17: TKR_EQ_0_VETO\n");
  }  
  if ((m_vetoword >> (m_TKR_SKIRT_VETO-15)) & 1)           {
   num_TKR_SKIRT_VETO++          ;
   //printf("AlgFilterV16: TKR_SKIRT_VETO\n");
  }  
  if ((m_vetoword >> (m_TKR_LT_2_ELO_VETO-15)) & 1)        {
   num_TKR_LT_2_ELO_VETO++       ;
   //printf("AlgFilterV15: TKR_LT_2_ELO_VETO\n");
  }  
  if (m_vetoword > 0) {
   num_ANY_VETO++;
   //printf("AlgFilterV: ANY_VETO\n");
  } else {
   //printf("AlgFilterV: NOT vetoed!\n");
  }
  
  return StatusCode::SUCCESS;
  
}

void FilterAlg::CreateLocals(){
  //NOTE: tcids is ordered from left to right: 0 1 ....15
  //so we have mask off the 3-in-a-rows and reverse the bits!
  int bit = 0;
  int temp = 0;
  m_triggered_towers = (m_tcids & 0xffff0000);
  while (m_triggered_towers){
    bit = FFS(m_triggered_towers);
    temp |= (int)pow(2.0,bit);
    m_triggered_towers ^= (int)pow(2.0,31-bit);
  }
  m_triggered_towers = temp;
  
  //Here we create a list of coincidences of layers hit for each tower
  //that has a 3-in-a-row
  int xy1[16], xy2[16], even, odd, tower;
  for (tower=0; tower<16; tower++) {
    m_coincidences_3[tower] = 0;
    m_coincidences_2[tower] = 0;
    xy1[tower] = 0;
    xy2[tower] = 0;
  }
  for (tower=0; tower<16; tower++){
    xy1[tower] |= m_xy00[tower] & m_xy11[tower] & m_xy22[tower];
    xy2[tower] |= m_xy00[tower] & m_xy11[tower] & m_xy33[tower];
    for (int i=0; i<9; i++){
      even = xy1[tower] & (int)pow(2.0,i+1);
      odd  = xy2[tower] & (int)pow(2.0,i);
      if (even){
        m_coincidences_3[tower] |= (int)pow(2.0, (16-2*(i)));
      }
      if (odd){
        m_coincidences_3[tower] |= (int)pow(2.0,(17-2*i));
      }
    }
  }
  //Here we create a list of coincidences of layers hit for each tower
  //that has a 2-in-a-row
  for (tower=0; tower<16; tower++){
    xy1[tower] = 0;
    xy2[tower] = 0;
  }
  for (tower=0; tower<16; tower++){
    xy1[tower] |= m_xy00[tower] & m_xy11[tower];
    xy2[tower] |= m_xy11[tower] & m_xy22[tower];
    for (int i=0; i<9; i++){
      even = xy2[tower] & (int)pow(2.0,i+1);
      odd  = xy1[tower] & (int)pow(2.0,i);
      if (even){
        m_coincidences_2[tower] |= (int)pow(2.0, (16-2*(i)));
      }
      if (odd){
        m_coincidences_2[tower] |= (int)pow(2.0,(17-2*i));
      }
    }
  }
  return;
}

void FilterAlg::initMasks(int m_MaskFront[], int m_MaskY[], int m_MaskX[]){
  //These masks are used to determine whether the splash veto condition
  //was met.
  //Front of ACD
  m_MaskFront[0]= 0x62;      m_MaskY[0]= 0x3;       m_MaskX[0]=  0x3;
  m_MaskFront[1]= 0xE5;      m_MaskY[1]= 0x7;       m_MaskX[1]=  0;
  m_MaskFront[2]= 0x1CA;     m_MaskY[2]= 0xE;       m_MaskX[2]=  0;
  m_MaskFront[3]= 0x394;     m_MaskY[3]= 0x1C;      m_MaskX[3]=  0;
  //m_MaskFront[4]= 0x184;     m_MaskY[4]= 0x18;      m_MaskX[4]=  0x30000;
  m_MaskFront[4]= 0x308;     m_MaskY[4]= 0x18;      m_MaskX[4]=  0x30000;
  m_MaskFront[5]= 0xC43;     m_MaskY[5]= 0;         m_MaskX[5]=  0x7;
  m_MaskFront[6]= 0x1CA7;    m_MaskY[6]= 0;         m_MaskX[6]=  0;
  m_MaskFront[7]= 0x394E;    m_MaskY[7]= 0;         m_MaskX[7]=  0;
  m_MaskFront[8]= 0x729C;    m_MaskY[8]= 0;         m_MaskX[8]=  0;
  m_MaskFront[9]= 0x6118;    m_MaskY[9]= 0;         m_MaskX[9]=  0x70000;
  m_MaskFront[10]=0x18860;   m_MaskY[10]=0;         m_MaskX[10]= 0xE;
  m_MaskFront[11]=0x394E0;   m_MaskY[11]=0;         m_MaskX[11]= 0;
  m_MaskFront[12]=0x729C0;   m_MaskY[12]=0;         m_MaskX[12]= 0;
  m_MaskFront[13]=0xE5380;   m_MaskY[13]=0;         m_MaskX[13]= 0;
  m_MaskFront[14]=0xC2300;   m_MaskY[14]=0;         m_MaskX[14]= 0xE0000;
  m_MaskFront[15]=0x310C00;  m_MaskY[15]=0;         m_MaskX[15]= 0x1C;
  m_MaskFront[16]=0x729C00;  m_MaskY[16]=0;         m_MaskX[16]= 0;
  m_MaskFront[17]=0xE53800;  m_MaskY[17]=0;         m_MaskX[17]= 0;
  m_MaskFront[18]=0x1CA7000; m_MaskY[18]=0;         m_MaskX[18]= 0;
  m_MaskFront[19]=0x1846000; m_MaskY[19]=0;         m_MaskX[19]= 0x1C0000;
  m_MaskFront[20]=0x218000;  m_MaskY[20]=0x30000;   m_MaskX[20]= 0x18;
  m_MaskFront[21]=0x538000;  m_MaskY[21]=0x70000;   m_MaskX[21]= 0;
  m_MaskFront[22]=0xA70000;  m_MaskY[22]=0xE0000;   m_MaskX[22]= 0;
  m_MaskFront[23]=0x14E0000; m_MaskY[23]=0x1C0000;  m_MaskX[23]= 0;
  m_MaskFront[24]=0x8C0000;  m_MaskY[24]=0x180000;  m_MaskX[24]= 0x180000;
  //Y minus side
  m_MaskFront[25]=0x3;       m_MaskY[25]=0x62;      m_MaskX[25]=0x21;
  m_MaskFront[26]=0x7;       m_MaskY[26]=0xE5;      m_MaskX[26]=0;
  m_MaskFront[27]=0xE;       m_MaskY[27]=0x1CA;     m_MaskX[27]=0;
  m_MaskFront[28]=0x1C;      m_MaskY[28]=0x394;     m_MaskX[28]=0;
  m_MaskFront[29]=0x18;      m_MaskY[29]=0x308;     m_MaskX[29]=0x210000;
  m_MaskFront[30]=0;         m_MaskY[30]=0x43;      m_MaskX[30]=0x21;
  m_MaskFront[31]=0;         m_MaskY[31]=0xA7;      m_MaskX[31]=0;
  m_MaskFront[32]=0;         m_MaskY[32]=0x14E;     m_MaskX[32]=0;
  m_MaskFront[33]=0;         m_MaskY[33]=0x29C;     m_MaskX[33]=0;
  m_MaskFront[34]=0;         m_MaskY[34]=0x118;     m_MaskX[34]=0x210000;
  //Y plus side
  m_MaskFront[35]=0x300000;  m_MaskY[35]=0x620000;  m_MaskX[35]=0x210;
  m_MaskFront[36]=0x700000;  m_MaskY[36]=0xE50000;  m_MaskX[36]=0;
  m_MaskFront[37]=0xE00000;  m_MaskY[37]=0x1CA0000; m_MaskX[37]=0;
  m_MaskFront[38]=0x1C00000; m_MaskY[38]=0x3940000; m_MaskX[38]=0;
  m_MaskFront[39]=0x1800000; m_MaskY[39]=0x3080000; m_MaskX[39]=0x2100000;
  m_MaskFront[40]=0;         m_MaskY[40]=0x430000;  m_MaskX[40]=0x210;
  m_MaskFront[41]=0;         m_MaskY[41]=0xA70000;  m_MaskX[41]=0;
  m_MaskFront[42]=0;         m_MaskY[42]=0x14E0000; m_MaskX[42]=0;
  m_MaskFront[43]=0;         m_MaskY[43]=0x29C0000; m_MaskX[43]=0;
  m_MaskFront[44]=0;         m_MaskY[44]=0x1180000; m_MaskX[44]=0x2100000;
  //X minus side
  m_MaskFront[45]=0x21;      m_MaskY[45]=0x21;      m_MaskX[45]=0x62;
  m_MaskFront[46]=0x421;     m_MaskY[46]=0;         m_MaskX[46]=0xE5;
  m_MaskFront[47]=0x8420;    m_MaskY[47]=0;         m_MaskX[47]=0x1CA;
  m_MaskFront[48]=0x108400;  m_MaskY[48]=0;         m_MaskX[48]=0x394;
  m_MaskFront[49]=0x108000;  m_MaskY[49]=0x210000;  m_MaskX[49]=0x308;
  m_MaskFront[50]=0;         m_MaskY[50]=0x21;      m_MaskX[50]=0x43;
  m_MaskFront[51]=0;         m_MaskY[51]=0;         m_MaskX[51]=0xA7;
  m_MaskFront[52]=0;         m_MaskY[52]=0;         m_MaskX[52]=0x14E;
  m_MaskFront[53]=0;         m_MaskY[53]=0;         m_MaskX[53]=0x29C;
  m_MaskFront[54]=0;         m_MaskY[54]=0x210000;  m_MaskX[54]=0x118;
  //X plus side
  m_MaskFront[55]=0x210;     m_MaskY[55]=0x210;     m_MaskX[55]=0x620000;
  m_MaskFront[56]=0x4210;    m_MaskY[56]=0;         m_MaskX[56]=0xE50000;
  m_MaskFront[57]=0x84200;   m_MaskY[57]=0;         m_MaskX[57]=0x1CA0000;
  m_MaskFront[58]=0x1084000; m_MaskY[58]=0;         m_MaskX[58]=0x3940000;
  m_MaskFront[59]=0x1080000; m_MaskY[59]=0x2100000; m_MaskX[59]=0x3080000;
  m_MaskFront[60]=0;         m_MaskY[60]=0x210;     m_MaskX[60]=0x430000;
  m_MaskFront[61]=0;         m_MaskY[61]=0;         m_MaskX[61]=0xA70000;
  m_MaskFront[62]=0;         m_MaskY[62]=0;         m_MaskX[62]=0x14E0000;
  m_MaskFront[63]=0;         m_MaskY[63]=0;         m_MaskX[63]=0x29C0000;
  m_MaskFront[64]=0;         m_MaskY[64]=0x2100000; m_MaskX[64]=0x1180000;
  return;
}

void FilterAlg::useAcdDigi(const Event::AcdDigiCol& digiCol){
  // Code in AcdReconAlg shows how to get the id of a tile that
  // is hit.  So get the id, and just set it in one of 3 lists.
  // Two lists will be for all the side tiles (16x4), and
  // one will be for the top tiles (25 of them).
  StatusCode temp_sc;
  double threshold;
  temp_sc = m_glastDetSvc->getNumericConstByName("acd.vetoThreshold", &threshold);
  
  //initialize the acd tile lists
  m_acd_Front = 0;
  m_acd_X = 0;
  m_acd_Y = 0;
  
  short s_face;
  short s_tile;
  short s_row;
  short s_column;
  
  Event::AcdDigiCol::const_iterator acdDigiIt;
  
  //loop over all tiles
  for (acdDigiIt = digiCol.begin(); acdDigiIt != digiCol.end(); ++acdDigiIt) {
    
    idents::AcdId id = (*acdDigiIt)->getId();
    
    // if it is a tile...
    if (id.tile()==true) {
      // toss out hits below threshold
      if ((*acdDigiIt)->getEnergy() < threshold) continue;
      //get the face, row, and column numbers
      s_face = id.face();
      s_row  = id.row();
      s_column = id.column();
      //set the tile number (depending on the face)
      if (s_face == 0)
        s_tile = s_column + s_row*5;
      else if ( (s_face >0) && (s_face < 5)){
        if (s_row < 3) s_tile = s_row*5 + s_column;
        else           s_tile = 15;
      }
      //set the appropriate bit for the tile number
      switch (s_face) {
      case 0: //top
        m_acd_Front |= (int)pow(2,s_tile);
        break;
      case 1: //-X
        m_acd_X |= (int)pow(2,s_tile);
        break;
      case 2: //-Y
        m_acd_Y |= (int)pow(2,s_tile);
        break;
      case 3: //+X
        m_acd_X |= (int)pow(2,s_tile+16);
        break;
      case 4: //+Y
        m_acd_Y |= (int)pow(2,s_tile+16);
        break;
      }//switch
    }//if (id.tile())
  }//for loop over tiles
  return;
}

void FilterAlg::CheckCal(){
  
  if (!(m_FilterStatus_LO & m_CAL_LO)){
    if ( acdFilter() ){
      setVeto(m_NOCALLO_FILTER_TILE_VETO);
    }
  }
  
  if (!(m_FilterStatus_LO & m_CAL_HI)){
    int numbits = sumBits(m_acd_X, m_acd_Y, m_acd_Front);
    if ( numbits >= 4){
      setVeto(m_SPLASH_0_VETO);
    }
//    else if (numbits == 3){
    else {
      if ( AFC_splash() ){
        setVeto(m_SPLASH_0_VETO);
      }
    }
  }
  return;
}

bool FilterAlg::AFC_splash(){
  
  unsigned int acdX = m_acd_X;
  unsigned int acdY = m_acd_Y;
  unsigned int acdFront = m_acd_Front;
  int bitnum = 0;
  int masknum = 0;
  
  //Loop over each hit tile
  while (acdFront){
    //get the bit position (FFS counts backwards)
    bitnum = 31 - FFS(acdFront);
    //map the bit to the mask numbering system:
    //for acdFront, the bit number is the mask number!
    masknum = bitnum;
    //Compare the masks to the hit tiles:
    //Check the hit tiles in each face against the
    //mask of bits.  There are 3 tiles total, but one is
    //the tile in question, so it doesn't count.  We
    //look at the total number of tiles in the mask region
    //(masking off the tile in the middle).  If there are
    //no tiles in the mask region (not counting the tile in
    //the middle, they must be outside the mask region,
    //and that counts as a splash!
    if ( (sumBits( (m_acd_Front & m_MaskFront[masknum]),
                   (m_acd_Y & m_MaskY[masknum]),
                   (m_acd_X & m_MaskX[masknum]) )
          <= 1) &&
         (sumBits( (m_acd_Front & 0x1ffffff),
                   (m_acd_Y & 0x3ff03ff),
                   (m_acd_X & 0x3ff03ff) )
          == 3) ) return true;//was: == 0
    //Eliminate that bit from acdFront so we don't look
    //at it twice.
    acdFront ^= (int)pow(2.0,bitnum);
  }
  while (acdY){
    //now look at the bits in the Y faces
    bitnum = 31 - FFS(acdY);
    masknum = mapBitY(bitnum);
    if (masknum != 65) {
      if ( (sumBits( (m_acd_Front & m_MaskFront[masknum]),
                     (m_acd_Y & m_MaskY[masknum]),
                     (m_acd_X & m_MaskX[masknum]) )
            <= 1) &&
           (sumBits( (m_acd_Front & 0x1ffffff),
                     (m_acd_Y & 0x3ff03ff),
                     (m_acd_X & 0x3ff03ff) )
            == 3) )
        return true;
    }
    acdY ^= (int)pow(2.0,bitnum);
  }
  while (acdX){
    //look at the bits in the X faces
    bitnum = 31 - FFS(acdX);
    masknum = mapBitX(bitnum);
    if (masknum != 65) {
      if ( (sumBits( (m_acd_Front & m_MaskFront[masknum]),
                     (m_acd_Y & m_MaskY[masknum]),
                     (m_acd_X & m_MaskX[masknum]) )
            <= 1) &&
           (sumBits( (m_acd_Front & 0x1ffffff),
                     (m_acd_Y & 0x3ff03ff),
                     (m_acd_X & 0x3ff03ff) )
            == 3) )
        return true;
    }
    acdX ^= (int)pow(2.0,bitnum);
  }
  
  return false;
}

int FilterAlg::mapBitX(int bitnum){
  switch(bitnum)
    {   //x plus
    case 16:  return 55;
    case 17:  return 56;
    case 18:  return 57;
    case 19:  return 58;
    case 20:  return 59;
    case 21:  return 60;
    case 22:  return 61;
    case 23:  return 62;
    case 24:  return 63;
    case 25:  return 64;
      //x minus
    case 0:   return 45;
    case 1:   return 46;
    case 2:   return 47;
    case 3:   return 48;
    case 4:   return 49;
    case 5:   return 50;
    case 6:   return 51;
    case 7:   return 52;
    case 8:   return 53;
    case 9:   return 54;
    }
  return 65;//if none of these tiles were hit
}

int FilterAlg::mapBitY(int bitnum){
  switch(bitnum)
    {   //y plus
    case 16:  return 35;
    case 17:  return 36;
    case 18:  return 37;
    case 19:  return 38;
    case 20:  return 39;
    case 21:  return 40;
    case 22:  return 41;
    case 23:  return 42;
    case 24:  return 43;
    case 25:  return 44;
      //y minus
    case 0:   return 25;
    case 1:   return 26;
    case 2:   return 27;
    case 3:   return 28;
    case 4:   return 29;
    case 5:   return 30;
    case 6:   return 31;
    case 7:   return 32;
    case 8:   return 33;
    case 9:   return 34;
    }
  return 65;//if none of these tiles were hit
}

void FilterAlg::evaluateAcd(){
  
  if (m_acd_X || m_acd_Y || m_acd_Front){
    if (m_energy < 10){
      setVeto(m_E0_TILE_VETO);
      return;
    }
    if ( acdFilter() ){
      if (m_energy < 350){
        setVeto(m_E350_FILTER_TILE_VETO);
        return;
      }
    }
  }
  
  if (m_evaluateAcdUncomment1){
    
    if (m_energy < 40000){
      if ( sumBits(m_acd_X, m_acd_Y, m_acd_Front) >= 4)
        setVeto(m_SPLASH_1_VETO);
      else if ( AFC_splash() )
        setVeto(m_SPLASH_1_VETO);
    }
    
  }
  
  return;
}

bool FilterAlg::acdFilter(){
  //mask off the lower two rows and see if
  //any tiles are left hit in the upper 2
  //or the top
  if ((m_acd_X     & 0x03ff03ff)
      ||(m_acd_Y     & 0x03ff03ff)
      ||(m_acd_Front & 0x01ffffff))
    return true;
  
  return false;
}

void FilterAlg::evaluateAtf(){
  if (m_energy > 5000) return;
  
  bool done = false;
  int count = 0;
  while (!done){ done = compare(count); }
  if (m_throttle) m_throttle=false;
  
  return;
}

void FilterAlg::evaluateZbottom(){
  
  getActiveTowers();
  int active = m_active_towers;
  int tower = 0;
  
  if (m_energy > 100){
    if (m_energy < 10) return;
    
    while (active){
      tower = getTowerID(active);
      //check to see if 4 of 6 planes closest to the cal are hit
      if ( checkPlanes(tower) ){
        return; //a 4/6 was found, so quit
      }
      active ^= (int)pow(2.0,tower);
    }
    
    setVeto(m_ZBOTTOM_VETO);//no 4/6 found
  }
  
  return;
}

bool FilterAlg::checkPlanes(int tower){
  /* The xcapture and ycapture variables each have the layers with hits
     for each given view, but the numbering is unusual.  It looks like this:
     
     1 3 5 7 9 11 13 15 17 0 2 4 6 8 10 12 14 16
     
     for each variable.  So if we want to know if 4 out of 6 layers are
     hit, we have to mask off the bits that we don't want.  We just want
     the layers closest to the cal, so the mask should be:
     
     1 0 0 0 0 0 0 0 0 1 1 0 0 0 0 0 0 0 = 0x20180
     
     Then we count the number of bits, and if there are 4 or more, there is
     the possibility of a track
  */
  
  if ( sumBits(m_xcapture[tower] & 0x20180, m_ycapture[tower] & 0x20180) >= 4 )
    return true;//found enough planes, so zbottom is not true
  else
    return false;
}


void FilterAlg::evaluateCal1(){
  //Currently, all of this code is commented out, but this is what it does if uncommented
  
  if (m_energy > 0){
    if (m_energy < 300 ){
      float e_ratio = m_layerEnergy[0]/m_energy;
//      if (e_ratio <= 0.01){
//        setVeto(m_EL0_ETOT_01_VETO);
//      }
//      else if (e_ratio >= 0.90){
//        setVeto(m_EL0_ETOT_90_VETO);
//      }
       int status = CFC__ratioLayerCheck 
                (4*m_layerEnergy[0], 4*m_energy,
		 10,    m_EL0_ETOT_01_VETO,
		 900,    m_EL0_ETOT_90_VETO);
       setVeto(status);

      
    }
  }
  
  
  
  return;
  
  //And there was another section that checked whether 20% of the
  //logs each had 1% of the energy.  This section has not ever been
  //implemnted since the filter has been in Gleam.  It is not coded here.
}

int FilterAlg::CFC__ratioLayerCheck (int num,      int den,
                                 int lo_limit, int lo_status,
                                 int hi_limit, int hi_status)
{
   int  shf;

   shf = FFS (den) - 0xb;
   
   /*
    | Check to see if have a small enough number to scale by 10 bits.
    | This means at least 11 bits free at the top to stay out of the
    | sign bit. 
   */
   if (shf < 0)
   {
       /* Scale so that there is no overflow */
       num >>= shf;
       den >>= shf;
   }
       

   /* Scale by 10 bits, this gives 1/1024 accuracy */
   num <<= 10;

   if      (num <= lo_limit * den) return lo_status;
   else if (num >= hi_limit * den) return hi_status;


   return 0;
}

/* ---------------------------------------------------------------------- *//*!

  \fn unsigned int tkrFilter  (const GammaCfgTkr       *cfgTkr,
                               unsigned int             energy,
                               unsigned int              acd_x,
                               unsigned int              acd_y,
                               unsigned int              acd_z,
			       unsigned int             vetoes,
                               EDR_tkr                    *tlr,
			       TFC_projections           *prjs)

  \brief  Performs the most computationally intensive portions of the
          filtering. This involves TKR pattern recognition and matching
          to the ACD, skirt region and CAL
  \return A summary status bit mask

  \param cfg    The tracker configuration and cut criteria
  \param energy The total energy in the CAL
  \param acd_x  The bit pattern of struck tiles in the side X+/X- planes
  \param acd_y  The bit pattern of struck tiles in the side Y+/Y- planes  
  \param acd_z  The bit pattern of struck tiles in the top  Z     plane
  \param vetoes The bit mask of vetoes. If, at any time in this routine
                a veto bit is added that matches in this word, the
                routine is aborted.
  \param tlr    The unpacked tracker data for the entire LAT
  \param prjs   Filled in with the projections for this event

                                                                          */
/* ---------------------------------------------------------------------- */
unsigned int FilterAlg::tkrFilter  (const GammaCfgTkr          *cfg,
                                unsigned int             energy,
                                unsigned int              acd_x,
                                unsigned int              acd_y,
                                unsigned int              acd_z,
				unsigned int             vetoes,
				EDR_tkr                    *tlr,
				TFC_projections           *prjs)
{
   EDR_tkrTower          *ttrs;
   TFC_projection         *prj;
   int                    tmsk;
   int                  status;
   int                  curCnt;
   int                dispatch;
   unsigned short int   twrMsk;
   unsigned int        doSkirt;
   const TFC_geometry     *geo;
 
   unsigned int tw   = -1;
   
//   TFC_projectionsPrint(prjs,tw);
    /* If all requested, limit to those that actually have info */

/*    printf("TFC_projectionsPrint input: twrMsk=%x\n",twrMsk);
    if (twrMsk == -1) twrMsk  = prjs->twrMsk << 16;
    else              twrMsk &= 0xffff0000;
    printf("TFC_projectionsPrint: twrMsk=%x\n",twrMsk);
    while (twrMsk)
    {
       int towerId = FFS (twrMsk);
       const TFC_projectionDir *dir = prjs->dir + towerId;
       twrMsk = FFS_eliminate (twrMsk, towerId);
       printProjections (prjs->prjs + dir->idx, dir->xCnt, dir->yCnt, towerId);
    }
    printf("TFC_projectionsPrint: done\n");
*/
   /* 
    | !!! IMPROVEMENT !!! 
    | -------------------
    | Need a better method for selecting only the towers that have their
    | strips unpacked. Don't want the TOT stuff.
    |
    | There is no check for tmsk == 0. This is a precondition of this
    | routine being called, i.e. it's already been done. 
   */
   tmsk   = tlr->twrMap & 0xffff0000;
   curCnt = 0;
   twrMsk = 0;
   tmsk  = prjs->twrMsk << 16;

   /*
    |  !!! KLUDGE !!!
    |  --------------
    |  This is too sloppy, need better way to initialize and keep the
    |  maximum number of projections under control
   */
   
   if (energy > cfg->topEmax) 
   {   
       acd_z = 0;
   }
   
   if (energy > cfg->row01Emax)
   {
       acd_x &= ~ACD_SIDE_TILES_M_ROW01;
       acd_y &= ~ACD_SIDE_TILES_M_ROW01;
   }
     
   if (energy > cfg->row2Emax) 
   {
       acd_x &= ~ACD_SIDE_TILES_M_ROW2;
       acd_y &= ~ACD_SIDE_TILES_M_ROW2;
   }


   /*
    | TFC_acdrojectTemplate produces a bit mask of which ACD planes a
    | candidate track will be projected to. Only planes that have any
    | chance are included.
   */
   dispatch = TFC_acdProjectTemplate ((int)acd_x, (int)acd_y, (int)acd_z);
   doSkirt  = energy < cfg->skirtEmax;
   
   prj    = prjs->prjs;   
   ttrs   = tlr->twrs;
   geo    = cfg->geometry;
   const TFC_geometryTkr    tkrgeo = geo->tkr;
   status = 0;
   while (tmsk)
   {
       int                  tower;
       EDR_tkrTower          *ttr;
//       int     tkrStatus;
       TFC_projectionDir     *dir;
       
       tower = FFS (tmsk);
       ttr   = ttrs + tower;
       tmsk  = FFS_eliminate (tmsk, tower);

	   dir       = prjs->dir + tower;
       prj = prjs->prjs + dir->idx;
       /* Find the projections */
           int   acdStatus;
           int skirtStatus;
           int        xCnt;
           int        yCnt;
           int        tCnt;


           /* Form the projection directory for this tower */
           xCnt = dir->xCnt;
           yCnt = dir->yCnt;

           /* Keep track of which towers have projections and total count */
	   twrMsk   |= (0x8000 >> tower);
	   tCnt      = xCnt + yCnt;
           curCnt   += tCnt;

	   /* 
            | Don't do the TKR/ACD matching if the energy is high to
            | make backsplash a concern.
	   */
	   if (dispatch)
	   {
              /* Project the candidate projections to the ACD planes */
	      acdStatus = TFC_acdProject (prj,
					  xCnt,
					  yCnt,
					  geo,
					  tower,
					  dispatch,
					  acd_x,
					  acd_y,
					  acd_z);
	      
	      /* Check if have any matches */
	      if (acdStatus)
              {
                 /* Have a match, classify the match type */
                 int which;

		 /* Check whether have TOP or SIDE face match */
		 which = (acdStatus & 0xf0000000) >> 28;
		 if (which == 4) status |= EFC_GAMMA_STATUS_M_TKR_TOP;
		 else
		 {
		     /* Side face match, check if have ROW01 or ROW23 match */
		     status |= (acdStatus & 0x3ff) 
		             ? EFC_GAMMA_STATUS_M_TKR_ROW01
		             : EFC_GAMMA_STATUS_M_TKR_ROW2;
		 }

		 if (isVetoed (status, vetoes)) goto EXIT;
	      }
           }

           
	   /* No ACD match, try projecting to the skirt */
           if (doSkirt)
           {
                 
               skirtStatus = FilterAlg_skirtProject (prj,
                                               xCnt,
                                               yCnt,
                                               geo,
                                               tower);

               /*
                | !!! KLUDGE !!!
                | --------------
                | Need a decision on this cut. Does one need to cancel
                | this cut if have 2 or more tracks in the event. Currently
                | it is not cancelled, so the logic says, as soon as
                | one finds any track in the skirt region, that's it.
               */
               if (skirtStatus)  
	       {
		 status |= EFC_GAMMA_STATUS_M_TKR_SKIRT;
		 if (isVetoed (status, vetoes)) goto EXIT;
	       }
           }

   }


   /* If there is evidence for less than 2 complete tracks... */
   if (curCnt < 3)
   {
       if (curCnt < 2)  
       { 
	   if (energy >= cfg->zeroTkrEmin)
	   {
               status |= EFC_GAMMA_STATUS_M_TKR_EQ_0;
	   }
	   goto EXIT;
       }


       /* Number of projections == 2, so call it one full track */
       status |= EFC_GAMMA_STATUS_M_TKR_EQ_1;

       /* If energy low enough, must have some evidence of two tracks */
       if (energy < cfg->twoTkrEmax) 
       {
           status |= EFC_GAMMA_STATUS_M_TKR_LT_2_ELO;
       }
   }
   else
   {
       /* Note that have ge 2 tracks */
       status |= EFC_GAMMA_STATUS_M_TKR_GE_2;
   }

   
 EXIT:
   EDM_INFOPRINTF ((EFC_Filter_edm, 
		    "LAT FILTER STATUS = %8.8x (xyCnt = %d)\n",
		    status, 
		    curCnt));
  

   return status;
}



int FilterAlg::getTowerID(unsigned int towers)
{
  //looks at the list of triggered towers, and gets the first tower
  //to look at.
  if (towers & 0x1) return 0;
  if (towers & 0x2) return 1;
  if (towers & 0x4) return 2;
  if (towers & 0x8) return 3;
  if (towers & 0x10) return 4;
  if (towers & 0x20) return 5;
  if (towers & 0x40) return 6;
  if (towers & 0x80) return 7;
  if (towers & 0x100) return 8;
  if (towers & 0x200) return 9;
  if (towers & 0x400) return 10;
  if (towers & 0x800) return 11;
  if (towers & 0x1000) return 12;
  if (towers & 0x2000) return 13;
  if (towers & 0x4000) return 14;
  if (towers & 0x8000) return 15;
  
  return -1; //if there are no matches
}

void FilterAlg::getMask(int towerID)
{
  m_maskTop = 0;
  m_maskX = 0;
  m_maskY = 0;
  
  //  If using the top 2 rows of the acd, use up to 12 tiles
  switch (towerID)
    {
    case 0://                     28   24   20   16   12   8    4    0
      m_maskTop = 0x63;    //0000 0000 0000 0000 0000 0000 0110 0011
      m_maskX = 0x63;      //0000 0000 0000 0000 0000 0000 0110 0011
      m_maskY = 0x63;      //0000 0000 0000 0000 0000 0000 0110 0011
      return;
    case 1://                     28   24   20   16   12   8    4    0
      m_maskTop = 0xC6;    //0000 0000 0000 0000 0000 0000 1100 0110
      m_maskX = 0;         //0
      m_maskY = 0xC6;      //0000 0000 0000 0000 0000 0000 1100 0110
      return;
    case 2://                     28   24   20   16   12   8    4    0
      m_maskTop = 0x18C;   //0000 0000 0000 0000 0000 0001 1000 1100
      m_maskX = 0;         //0
      m_maskY = 0x18C;     //0000 0000 0000 0000 0000 0001 1000 1100
      return;
    case 3://                     28   24   20   16   12   8    4    0
      m_maskTop = 0x318;   //0000 0000 0000 0000 0000 0011 0001 1000
      m_maskX = 0x630000;  //0000 0000 0110 0011 0000 0000 0000 0000
      m_maskY = 0x318;     //0000 0000 0000 0000 0000 0011 0001 1000
      return;
    case 4://                     28   24   20   16   12   8    4    0
      m_maskTop = 0xC60;   //0000 0000 0000 0000 0000 1100 0110 0000
      m_maskX = 0xC6;      //0000 0000 0000 0000 0000 0000 1100 0110
      m_maskY = 0;         //0
      return;
    case 5://                      28   24   20   16   12   8    4    0
      m_maskTop = 0x18C0;   //0000 0000 0000 0000 0001 1000 1100 0000
      m_maskX = 0;          //0
      m_maskY = 0;          //0
      return;
    case 6://                     28   24   20   16   12   8    4    0
      m_maskTop = 0x3180;  //0000 0000 0000 0000 0011 0001 1000 0000
      m_maskX = 0;         //0
      m_maskY = 0;         //0
      return;
    case 7://                     28   24   20   16   12   8    4    0
      m_maskTop = 0x6300;  //0000 0000 0000 0000 0110 0011 0000 0000
      m_maskX = 0xC60000;  //0000 0000 1100 0110 0000 0000 0000 0000
      m_maskY = 0;         //0
      return;
    case 8://                      28   24   20   16   12   8    4    0
      m_maskTop = 0x18C00;  //0000 0000 0000 0001 1000 1100 0000 0000
      m_maskX = 0x18C;      //0000 0000 0000 0000 0000 0001 1000 1100
      m_maskY = 0;          //0
      return;
    case 9://                      28   24   20   16   12   8    4    0
      m_maskTop = 0x31800;  //0000 0000 0000 0011 0001 1000 0000 0000
      m_maskX = 0;          //0
      m_maskY = 0;          //0
      return;
    case 10://                    28   24   20   16   12   8    4    0
      m_maskTop = 0x63000; //0000 0000 0000 0110 0011 0000 0000 0000
      m_maskX = 0;         //0
      m_maskY = 0;         //0
      return;
    case 11://                    28   24   20   16   12   8    4    0
      m_maskTop = 0xC6000; //0000 0000 0000 1100 0110 0000 0000 0000
      m_maskX = 0x18C0000; //0000 0001 1000 1100 0000 0000 0000 0000
      m_maskY = 0;         //0
      return;
    case 12://                     28   24   20   16   12   8    4    0
      m_maskTop = 0x318000; //0000 0000 0011 0001 1000 0000 0000 0000
      m_maskX = 0x318;      //0000 0000 0000 0000 0000 0011 0001 1000
      m_maskY = 0x630000;   //0000 0000 0110 0011 0000 0000 0000 0000
      return;
    case 13://                    28   24   20   16   12   8    4    0
      m_maskTop = 0x630000;//0000 0000 0110 0011 0000 0000 0000 0000
      m_maskX = 0;         //0
      m_maskY = 0xC60000;  //0000 0000 1100 0110 0000 0000 0000 0000
      return;
    case 14://                    28   24   20   16   12   8    4    0
      m_maskTop = 0xC60000;//0000 0000 1100 0110 0000 0000 0000 0000
      m_maskX = 0;         //0
      m_maskY = 0x18C0000; //0000 0001 1000 1100 0000 0000 0000 0000
      return;
    case 15://                    28   24   20   16   12   8    4    0
      m_maskTop =0x18C0000;//0000 0001 1000 1100 0000 0000 0000 0000
      m_maskX = 0x3180000; //0000 0011 0001 1000 0000 0000 0000 0000
      m_maskY = 0x3180000; //0000 0011 0001 1000 0000 0000 0000 0000
      return;
    }
  return;
}

void FilterAlg::removeTower(int twr, unsigned int& triggeredtowers)
{
  if (twr==0){triggeredtowers ^= 0x1; return;}
  if (twr==1){triggeredtowers ^= 0x2; return;}
  if (twr==2){triggeredtowers ^= 0x4; return;}
  if (twr==3){triggeredtowers ^= 0x8; return;}
  if (twr==4){triggeredtowers ^= 0x10; return;}
  if (twr==5){triggeredtowers ^= 0x20; return;}
  if (twr==6){triggeredtowers ^= 0x40; return;}
  if (twr==7){triggeredtowers ^= 0x80; return;}
  if (twr==8){triggeredtowers ^= 0x100; return;}
  if (twr==9){triggeredtowers ^= 0x200; return;}
  if (twr==10){triggeredtowers ^= 0x400; return;}
  if (twr==11){triggeredtowers ^= 0x800; return;}
  if (twr==12){triggeredtowers ^= 0x1000; return;}
  if (twr==13){triggeredtowers ^= 0x2000; return;}
  if (twr==14){triggeredtowers ^= 0x4000; return;}
  if (twr==15){triggeredtowers ^= 0x8000; return;}
  
  return;
}

bool FilterAlg::compare(int &count)
{
  int tower = 0;
  int start_layer = 0;
  int start_layer_2 = 0;
  int start_layer_3 = 0;
  int tworow = 2;
  int threerow = 3;
  const int Row0 = 0;
  const int Row01 = 1;
  const int Row012 = 2;
  int valid_tower = -1;
  bool done = true;
  bool notdone = false;
  unsigned int possible_towers;
  if (count == 0){
    possible_towers = getPossible();
    count++;
  }
  
  if ((tower = getTowerID(possible_towers)) == -1) return done;//no more towers
  getMask(tower);
  removeTower(tower,possible_towers);
  if ( (m_acd_Front & m_maskTop)||(m_acd_X & m_maskX)||(m_acd_Y & m_maskY) )
    {
      m_throttle = true;
      valid_tower = coincidenceLevel(tower);
      
      if ((int)pow(2.0,tower) & m_triggered_towers)
        start_layer_3 = getStart(tower,threerow);
      if ((int)pow(2.0,15-tower) & (m_tcids & 0xffff))//Have to do 15-tower because tcids
        start_layer_2 = getStart(tower,tworow);     //is numbered from left to right
      if (start_layer_2 > start_layer_3) start_layer = start_layer_2;
      else start_layer = start_layer_3;
      
      int newstart = triggerForm(m_xcapture[tower],m_ycapture[tower]);
      if (valid_tower >=0) start_layer = valid_tower;
      
      if (valid_tower>=0){
//        if ((m_acd_Front & m_maskTop) && (start_layer > 14)){
        if ((m_acd_Front & m_maskTop) && (newstart <= 2)){
          setVeto(m_TOP_VETO);
                   return done;
        }
        else if ((m_acd_X & m_maskX) || (m_acd_Y & m_maskY)){
          if      ((start_layer >= 13)) getRowMask(tower, Row0);
          else if ((start_layer <=12) && (start_layer >=6)) getRowMask(tower, Row01);
          else if ((start_layer <=5)) getRowMask(tower,Row012);
          
          if ( (m_acd_X & m_maskX & m_rowMaskX) || (m_acd_Y & m_maskY & m_rowMaskY) ){
            setVeto(m_SIDE_VETO);
            return done;
          }
        }
      }
      
    }
  if (possible_towers==0) return done;
  return notdone;
}

int FilterAlg::triggerForm (unsigned int x, unsigned int y)
{
   unsigned int      xt;
   unsigned int      yt;
   unsigned int    xy00;
   unsigned int    xy11;
   unsigned int    xy22;
   unsigned int    xy01;
   unsigned int    xy02;
   unsigned int    xy12;
   unsigned int    xy13;
   unsigned int    xy23;
   unsigned int   xy012;
   unsigned int   xy013;
   unsigned int   xy023;
   unsigned int   xy123;
   unsigned int trigger;
   int            start;
   int           length;
   
   start = 31;
  /*
            876543210fedcba9876543210
       x =         13579bdfh02468aceg
       y =         13579bdfh02468aceg
       
      xt   = (x & 0x1ff) | ((x & (0x1ff << 9) << 7));
      yt   = (y & 0x1ff) | ((y & (0x1ff << 9) << 7));      
      
      xt  = 13579bdfh.......02468aceg
      xt  = 13579bdfh.......02468aceg


      xy00 = xt & yt;
      xy11 = (xy00 << 16) | (xy00 >> 17)
      xy22 = (xy00 >> 1)
      xy33 =  xy11 >> 1)

                fedcba9876543210fedcba9876543210 
      xy01 = xy(       13579bdfh       02468aceg)    xy00
           & xy(       02468aceg       _13579bdf)    xy11
           = xy00 &  xy11;

      xy02 = xy(       13579bdfh       02468aceg)    xy00
           = xy(       _13579bdfh      _02468ace)    xy22
           = xy00 & xy22;

      xy12 = xy(       02468aceg       _13579bdf)    xy11
           = xy(       _13579bdfh      _02468ace)    xy22
           =  xy11 & xy22;
             
      xy13 = xy(       02468aceg       _13579bdf)    xy11
           & xy(       _02468aceg       _13579bd)    xy33
           = xy11 & (xy11 >> 1);

      xy23 = xy(       _13579bdfh      _02468ace)    xy22
           & xy(       _02468aceg       _13579bd)    xy33
           = xy01 >> 1;
   */

   
   /*
    | To get a 7/8 or 6/6, must have at least a 4/4 somewhere. This is
    | a quick check to eliminate towers with no possibility of a trigger.
    | Initialize the return value to be the 2/2 coincidence.
   */
   xt   =  (x & 0x1ff) | ((x & (0x1ff << 9)) << 7);
   yt   =  (y & 0x1ff) | ((y & (0x1ff << 9)) << 7);
   xy00 = (xt & yt);
   xy11 = (xy00 << 16) | (xy00 >> 17);
   xy01 =  xy00 & xy11;
//   printf ("X   : %8.8x\n"
//                        "Y   : %8.8x\n"
//                        "XT  : %8.8x\n"
//                        "YT  : %8.8x\n"
//                        "XY00: %8.8x\n"
//                        "XY11: %8.8x\n"
//                        "XY01: %8.8x\n",
//                        x, y, xt, yt, xy00, xy11, xy01);
   
   if (xy01 == 0)
   {
       /*
        | No need to fill in remaining trigger fields, they are defaulted.
        | In this case and in this case only, the layer bits are still in
        | the accept order. For the most part no one ever looks at the
        | layer map when there is less than a 4/4 coincidence. Basically,
        | doing the remapping isn't worth the time. (Remember, this case
        | is the most common fate, so saving time here is worth this
        | complication.)
       */
       return start;
   }
   
   
   /*
    | Try going for 7/8 layer coincidence. In order to get the length
    | the coincidence (ie the number of struck planes) correct, the
    | length is initialized at 1. That is because the minimum coincidence
    | length for 7/8 is 4 planes and the minimum coincidence length for
    | 3/3 is 3 planes. Arbitrarily, the length of coincidence is baselined
    | at 3 planes.
   */
   xy22      = xy00 >> 1;
   xy02      = xy00 & xy22;
   xy12      = xy11 & xy22;
   xy13      = xy11 & (xy11 >> 1);
   xy23      = xy01 >> 1;
   xy012     = xy01 & xy22;
   xy013     = xy01 & xy13;
   xy023     = xy02 & xy23;
   xy123     = xy12 & xy23;
   trigger   = trigger7of8Form (xt, yt, xy012, xy013, xy023, xy123);
   
   
   length    = 3;
//   t.si      = 0;
//   t.bf.type = TFC_K_TRIGGER_TYPE_7_OF_8;

   
   /*
    | If no 7/8, try 6/6 coincidence. In order to get the length of
    | the coincidence (ie the number of struck planes) correct, the
    | length is initialized at 0.
   */
   if (trigger == 0)
   {
       /*
                       fedcba9876543210fedcba9876543210
            xy012 = xy(       13579bdfh       02468aceg)    xy00
                  & xy(       02468aceg       _13579bdf)    xy11
                  & xy(       _13579bdfh      _02468ace)    xy22
       */
      trigger = xy01 & xy02;
      

      /* No 6/6 of coincidence */  
      if (trigger == 0)
      {
          /* Only made it to 4/4 */
//          t.si        = 0;
//          t.bf.type   = TFC_K_TRIGGER_TYPE_4_OF_4;
//          t.bf.layers = ((xy01 >> OFFSET_ODD) & (0x1ff << 9)) | (xy01 & 0x1ff);
          return start;
      }

      /* No 7/8, but did have a 6/6 */
//      t.bf.type = TFC_K_TRIGGER_TYPE_6_OF_6;
//      length    = 2;
   }
   else
   {
   }
   

   trigger = triggerRemap (trigger);
   
   /*
    | Must have a trigger now. Find the starting layer number and the
    | length of the coincidence. Note that the layer closest to the ACD
    | top plane is labelled as 0. 
   */
   trigger   <<= 32 - 18;        
   start       = FFS (trigger);   
   return start;
}

int FilterAlg::trigger7of8Form (unsigned int x,
                                 unsigned int y, 
                                 unsigned int xy012,
                                 unsigned int xy013,
                                 unsigned int xy023,
                                 unsigned int xy123)
{
   unsigned int      xy;
   unsigned int x_or_y0;
   unsigned int x_or_y1;

   /*

    x_or_y0 =   (       13579bdfh       02468aceg)
    x_or_y1 =   (       02468aceg       _13579bdf)

                 fedcba9876543210fedcba9876543210    
      xy012 = xy(       13579bdfh       02468aceg)    xy00
            & xy(       02468aceg       _13579bdf)    xy11
            & xy(       _13579bdfh      _02468ace)    xy22
            &   (       _02468aceg      __13579bd)    x_or_y1 >> 1

      xy013 = xy(       13579bdfh       02468aceg)    xy00
            & xy(       02468aceg       _13579bdf)    xy11
            &   (       _13579bdfh      _02468ace)    x_or_y0 >> 1            
            & xy(       _02468aceg      __13579bdf)   xy33


      xy023 = xy(       13579bdfh       02468aceg)    xy00
            &   (       02468aceg       _13579bdfh)   x_or_y1      
            & xy(       _13579bdfh      _02468ace)    xy22
            & xy(       _02468aceg      __13579bdf)   xy33


      xy123 =   (       13579bdfh       02468aceg)    x_or_y0
            & xy(       02468aceg       _13579bdf)    xy11
            & xy(       _13579bdfh      _02468ace)    xy22
            & xy(       _02468aceg      __13579bdf)   xy33
            
   */
   x_or_y0 = (x | y);
   x_or_y1 = (x_or_y0 << 16) | (x_or_y0 >> 17);



   /* Now form all the triple coincidence AND'd with the missing X or Y   */
   xy   = ((xy012 & ((x_or_y1)>>1)) |          /* (xy321 & x|y(0)) */
           (xy013 & ((x_or_y0)>>1)) |          /* (xy320 & x|y(1)) */
           (xy023 & ((x_or_y1)>>0)) |          /* (xy310 & x|y(2)) */
           (xy123 & ((x_or_y0)>>0)));          /* (xy210 & x|y(3)) */

   
   return (int) xy;
}

 int FilterAlg::triggerRemap (int trigger)
{
   /*
    | Two variables, 'even' and 'odd' are used during the remapping
    | even though, strictly speaking, only one is needed. Using two
    | variables allows both integer units on those processors that
    | have it to be used. Using an extra variable costs one additional
    | OR instruction, but allows 18 other instructions to be executed
    | in pairs.
    |
    | This routine should take on the order of 10 clock cycles, or
    | for 133 MHz RAD750, about 75nsecs.
   */
   unsigned int even;
   unsigned int  odd;

   /*
    | Note that layers 0 and 1 are not remapped. They can never begin
    | a coincidence, hence no need to remap them.
   */
   TFC_TRG_REMAP_INIT (trigger, even, odd, 2, 3);   
   TFC_TRG_REMAP      (trigger, even, odd, 4, 5);
   TFC_TRG_REMAP      (trigger, even, odd, 6, 7);
   TFC_TRG_REMAP      (trigger, even, odd, 8, 9);
   TFC_TRG_REMAP      (trigger, even, odd, A, B);
   TFC_TRG_REMAP      (trigger, even, odd, C, D);
   TFC_TRG_REMAP      (trigger, even, odd, E, F);
   TFC_TRG_REMAP      (trigger, even, odd, G, H);

   

   return even | odd;
}

void FilterAlg::getRowMask(int tower, int row){
  switch (tower)
    {
    case 0:{//x-, y-
      if      (row == 0){ m_rowMaskX = 0x3; m_rowMaskY = 0x3; return; }
      else if (row == 1){ m_rowMaskX = 0x63; m_rowMaskY = 0x63; return; }
      else if (row == 2){ m_rowMaskX = 0xC63; m_rowMaskY = 0xC63; return; } }
    case 1:{//y-
      if      (row == 0){ m_rowMaskX = 0; m_rowMaskY = 0x6; return; }
      else if (row == 1){ m_rowMaskX = 0; m_rowMaskY = 0xC6; return; }
      else if (row == 2){ m_rowMaskX = 0; m_rowMaskY = 0x18C6; return; } }
    case 2:{//y-
      if      (row == 0){ m_rowMaskX = 0; m_rowMaskY = 0xC; return; }
      else if (row == 1){ m_rowMaskX = 0; m_rowMaskY = 0x18C; return; }
      else if (row == 2){ m_rowMaskX = 0; m_rowMaskY = 0x318C; return; } }
    case 3:{//x+, y-
      if      (row == 0){ m_rowMaskX = 0x30000; m_rowMaskY = 0x18; return; }
      else if (row == 1){ m_rowMaskX = 0x630000; m_rowMaskY = 0x318; return; }
      else if (row == 2){ m_rowMaskX = 0xC630000; m_rowMaskY = 0x6318; return; } }
    case 4:{//x-
      if      (row == 0){ m_rowMaskX = 0x6; m_rowMaskY = 0; return; }
      else if (row == 1){ m_rowMaskX = 0xC6; m_rowMaskY = 0; return; }
      else if (row == 2){ m_rowMaskX = 0x18C6; m_rowMaskY = 0; return; } }
    case 5:{
      if      (row == 0){ m_rowMaskX = 0; m_rowMaskY = 0; return; }
      else if (row == 1){ m_rowMaskX = 0; m_rowMaskY = 0; return; }
      else if (row == 2){ m_rowMaskX = 0; m_rowMaskY = 0; return; } }
    case 6:{
      if      (row == 0){ m_rowMaskX = 0; m_rowMaskY = 0; return; }
      else if (row == 1){ m_rowMaskX = 0; m_rowMaskY = 0; return; }
      else if (row == 2){ m_rowMaskX = 0; m_rowMaskY = 0; return; } }
    case 7:{//x+
      if      (row == 0){ m_rowMaskX = 0x60000; m_rowMaskY = 0; return; }
      else if (row == 1){ m_rowMaskX = 0xC60000; m_rowMaskY = 0; return; }
      else if (row == 2){ m_rowMaskX = 0x18C60000; m_rowMaskY = 0; return; } }
    case 8:{//x-
      if      (row == 0){ m_rowMaskX = 0xC; m_rowMaskY = 0; return; }
      else if (row == 1){ m_rowMaskX = 0x18C; m_rowMaskY = 0; return; }
      else if (row == 2){ m_rowMaskX = 0x318C; m_rowMaskY = 0; return; } }
    case 9:{
      if      (row == 0){ m_rowMaskX = 0; m_rowMaskY = 0; return; }
      else if (row == 1){ m_rowMaskX = 0; m_rowMaskY = 0; return; }
      else if (row == 2){ m_rowMaskX = 0; m_rowMaskY = 0; return; } }
    case 10:{
      if      (row == 0){ m_rowMaskX = 0; m_rowMaskY = 0; return; }
      else if (row == 1){ m_rowMaskX = 0; m_rowMaskY = 0; return; }
      else if (row == 2){ m_rowMaskX = 0; m_rowMaskY = 0; return; } }
    case 11:{//x+
      if      (row == 0){ m_rowMaskX = 0xC0000; m_rowMaskY = 0; return; }
      else if (row == 1){ m_rowMaskX = 0x18C0000; m_rowMaskY = 0; return; }
      else if (row == 2){ m_rowMaskX = 0x318C0000; m_rowMaskY = 0; return; } }
    case 12:{//x-, y+
      if      (row == 0){ m_rowMaskX = 0x18; m_rowMaskY = 0x30000; return; }
      else if (row == 1){ m_rowMaskX = 0x318; m_rowMaskY = 0x630000; return; }
      else if (row == 2){ m_rowMaskX = 0x6318; m_rowMaskY = 0xC630000; return; } }
    case 13:{//y+
      if      (row == 0){ m_rowMaskX = 0; m_rowMaskY = 0x60000; return; }
      else if (row == 1){ m_rowMaskX = 0; m_rowMaskY = 0xC60000; return; }
      else if (row == 2){ m_rowMaskX = 0; m_rowMaskY = 0x18C60000; return; } }
    case 14:{//y+
      if      (row == 0){ m_rowMaskX = 0; m_rowMaskY = 0xC0000; return; }
      else if (row == 1){ m_rowMaskX = 0; m_rowMaskY = 0x18C0000; return; }
      else if (row == 2){ m_rowMaskX = 0; m_rowMaskY = 0x318C0000; return; } }
    case 15:{//x+, y+
      if      (row == 0){ m_rowMaskX = 0x180000; m_rowMaskY = 0x180000; return; }
      else if (row == 1){ m_rowMaskX = 0x3180000; m_rowMaskY = 0x3180000; return; }
      else if (row == 2){ m_rowMaskX = 0x63180000; m_rowMaskY = 0x63180000; return; } }
    }
  m_rowMaskX = 0;
  m_rowMaskY = 0;
  return;
}

int FilterAlg::getPossible(){
  int possible = 0;
  int bit = 0;
  int temp = 0;
  possible |= ((m_tcids >> 16) | m_tcids);
  while (possible){
    bit = FFS(possible);
    temp |= (int)pow(2.0,bit);
    possible ^= (int)pow(2.0,31-bit);
  }
  temp = (temp >> 16) & 0x0000ffff;
  return temp;
}

void FilterAlg::getActiveTowers(){
  m_active_towers = 0;
  for (int i=0; i<16;i++){
    if (m_xcapture[i] || m_ycapture[i]) m_active_towers |= (int)pow(2.0,i) ;
  }
  return;
}

int FilterAlg::getStart(int tower, int rows){
  //here, get the starting layer of the coincidence for this tower
  if (rows == 3){
    if (m_coincidences_3[tower]) return 31 - FFS(m_coincidences_3[tower]);
  }
  if (rows == 2){
    if (m_coincidences_2[tower]) return 31 - FFS(m_coincidences_2[tower]);
  }
  return 0;
}

int FilterAlg::coincidenceLevel(int tower){
  //determine if there is a 6/6 or 7/8 in this tower.  Use xcapture and ycapture
  int xlay3, ylay3, xlay4, ylay4; //local versions of xlayers and ylayers
  int xlay_temp, ylay_temp;
  int six_layers = -1;
  int seven_layers = -1;
  
  xlay3 = m_xcapture[tower];
  ylay3 = m_ycapture[tower];
  xlay4 = m_xcapture[tower];
  ylay4 = m_ycapture[tower];
  
  reorderLayers(xlay3);
  reorderLayers(ylay3);
  reorderLayers(xlay4);
  reorderLayers(ylay4);
  
  int i;
  //look for 6 out of 6 bits set
  for (i = 0; i<16; i++){
    xlay_temp = xlay3 & 0x7;//mask off 3 bits to look at
    ylay_temp = ylay3 & 0x7;
    xlay3 = xlay3 >> 1;     //shift the bits to the right (knocking off the lsb)
    ylay3 = ylay3 >> 1;
    if ( (cntBits(xlay_temp) + cntBits(ylay_temp)) == 6) six_layers = 2+i;;
  }
  //look for 7 out of 8 bits set
  for (i = 0; i<15; i++){
    xlay_temp = xlay4 & 0xf;//mask off 4 bits to look at
    ylay_temp = ylay4 & 0xf;
    xlay4 = xlay4 >> 1;
    ylay4 = ylay4 >> 1;
    if ( (cntBits(xlay_temp) + cntBits(ylay_temp)) > 6) seven_layers = 3+i;
  }
  
  if (six_layers > seven_layers) return six_layers;
  else return seven_layers;
  
  //return false;
}

void FilterAlg::reorderLayers(int& layers){
  int temp = 0;
  int start = 8;//this is not a typo
  int index2 = 0;
  int tempindex = 0;
  
  while (start >= 0){
    index2 = start + 9;
    if (layers & (int)pow(2.0,start))
      temp |= (int)pow(2.0,tempindex);
    if (layers & (int)pow(2.0,index2))
      temp |= (int)pow(2.0,tempindex+1);
    tempindex += 2;
    start--;
  }
  
  layers = temp;
  return;
}

int FilterAlg::sumBits(unsigned int word1, unsigned int word2){
  
  return cntBits(word1) + cntBits(word2);
}

int FilterAlg::sumBits(unsigned int word1, unsigned int word2, unsigned int word3){
  
  return cntBits(word1) + cntBits(word2) + cntBits(word3);
}

int FilterAlg::cntBits (unsigned int word)
{
  int count = 0;
  
  while (word)
    {
      int bit = FFS (word);
      
      count += 1;
      word  &= ~(0x80000000 >> bit);
    }
  
  return count;
}

int FilterAlg::FFS (unsigned int lw){
  int n;
  int cpy = lw;
  if (cpy == 0) n = 32;
  else  { n = 0; while(cpy >= 0) { cpy <<= 1; n++; } }
  return n;
}

int FilterAlg::createTowerMask(int towerword){
  
  return ( ( (towerword & 0xffff0000) >> 0x10)|(towerword & 0x0000ffff) );
}
/*
  void FilterAlg::findProjections(int tower, prjs& projections){
  //find the projections here!
  }
*/
void FilterAlg::setVeto(int veto){
  m_vetoword |= (int)pow(2.0,(veto-15));
  return;
}


/* ---------------------------------------------------------------------- *//*!

  \fn            int isVetoed (unsigned int status, unsigned int vetoes)
  \brief         Checks whether any of the veto bits are up
  \return        Non-zero if the \a status bits contain any of the 
                 veto bits.

  \param  status The current set of status bits
  \param  vetoes The set of veto bits

                                                                          */
/* ---------------------------------------------------------------------- */
int FilterAlg::isVetoed (unsigned int status, unsigned int vetoes)
{
  return status & vetoes;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     const TFC_geometry *locateGeo (int id, int printIt)
  \brief  Locates the specified geometry within the list of available
          geometries.
  \return A pointer to the specified geometry.

  \param      id  The id of the specified geometry.
  \param printIt  If non-zero, prints the geometry.

   If the specified geometry is not located, a list of the available 
   geometries is printed and the program exits
									  */
/* ---------------------------------------------------------------------- */
const TFC_geometry *FilterAlg::locateGeo (int id, int printIt)
{

    const TFC_geometry *geo;

    /* Lookup the specified geometry */
    geo = TFC_geosLocate (TFC_Geos, -1, id);

    /* If can't find the specified geometry, report error and quit */
    if (!geo) 
    {
       printf ("ERROR: Unable to locate the specified geometry, id = %d\n"
	       "       The available choices are\n\n");

       EDM_CODE 
       (
          {
            int  idx;

	    /* Print just a header line */
	    puts ("");
	    TFC_geometryPrint (NULL, TFC_M_GEO_OPTS_TAG_HDR);

	    /* Print the tag for each */
	    idx = 0;
	    while ((geo = TFC_Geos[idx++])) 
	    {
	     TFC_geometryPrint (geo, TFC_M_GEO_OPTS_TAG);
	    }
         }
      )

       exit (-1);
    }

   
    /* Print the detector geometry if desired */
    EDM_CODE
    (
     puts ("");
     TFC_geometryPrint (geo, 
                        printIt ? TFC_M_GEO_OPTS_ALL 
                                : TFC_M_GEO_OPTS_TAG_HDR | TFC_M_GEO_OPTS_TAG);
    )
 
    return geo;
}
/* ---------------------------------------------------------------------- */




/*
  Into TDS: xy00, xy11, xy22, xy33 from line 814 and following
  Into root file: Have to do some manipulations.  xy1 = (xy00 & xy11 & xy 22), xy2 = (xy00 & xy11 & xy33).
  xy1 has bits that represent coincidences that begin in odd or even layers, depending on whether one is
  looking for 2 bi-layer coincidences or 3 bi-layer coincidences:
  bit position   8  7  6  5  4  3  2  1  0
  value              1  3  5  7  9  11 13 15 17 when looking at 2 bi-layer coincidences
  value          2  4  6  8  10 12 14 16 -- when looking at 3 bi-layer coincidences
  
  xy2 has bits that represent coincidences that begin in even or odd layers, depending on whether one is
  looking for 2 bi-layer coincidences or 3 bi-layer coincidences:
  bit position   8  7  6  5  4  3  2  1  0
  value              2  4  6  8  10 12 14 16 -- when looking at 2 bi-layer coincidences
  1  3  5  7  9  11 13 15 17 when looking at 3 bi-layer coincidences
  
  We need to form a new word that merges these bits like this:
  bit position 10  F  E  D  C  B  A 9 8 7 6 5 4 3 2 1 0
  value        16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
  where the "value" bits are taken from xy1 and xy2.
  We call this "m_coincidences_2[]" or "m_coincidences_3[]" depending on how many layers
  we are looking for.
  
  Note that in this layer numbering, it counts up from the ACD, but later
  he reverses it, and counts down!
  
  However, xcapture and ycapture (in addition to xy00) are always numbered in this order:
  1 3 5 7 9 11 13 17 0 2 4 6 8 10 12 14 16
  Then xy11:         1 3 5 7 9 11 13 15 17
  And xy22:        0 2 4 6 8 1012 14 16 --
  And xy33:            1 3 5 7 9  11 13 15
  ...which we get from these bit manipulations:
  xy00 = xcapture & ycapture;
  xy11 = xy00 >> 9;
  xy22 = xy00 << 1;
  xy33 = xy00 >> 0xa;
*/

