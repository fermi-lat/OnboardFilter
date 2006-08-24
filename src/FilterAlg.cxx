

#define EFC_DFILTER

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>





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

#include "OnboardFilterTds/FilterStatus.h"
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


/* --------------------------------------------------------------------- */




static const AlgFactory<FilterAlg> Factory;
const IAlgFactory& FilterAlgFactory = Factory;  
  
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
  
//  m_CAL_LO = 0x400;
//  m_CAL_HI = 0x800;
  
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
 
  return StatusCode::SUCCESS;
  
}
