#include "OnboardFilter/FilterAlg.h"

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
	initMasks(m_MaskFront,m_MaskY,m_MaskX);

	m_CAL_LO = 0x400;
	m_CAL_HI = 0x800;

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
    return StatusCode::SUCCESS;
}

StatusCode FilterAlg::finalize(){
    MsgStream log(msgSvc(),name());
    log<<MSG::DEBUG<<"Finalizing"<<endreq;
    return StatusCode::SUCCESS;
}

StatusCode FilterAlg::execute(){
    MsgStream log(msgSvc(),name());
    log<<MSG::DEBUG<<"execute()"<<endreq;
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
        log<<MSG::ERROR<<"Unable to retrieve FilterStatus from TDS"<<endreq;
        return StatusCode::FAILURE;
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
    tkrFilter(tkrGeoSvc);

    //All Vetoes have been filled in.
    m_count++;//for debugging purposes
	//put the vetoword in the tds
  	newFilterAlgData->setVetoWord(m_vetoword);

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
    m_MaskFront[4]= /*0x184;*/ 0x308;     m_MaskY[4]= 0x18;      m_MaskX[4]=  0x30000;
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
	   else if (numbits == 3){
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
          if ( (e_ratio > 0.01) && (e_ratio < 0.90))
			  return;
		  else if (e_ratio <= 0.01){
			  setVeto(m_EL0_ETOT_01_VETO);
			  return;
		  }
		  else if (e_ratio >= 0.90){
		      setVeto(m_EL0_ETOT_90_VETO);
		      return;
		  }
       }
    }
	return;

	//And there was another section that checked whether 20% of the
    //logs each had 1% of the energy.  This section has not ever been
    //implemnted since the filter has been in Gleam.  It is not coded here.
}



void FilterAlg::tkrFilter(ITkrGeometrySvc *tkrGeoSvc){

   const int TOPFACE = 0;
   const int ROW01 = 1;
   const int ROW23 = 2;
   int numProjections = 0;
   int totalnumProjections = 0;
   int firstmatch, tilematch, dispatch, acd_firstmatch;
   int prjcnt, view, prjstart, counter;

    const int id = 0;
   	const struct _TFC_geometry *geo;
    geo = TFC_geosLocate (TFC_Geos, -1, id);

    //Get filter info from TDS
    SmartDataPtr<OnboardFilterTds::FilterStatus> filter(eventSvc(),
        "/Event/Filter/FilterStatus");

   //if there is NO possibility of a TKR trigger,
   //and the energy is less than 250 MeV
   if ( m_tcids == 0 ){
      if (m_energy > 250) setVeto(m_TKR_EQ_0_VETO);
   }
   else{
       unsigned int tmsk = createTowerMask(m_tcids);
       dispatch = FilterAlg_acdProjectTemplate (m_acd_X, m_acd_Y, m_acd_Front);
	   do{

          int tower;
	      tower = FFS (tmsk) - 16;
	      tmsk &= ~(0x80000000 >> (tower+16));

          /*for each tower that has a 2 or 3IAR, find projections using
	      whatever method the user chooses.  If the user wants to use the
	      method that the filter uses, the user can just take the output
	      of the filter and is mostly done.

          Once the projections for this tower are found, see if they match
	      with any ACD tiles that are hit.  Info about this is in the
	      TDS, so no work is necessary.
          */

          //findProjections(tower, projections);//**USER CAN WRITE THIS**
          //For now, rather than find all the projections, just read them
          //in from the TDS.

		  //look for a match between the tower
		  const projections *prj = filter->getProjection();

		   firstmatch = ACDProject(tkrGeoSvc,tower, prj);
           //determine whether the first tile match is from TOP or a ROW
           tilematch = evaluateTiles(firstmatch);

		   int xcnt = prj->xy[tower][0];
		   int ycnt = prj->xy[tower][1];
		   numProjections = xcnt+ycnt;
		   totalnumProjections += numProjections;

		  //if (m_energy < 5000){

		  //Here, use FilterAlg_acd.c.  Have to determine where the x
	      //projection starts for this tower.
		  //Loop over the projections in this view
          //get count of projections in this view
          view = 0;
		  prjcnt = prj->xy[tower][view]+prj->xy[tower][view+1];

if (m_useFilterProjecting){
          //count all the projections in towers before this one
          prjstart = 0;
          for (counter = 0; counter < tower; counter++){
             prjstart += prj->xy[counter][0] + prj->xy[counter][1];
          }

		  acd_firstmatch = 0xffffffff;
		  if (prjcnt > 0){
		      acd_firstmatch = FilterAlg_acdProject (prj,prjstart,
                                                     xcnt,ycnt,geo,tower,
                                                     dispatch,
													 m_acd_X,m_acd_Y,m_acd_Front);
			  //if it returns zero (which means no coincidences), set it to the
			  //default value again
			  if (acd_firstmatch == 0x0) acd_firstmatch = 0xffffffff;
		  }
		  else acd_firstmatch = 0xffffffff;

		  if ((unsigned int)acd_firstmatch != 0xffffffff){
		      if (( (acd_firstmatch & 0xf0000000) >> 28) == 4){
		          tilematch = TOPFACE;
		          firstmatch = 88;
		      }
		      else if (acd_firstmatch & 0x3ff){
			      tilematch = ROW01;
			      firstmatch = 88;
		      }
		      else {
			      tilematch = ROW23;
			      firstmatch = 88;
		      }
		  }
		  else {firstmatch = 89; tilematch = 3;}
}
		   if (numProjections == 0) tilematch = 3;//so it doesn't trip ROW23

		   if ( firstmatch < 89 ){//values 0-88 mean there was a tile match
              if ( tilematch == TOPFACE){
                 if (m_energy < 30000) setVeto(m_TKR_TOP_VETO);
              }
              else{
                 if ( tilematch == ROW01 ){
                    if (m_energy < 10000) setVeto(m_TKR_ROW01_VETO);
                 }
                 else if ( tilematch == ROW23 ){
                    if (m_energy < 30000) setVeto(m_TKR_ROW23_VETO);
                 }
              }
              return; //exit the routine
           }
		  //}// if energy

          if (m_energy == 0){
            //Project to the skirt region.  The filter does this, so one can
	        //just trust the filter output.  Or if the user wants to do it
		    //themselves, they can do that.

            //if ( SkirtProject() ) setVeto(m_TKR_SKIRT_VETO);

              //Until SkirtProject() is coded, copy from OnboardFilter:
              if (m_FilterStatus_HI & 0x2) setVeto(m_TKR_SKIRT_VETO);
          }

       }
       while (tmsk);

       //If the filter makes it to this point (no veto), then it does this:
      if ( totalnumProjections < 3 ){
          if (totalnumProjections < 2){
              if (m_energy > 250) return setVeto(m_TKR_EQ_0_VETO);
          }
          //veto commented out: if (m_energy < 350) setVeto(m_TKR_LT_2_ELO_VETO);
      }

   return; //we are completely done filtering

   }//else
   return;
}//latFilter

int FilterAlg::ACDProject(ITkrGeometrySvc *tkrGeoSvc,int tower, const OnboardFilterTds::projections *prj){

    //loop over all the tiles, extending the projections back
	//to the tiles to see if they were hit.
    //Order: top, x-, x+, y-, y+
    double /*dist,*/ tiledim_x, tiledim_y, tiledim_z;
    int face, row, col/*, r_row, r_col, c_row, c_col*/;
	unsigned int acdX_minus = m_acd_X & 0x0000ffff;
	unsigned int acdX_plus  = m_acd_X & 0xffff0000;
	unsigned int acdY_minus = m_acd_Y & 0x0000ffff;
	unsigned int acdY_plus  = m_acd_Y & 0xffff0000;
	unsigned int acdFront = m_acd_Front;
    int bitnum, tile, prjcnt,view;
	int numtiles = cntBits(m_acd_X) + cntBits(m_acd_Y) + cntBits(m_acd_Front);
	int firstmatch = 0;
	HepVector3D newPrj;
	HepPoint3D acdCoord,prjStart,prjStop;

	StatusCode sc = StatusCode::SUCCESS;
	MsgStream log(msgSvc(),name());

	//Loop over all the tiles that are hit
	for (int it=0;it<numtiles;it++){
		//Get the tile and face to look at
		if (acdFront){
			bitnum = 31 - FFS(acdFront);     //gets first tile
			tile = bitnum;                   //set the tile number
			acdFront ^= (int)pow(2.0,bitnum);  //remove the tile from acdFront
			face = 0;
		}
		else if (acdX_minus){
			bitnum = 31 - FFS(acdX_minus);   //gets first tile
			tile = bitnum;              //set the tile number
		    acdX_minus ^= (int)pow(2.0,bitnum);//remove the tile from acdX_minus
			face = 0x1;
		}
		else if (acdX_plus){
			bitnum = 31 - FFS(acdX_plus);    //get the first tile
			tile = bitnum - 16;                   //set the tile number
			acdX_plus ^= (int)pow(2.0,bitnum); //remove the tile from acdX_plus
		    face = 0x3;
		}
		else if (acdY_minus){
			bitnum = 31 - FFS(acdY_minus);   //get the first tile
			tile = bitnum;              //set the tile number
		    acdY_minus ^= (int)pow(2.0,bitnum);//remove the tile from acdY_minus
			face = 0x2;
		}
		else if (acdY_plus){
			bitnum = 31 - FFS(acdY_plus);    //get the first tile
			tile = bitnum - 16;                   //set the tile number
			acdY_plus ^= (int)pow(2.0,bitnum); //remove the tile from acdY_plus
		    face = 0x4;
		}

		//generate a row and column number from the tile and face
		convertId(tile, face, row, col);

    		//create an acdId object for the tile
		    idents::AcdId acdId(0, face, row, col);

			//get the volume id and transform for the tile
            idents::VolumeIdentifier volId = acdId.volId();
            std::string str;
            std::vector<double> dim;
            sc = m_glastDetSvc->getShapeByID(volId, &str, &dim);
            if ( sc.isFailure() ) {
                //log << MSG::WARNING << "Failed to retrieve Shape by Id - probably invalid volId" << endreq;
                return 89;
		    }
            HepTransform3D transform;
            sc = m_glastDetSvc->getTransform3DByID(volId, &transform);
            if (sc.isFailure() ) {
                log << MSG::WARNING << "Failed to get transformation" << endreq;
                return 89;
            }
		    HepPoint3D center(0., 0., 0.);
            HepPoint3D acdCenter = transform * center;

			//get the tile dimensions
			if ((face==0x0)||(face==0x2)||(face==0x4)){
			    tiledim_x = dim[0];
			    tiledim_y = dim[1];
			    tiledim_z = dim[2];
			}
			else if ((face==0x1)||(face==0x3)){
			    tiledim_x = dim[1];//try reversing them
			    tiledim_y = dim[0];
			    tiledim_z = dim[2];
			}

			//determine which view to look at when looking at the row coord
			if (face == 0) //use the y coordinates
				view = 1;
		    else if ((face == 0x1) || (face == 0x3))//use the x coordinates (x view)
			    view = 0;
			else if ((face == 0x2) || (face == 0x4))//use the y coordinates (y view)
			    view = 1;

			//Loop over the projections in this view
			//get count of projections in this view
			prjcnt = prj->xy[tower][view];

            firstmatch = projectionLoop(prjcnt,tile,face,tkrGeoSvc,prj,tower,view,
				                        tiledim_x, tiledim_y, tiledim_z, acdCenter);
			if ((firstmatch != 0) && (firstmatch != 89)) return firstmatch;
	}

	if ((firstmatch != 89)&&(numtiles>0)) return firstmatch;
	else return 89;//means a tile is not hit
}

int FilterAlg::projectionLoop(int prjcnt,int tile,int face,ITkrGeometrySvc *tkrGeoSvc,const projections *prj,
							   int tower,int view, double tiledim_x, double tiledim_y, double tiledim_z
							   ,HepPoint3D acdCenter)
{

double xdist, ydist, zdist, theta, deltaX, deltaY, slope, topHit, midHit,
       check;
float lesserBoundary, greaterBoundary;
bool rowhit = false;
bool colhit = false;
int prjit, prjstart;
HepVector3D newPrj;
HepPoint3D prjStart,acdCoord,prjStop,boundCoord1,boundCoord2,
           checkPoint1,checkPoint2,checkPoint3,checkPoint4;

//count all the projections in towers before this one
int counter;
prjstart = 0;
for (counter = 0; counter < tower; counter++){
    prjstart += prj->xy[counter][0] + prj->xy[counter][1];
}
if (view == 1){
	prjstart += prj->xy[tower][0];
}

//now loop over the projections in this view
for (prjit = prjstart; prjit < (prjcnt + prjstart); prjit++){

	//only consider projections that start in layer 8 or higher!
	if ((prj->prjs[prjit].max >= 8) && (face == 0)){
	   //convert the projection into a vector
	   newPrj = convertPrj(tkrGeoSvc,prj,prjit,tower,view,prjStart);

	   //see where the vector intersects the acd
	   acdCoord = getAcdCoord(newPrj,prjStart, acdCenter, theta, view,face,
		                   tiledim_x, tiledim_y, tiledim_z, boundCoord1, boundCoord2,
						   checkPoint1, checkPoint2, checkPoint3, checkPoint4);

	   //see how far it is from the edge of the acd
	   xdist = fabs(acdCenter.x() - acdCoord.x());
	   ydist = fabs(acdCenter.y() - acdCoord.y());
	   zdist = fabs(acdCenter.z() - acdCoord.z());

	   getTileBoundaries(tile, face, view, lesserBoundary, greaterBoundary);

	   //see if the row was hit by this projection
	   rowhit = false;

if (m_useGleamTileGeometry){
	   if (ydist <= (tiledim_y/2)) rowhit = true;
}
else if (!m_useGleamTileGeometry){
	   if ((acdCoord.y() >= (lesserBoundary))&&(acdCoord.y() <= (greaterBoundary)))
		   rowhit = true;
}

	   if (rowhit) prjit = (prjcnt+prjstart);
    }
	else if (face != 0){
	   //convert the projection into a vector
	   newPrj = convertPrj(tkrGeoSvc,prj,prjit,tower,view,prjStart);

	   //see where the vector intersects the acd
	   acdCoord = getAcdCoord(newPrj,prjStart, acdCenter, theta, view,face,
		                   tiledim_x, tiledim_y, tiledim_z, boundCoord1, boundCoord2,
						   checkPoint1, checkPoint2, checkPoint3, checkPoint4);

	   //see how far it is from the edge of the acd
	   xdist = fabs(acdCenter.x() - acdCoord.x());
	   ydist = fabs(acdCenter.y() - acdCoord.y());
	   zdist = fabs(acdCenter.z() - acdCoord.z());

	   //see if the row was hit by this projection
	   rowhit = false;
if (m_useGleamTileGeometry){
	   if ((face == 0x1) || (face == 0x3)){
	       if (zdist <= (tiledim_z/2)) rowhit = true;
	   }
}
else if (!m_useGleamTileGeometry){
	   if ((acdCoord.z() >= lesserBoundary)&&(acdCoord.z() <= greaterBoundary))
	       rowhit = true;
}

	   //have to do some additional checking to satisfy some conditions that
	   //JJ wrote into the Filter code.  See TFC_acd.c.
	   if (face == 0x1){//we are using view 0 (x) here
	      if ((tower != 0) && (tower != 4) && (tower != 8) && (tower != 12)){
		      deltaX = (prj->prjs[prjit].hits[0] - prj->prjs[prjit].hits[2]);
			  if (deltaX <= 2*1536/3) rowhit = false;
		  }
		  else{
		      //does the slope make sense?
			  topHit = prj->prjs[prjit].hits[0];
			  midHit = prj->prjs[prjit].hits[1];
			  slope = topHit - midHit;
			  if (slope >= 0) rowhit = false;//slope must be negative
			  //does it exit the tower quickly?
			  deltaX = topHit - prj->prjs[prjit].hits[2];
			  check = topHit + deltaX;
			  if (check >= 0) rowhit = false;//check must be less than zero
		  }
	   }
	   else if (face == 0x2){//using view 1 (y)
	      if ((tower != 0) && (tower != 1) && (tower != 2) && (tower != 3)){
	          deltaY = (prj->prjs[prjit].hits[0] - prj->prjs[prjit].hits[2]);
			  if (deltaY <= 2*1536/3) rowhit = false;
		  }
		  else{
		      //does the slope make sense?
			  topHit = prj->prjs[prjit].hits[0];
			  midHit = prj->prjs[prjit].hits[1];
			  slope = topHit - midHit;
			  if (slope >= 0) rowhit = false;//slope must be negative
			  //does it exit the tower quickly?
			  deltaY = topHit - prj->prjs[prjit].hits[2];
			  check = topHit + deltaY;
			  if (check >= 0) rowhit = false;//check must be less than zero
		  }
	   }
	   else if (face == 0x3){//using view 0 (x)
	      if ((tower != 3) && (tower != 7) && (tower != 11) && (tower != 15)){
	          deltaX = (prj->prjs[prjit].hits[0] - prj->prjs[prjit].hits[2]);
	          if (deltaX <= 2*1536/3) rowhit = false;
		  }
		  else{
		      //does the slope make sense?
			  topHit = prj->prjs[prjit].hits[0];
			  midHit = prj->prjs[prjit].hits[1];
			  slope = topHit - midHit;
			  if (slope <= 0) rowhit = false;//slope must be positive
			  //does it exit the tower quickly?
			  deltaX = topHit - prj->prjs[prjit].hits[2];
			  check = topHit + deltaX;
			  if (check <= 1582) rowhit = false;//check must be greater than 1582
		  }
	   }
	   else if (face == 0x4){//using view 1 (y)
		  if ((tower != 12) && (tower != 13) && (tower != 14) && (tower != 15)){
		      deltaY = (prj->prjs[prjit].hits[0] - prj->prjs[prjit].hits[2]);
		      if (deltaY <= 2*1536/3) rowhit = false;
		  }
		  else{
		      //does the slope make sense?
			  topHit = prj->prjs[prjit].hits[0];
			  midHit = prj->prjs[prjit].hits[1];
			  slope = topHit - midHit;
			  if (slope <= 0) rowhit = false;//slope must be positive
			  //does it exit the tower quickly?
			  deltaY = topHit - prj->prjs[prjit].hits[2];
			  check = topHit + deltaY;
			  if (check <= 1582) rowhit = false;//check must be greater than 1582
		  }
	   }

	   if (rowhit) prjit = (prjcnt+prjstart);

	}//else if (face != 0)

}//prj loop for view = 1

//*************************************************************

//determine which view to look at when looking at the col coord
if (face == 0) //use the x coordinates
	view = 0;
else if ((face == 0x1) || (face == 0x3))//use the y coordinates (y view)
    view = 1;
else if ((face == 0x2) || (face == 0x4))//use the x coordinates (x view)
    view = 0;

//Loop over the projections in this view
//get count of projections in this view
prjcnt = prj->xy[tower][view];

//count all the projections in towers before this one
prjstart = 0;
for (counter = 0; counter < tower; counter++){
    prjstart += prj->xy[counter][0] + prj->xy[counter][1];
}
if (view == 1){
	prjstart += prj->xy[tower][0];
}

for (prjit = prjstart; prjit < (prjcnt+prjstart); prjit++){//prj loop for view = 0

	//only consider projections that start in layer 8 or higher!
	if ((prj->prjs[prjit].max >= 8) && (face == 0)){

	   //convert the projection into a vector and see where it intersects the acd
	   newPrj = convertPrj(tkrGeoSvc,prj,prjit,tower,view,prjStart);

	   //see where the vector intersects the acd
	   acdCoord = getAcdCoord(newPrj,prjStart, acdCenter, theta, view,face,
		                   tiledim_x, tiledim_y, tiledim_z, boundCoord1, boundCoord2,
						   checkPoint1, checkPoint2, checkPoint3, checkPoint4);

	   //see how far it is from the edge of the acd
	   xdist = fabs(acdCenter.x() - acdCoord.x());
	   ydist = fabs(acdCenter.y() - acdCoord.y());
	   zdist = fabs(acdCenter.z() - acdCoord.z());

	   getTileBoundaries(tile, face, view, lesserBoundary, greaterBoundary);

	   //see if the tile was hit by this projection
	   colhit = false;
	   if (face == 0){ //use the y coordinates
		   if (m_useGleamTileGeometry){
		       if (xdist <= (tiledim_x/2 /*+ 5*/)) colhit = true;
		   }
		   else if (!m_useGleamTileGeometry){
	           if ((acdCoord.x() >= (lesserBoundary-1))&&(acdCoord.x() <= (greaterBoundary+1)))
			       colhit = true;
		   }
	   }
	   else if ((face == 0x1) || (face == 0x3)){//use the y coordinates (y view)
		   if (m_useGleamTileGeometry){
		       if (ydist <= (tiledim_y/2)) colhit = true;
		   }
		   else if (!m_useGleamTileGeometry){
		       if ((acdCoord.y() >= lesserBoundary)&&(acdCoord.y() <= greaterBoundary))
			       colhit = true;
		   }
	   }
	   else if ((face == 0x2) || (face == 0x4)){//use the x coordinates (x view)
		   if (m_useGleamTileGeometry){
		       if (xdist <= (tiledim_x/2)) colhit = true;
		   }
		   else if (!m_useGleamTileGeometry){
			   if ((acdCoord.x() >= lesserBoundary)&&(acdCoord.x() <= greaterBoundary))
			   colhit = true;
		   }
	   }

	   if (colhit) prjit = (prjcnt+prjstart);//make the loop end

	}
	else if (face != 0){


	   //convert the projection into a vector and see where it intersects the acd
	   newPrj = convertPrj(tkrGeoSvc,prj,prjit,tower,view,prjStart);

	   //see where the vector intersects the acd
	   acdCoord = getAcdCoord(newPrj,prjStart, acdCenter, theta, view,face,
		                   tiledim_x, tiledim_y, tiledim_z, boundCoord1, boundCoord2,
						   checkPoint1, checkPoint2, checkPoint3, checkPoint4);

	   //see if the tile was hit by this projection
	   colhit = false;
/*	   bool X1_Within_Boundaries = ((boundCoord1.x() <= checkPoint1.x() )
		                     &&  (checkPoint1.x() <= boundCoord2.x() ) );
       bool X2_Within_Boundaries = ((boundCoord1.x() <= checkPoint2.x() )
		                     &&  (checkPoint2.x() <= boundCoord2.x() ) );
       bool X3_Within_Boundaries = ((boundCoord1.x() <= checkPoint3.x() )
		                     &&  (checkPoint3.x() <= boundCoord2.x() ) );
       bool X4_Within_Boundaries = ((boundCoord1.x() <= checkPoint4.x() )
		                     &&  (checkPoint4.x() <= boundCoord2.x() ) );
       bool Y1_Within_Boundaries = ((boundCoord1.y() <= checkPoint1.y() )
		                     &&  (checkPoint1.y() <= boundCoord2.y() ) );
       bool Y2_Within_Boundaries = ((boundCoord1.y() <= checkPoint2.y() )
		                     &&  (checkPoint2.y() <= boundCoord2.y() ) );
	   bool Y3_Within_Boundaries = ((boundCoord1.y() <= checkPoint3.y() )
		                     &&  (checkPoint3.y() <= boundCoord2.y() ) );
       bool Y4_Within_Boundaries = ((boundCoord1.y() <= checkPoint4.y() )
		                     &&  (checkPoint4.y() <= boundCoord2.y() ) );
	   bool Z1_Within_Boundaries = ((boundCoord1.z() <= checkPoint1.z() )
		                     &&  (checkPoint1.z() <= boundCoord2.z() ) );
	   bool Z2_Within_Boundaries = ((boundCoord1.z() <= checkPoint2.z() )
		                     &&  (checkPoint2.z() <= boundCoord2.z() ) );
       bool Z3_Within_Boundaries = ((boundCoord1.z() <= checkPoint3.z() )
		                     &&  (checkPoint3.z() <= boundCoord2.z() ) );
	   bool Z4_Within_Boundaries = ((boundCoord1.z() <= checkPoint4.z() )
		                     &&  (checkPoint4.z() <= boundCoord2.z() ) );
*/
	   int X1_Within_Bound=0; if ((boundCoord1.x() <= checkPoint1.x() )
		                     &&  (checkPoint1.x() <= boundCoord2.x() ) )
							 X1_Within_Bound = 1;
       int X2_Within_Bound=0; if ((boundCoord1.x() <= checkPoint2.x() )
		                     &&  (checkPoint2.x() <= boundCoord2.x() ) )
							 X2_Within_Bound = 1;
       int X3_Within_Bound=0; if ((boundCoord1.x() <= checkPoint3.x() )
		                     &&  (checkPoint3.x() <= boundCoord2.x() ) )
							 X3_Within_Bound = 1;
       int X4_Within_Bound=0; if ((boundCoord1.x() <= checkPoint4.x() )
		                     &&  (checkPoint4.x() <= boundCoord2.x() ) )
							 X4_Within_Bound = 1;
       int Y1_Within_Bound=0; if ((boundCoord1.y() <= checkPoint1.y() )
		                     &&  (checkPoint1.y() <= boundCoord2.y() ) )
							 Y1_Within_Bound = 1;
       int Y2_Within_Bound=0; if ((boundCoord1.y() <= checkPoint2.y() )
		                     &&  (checkPoint2.y() <= boundCoord2.y() ) )
							 Y2_Within_Bound = 1;
	   int Y3_Within_Bound=0; if ((boundCoord1.y() <= checkPoint3.y() )
		                     &&  (checkPoint3.y() <= boundCoord2.y() ) )
							 Y3_Within_Bound = 1;
       int Y4_Within_Bound=0; if ((boundCoord1.y() <= checkPoint4.y() )
		                     &&  (checkPoint4.y() <= boundCoord2.y() ) )
							 Y4_Within_Bound = 1;
	   int Z1_Within_Bound=0; if ((boundCoord1.z() <= checkPoint1.z() )
		                     &&  (checkPoint1.z() <= boundCoord2.z() ) )
							 Z1_Within_Bound = 1;
	   int Z2_Within_Bound=0; if ((boundCoord1.z() <= checkPoint2.z() )
		                     &&  (checkPoint2.z() <= boundCoord2.z() ) )
							 Z2_Within_Bound = 1;
       int Z3_Within_Bound=0; if ((boundCoord1.z() <= checkPoint3.z() )
		                     &&  (checkPoint3.z() <= boundCoord2.z() ) )
							 Z3_Within_Bound = 1;
	   int Z4_Within_Bound=0; if ((boundCoord1.z() <= checkPoint4.z() )
		                     &&  (checkPoint4.z() <= boundCoord2.z() ) )
							 Z4_Within_Bound = 1;

	   int total_within = 0;
       if ((face == 0x1) || (face == 0x3)){//use the y coordinates (y view)
		   total_within = Y3_Within_Bound + Y4_Within_Bound
			              + Z1_Within_Bound + Z2_Within_Bound;
		   if (total_within >= 2) colhit = true;
	   }
	   else if ((face == 0x2) || (face == 0x4)){//use the x coordinates (x view)
	   	   total_within = X3_Within_Bound + X4_Within_Bound
			              + Z1_Within_Bound + Z2_Within_Bound;
		   if (total_within >= 2) colhit = true;
	   }
/*
if (m_PrjColMatch)
{
       //NOTE:  JJ does not do this part in his code.  He only checks to see if
	   //it falls within a row, not a column.
	   //Actually, this part may not make any sense at all.  I'll leave it in
	   //the code commented out.

	   if (face == 0x1){//we are using view 1 (y) here
	      if ((tower != 0) && (tower != 4) && (tower != 8) && (tower != 12)){
		      deltaY = fabs(prj->prjs[prjit].hits[2] - prj->prjs[prjit].hits[0]);
			  if (deltaY <= 2*1536/3) colhit = false;
		  }
	   }
	   else if (face == 0x2){//using view 0 (x)
	      if ((tower != 0) && (tower != 1) && (tower != 2) && (tower != 3)){
	          deltaX = fabs(prj->prjs[prjit].hits[2] - prj->prjs[prjit].hits[0]);
		      if (deltaX <= 2*1536/3) colhit = false;
		  }
	   }
	   else if (face == 0x3){//using view 1 (y)
	      if ((tower != 3) && (tower != 7) && (tower != 11) && (tower != 15)){
	          deltaY = fabs(prj->prjs[prjit].hits[2] - prj->prjs[prjit].hits[0]);
	          if (deltaY <= 2*1536/3) colhit = false;
		  }
	   }
	   else if (face == 0x4){//using view 0 (x)
		  if ((tower != 12) && (tower != 13) && (tower != 14) && (tower != 15)){
		      deltaX = fabs(prj->prjs[prjit].hits[2] - prj->prjs[prjit].hits[0]);
		      if (deltaX <= 2*1536/3) colhit = false;
		  }
	   }
}
*/
	   if (colhit) prjit = (prjcnt+prjstart);//make the loop end
	}

}//prj loop

//if a tile is hit, return the tile.
if (!m_PrjColMatch){//no added functionality (checking the columns for a projection match)

if ((face==0)&&(rowhit && colhit))
    return reconstructTileNumber(tile, face);
else if ((face != 0) && (rowhit))
    return reconstructTileNumber(tile, face);

}
else if (m_PrjColMatch){

if (rowhit && colhit) return reconstructTileNumber(tile,face);

}

return 89;//if no matches are found, returns this value, which means...no matches!

}

HepVector3D FilterAlg::convertPrj(ITkrGeometrySvc *tkrGeoSvc, const projections *prj,
								  int prjit, int tower,int view, HepPoint3D &prjStart){
    HepPoint3D prjStop;
	HepVector3D newPrj;

	//get the location of the first hit down from the top
    prjStart = findStripPosition(tkrGeoSvc,tower,prj->prjs[prjit].max,
   				                 view,prj->prjs[prjit].hits[0]);
    //get the location of the top hit
    prjStop = findStripPosition(tkrGeoSvc,tower,prj->prjs[prjit].max-1,
					            view,prj->prjs[prjit].hits[1]);

	//this creates the vector centered in the middle of the tower
	newPrj = prjStart-prjStop;//have the vector pointing up
	newPrj = newPrj/newPrj.mag();//normalize it to a unit vector

	return newPrj;
}

HepPoint3D FilterAlg::getAcdCoord(HepVector3D newPrj,HepPoint3D prjStart, HepPoint3D acdCenter,
								  double &theta, int view,int face,
								  double tiledim_x, double tiledim_y, double tiledim_z,
								  HepPoint3D &boundCoord1, HepPoint3D &boundCoord2,
								  HepPoint3D &checkPoint1, HepPoint3D &checkPoint2,
								  HepPoint3D &checkPoint3, HepPoint3D &checkPoint4){

	HepPoint3D acdCoord;
	double dZ, dX, dY, l, l1, l2, l3, l4, lprime,
		   x, y, z, x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4,
		   bX1, bX2, bY1, bY2, bZ1, bZ2;

	x = newPrj.x();
	y = newPrj.y();
	z = newPrj.z();
	theta = asin(z);
	if (face == 0){//we're looking at the top here
	   dZ = acdCenter.z() - prjStart.z();//?subtract acdZ/2 to account for the thickness of the tile?
	   l = fabs(dZ)/sin(theta);
	   lprime = l*cos(theta);
	   if (view == 1){
	       dY = lprime;
		   dX = 0;
		   if ((y<0)&&(dY>0)) dY *= -1;//make sure dY is negative if it should be!
		   if ((y>0)&&(dY<0)) dY *= -1;//make sure dY is positive if it should be!
	   }
	   else if (view == 0){
	   	   dY = 0;
		   dX = lprime;
		   if ((x<0)&&(dX>0)) dX *= -1;//make sure dX is negative if it should be!
		   if ((x>0)&&(dX<0)) dX *= -1;//make sure dX is positive if it should be!
	   }

	}
	else if ((face==1) || (face==3)){
		if (view == 1){//face = 1 or 3

			bX1 = acdCenter.x() - tiledim_x/2;
			bX2 = acdCenter.x() + tiledim_x/2;
			bY1 = acdCenter.y() - tiledim_y/2;
	        bY2 = acdCenter.y() + tiledim_y/2;
            bZ1 = acdCenter.z() - tiledim_z/2;
			bZ2 = acdCenter.z() + tiledim_z/2;

			boundCoord1 = HepPoint3D(bX1, bY1, bZ1);
			boundCoord2 = HepPoint3D(bX2, bY2, bZ2);

			//calculate the intersection points!
			y1 = prjStart.y() - bY1;
			l1 = y1/cos(theta);
			z1 = l1*sin(theta);// + prjStart.z();
			if ((y<0)&&(y1<0)) z1 = -1*fabs(z1);
			if ((y<0)&&(y1>0)) z1 = fabs(z1);
			if ((y>0)&&(y1<0)) z1 = fabs(z1);
			if ((y>0)&&(y1>0)) z1 = -1*fabs(z1);

			y2 = prjStart.y() - bY2;
			l2 = y2/cos(theta);
		    z2 = l2*sin(theta);// + prjStart.z();
			if ((y<0)&&(y2<0)) z2 = -1*fabs(z2);
			if ((y<0)&&(y2>0)) z2 = fabs(z2);
			if ((y>0)&&(y2<0)) z2 = fabs(z2);
			if ((y>0)&&(y2>0)) z2 = -1*fabs(z2);

			z3 = bZ1 - prjStart.z();//check this!  what if it starts above bZ1?
            l3 = z3/sin(theta);//no fabs(z3) because we want to let l3 be negative
			y3 = l3*cos(theta);
            if ((y<0)&&(z3<0)) y3 = -1*fabs(y3);
			if ((y<0)&&(z3>0)) y3 = fabs(y3);
			if ((y>0)&&(z3>0)) y3 = -1*fabs(y3);
			if ((y>0)&&(z3<0)) y3 = fabs(y3);

			z4 = bZ2 - prjStart.z();
			l4 = z4/sin(theta);
			y4 = l4*cos(theta);
            if ((y<0)&&(z4<0)) y4 = -1*fabs(y4);
			if ((y<0)&&(z4>0)) y4 = fabs(y4);
			if ((y>0)&&(z4>0)) y4 = -1*fabs(y4);
			if ((y>0)&&(z4<0)) y4 = fabs(y4);

            checkPoint1 = HepPoint3D(0, bY1, z1+prjStart.z());
			checkPoint2 = HepPoint3D(0, bY2, z2+prjStart.z());
			checkPoint3 = HepPoint3D(0, y3+prjStart.y(), bZ1);
			checkPoint4 = HepPoint3D(0, y4+prjStart.y(), bZ2);
		}
		else if (view == 0){
		    dX = acdCenter.x() - prjStart.x();
			l=fabs(dX)/cos(theta);
			dZ = l*sin(theta);
			dY = 0;
			if((z<0)&&(dZ>0)) dZ *= -1;
			if((z>0)&&(dZ<0)) dZ *= -1;
		}
	  }
	  else if ((face==2) || (face==4)){
		if (view==0){
			//get the tile boundaries
			bX1 = acdCenter.x() - tiledim_x/2;
			bX2 = acdCenter.x() + tiledim_x/2;
			bY1 = acdCenter.y() - tiledim_y/2;
	        bY2 = acdCenter.y() + tiledim_y/2;
            bZ1 = acdCenter.z() - tiledim_z/2;
			bZ2 = acdCenter.z() + tiledim_z/2;

			boundCoord1 = HepPoint3D(bX1, bY1, bZ1);
			boundCoord2 = HepPoint3D(bX2, bY2, bZ2);

			//get the intersection points
			x1 = prjStart.x()-bX1;
			l1=x1/cos(theta);
		    z1 = l1*sin(theta); //allow z1 to be negative if it wants to be
            if ((x<0)&&(x1<0)) z1 = -1*fabs(z1);
			if ((x<0)&&(x1>0)) z1 = fabs(z1);
			if ((x>0)&&(x1<0)) z1 = fabs(z1);
			if ((x>0)&&(x1>0)) z1 = -1*fabs(z1);

            x2 = prjStart.x() - bX2;
			l2=x2/cos(theta);
		    z2 = l2*sin(theta); //allow z2 to be negative if it wants to be
            if ((x<0)&&(x2<0)) z2 = -1*fabs(z2);
			if ((x<0)&&(x2>0)) z2 = fabs(z2);
			if ((x>0)&&(x2<0)) z2 = fabs(z2);
			if ((x>0)&&(x2>0)) z2 = -1*fabs(z2);

			z3 = bZ1 - prjStart.z();//check this!  what if it starts above bZ1?
            l3 = z3/sin(theta);//not fabs(z3), because we want to let l3 be negative
			x3 = l3*cos(theta);

			z4 = bZ2 - prjStart.z();
			l4 = z4/sin(theta);
			x4 = l4*cos(theta);

			checkPoint1 = HepPoint3D(bX1, 0, z1+prjStart.z());
			checkPoint2 = HepPoint3D(bX2, 0, z2+prjStart.z());
			checkPoint3 = HepPoint3D(x3+prjStart.x(), 0, bZ1);
			checkPoint4 = HepPoint3D(x4+prjStart.x(), 0, bZ2);
		}
		else if (view==1){
		    dX = acdCenter.x() - prjStart.x();
			l=fabs(dX)/cos(theta);
			dZ = l*sin(theta);
			dY = 0;
			if((z<0)&&(dZ>0)) dZ *= -1;
			if((z>0)&&(dZ<0)) dZ *= -1;
		}
	}

	acdCoord = HepPoint3D(dX,dY,dZ) + prjStart;

	return acdCoord;
}

void FilterAlg::getTileBoundaries(int tile, int face, int view,
float &lesserBoundary, float &greaterBoundary){

	int row, col;
	const int id = 0;//the default id, which currently picks out the latest geometry
	const double StripConversion = 0.228;

	const struct _TFC_geometry *geo;
    geo = TFC_geosLocate (TFC_Geos, -1, id);

 	convertId(tile, face, row, col);

	if ( face == 0){
		if ( view == 0 ){
			if    (col == 0){ lesserBoundary  = geo->acd.xTopEdges[0]*StripConversion;
			                  greaterBoundary = geo->acd.xTopEdges[1]*StripConversion;}
			else if (col==1){ lesserBoundary  = geo->acd.xTopEdges[1]*StripConversion;
			                  greaterBoundary = geo->acd.xTopEdges[2]*StripConversion;}
			else if (col==2){ lesserBoundary  = geo->acd.xTopEdges[2]*StripConversion;
			                  greaterBoundary = geo->acd.xTopEdges[3]*StripConversion;}
			else if (col==3){ lesserBoundary  = geo->acd.xTopEdges[3]*StripConversion;
			                  greaterBoundary = geo->acd.xTopEdges[4]*StripConversion;}
			else if (col==4){ lesserBoundary  = geo->acd.xTopEdges[4]*StripConversion;
			                  greaterBoundary = geo->acd.xTopEdges[5]*StripConversion;}
    	}
		else if (view == 1){
			if    (row == 0){ lesserBoundary  = geo->acd.yTopEdges[0]*StripConversion;
			                  greaterBoundary = geo->acd.yTopEdges[1]*StripConversion;}
			else if (row==1){ lesserBoundary  = geo->acd.yTopEdges[1]*StripConversion;
			                  greaterBoundary = geo->acd.yTopEdges[2]*StripConversion;}
			else if (row==2){ lesserBoundary  = geo->acd.yTopEdges[2]*StripConversion;
			                  greaterBoundary = geo->acd.yTopEdges[3]*StripConversion;}
			else if (row==3){ lesserBoundary  = geo->acd.yTopEdges[3]*StripConversion;
			                  greaterBoundary = geo->acd.yTopEdges[4]*StripConversion;}
			else if (row==4){ lesserBoundary  = geo->acd.yTopEdges[4]*StripConversion;
			                  greaterBoundary = geo->acd.yTopEdges[5]*StripConversion;}
		}
	}
	else if (face != 0){
			if      (row==0){ lesserBoundary  = geo->acd.zSides[1]/10.;
			                  greaterBoundary = geo->acd.zSides[0]/10.;}
			else if (row==1){ lesserBoundary  = geo->acd.zSides[2]/10.;
			                  greaterBoundary = geo->acd.zSides[1]/10.;}
			else if (row==2){ lesserBoundary  = geo->acd.zSides[3]/10.;
			                  greaterBoundary = geo->acd.zSides[2]/10.;}
			else if (row==3){ lesserBoundary  = geo->acd.zSides[4]/10.;
			                  greaterBoundary = geo->acd.zSides[3]/10.;}
			else if (row==4){ lesserBoundary  = geo->acd.zSides[5]/10.;
			                  greaterBoundary = geo->acd.zSides[4]/10.;}
	}
	//NOTE: In order to use JJ's tile geometry with my track projection methods, one would
	//      have to add boundaries for the columns in addition to the rows.
	//      These would replace boundCoord1, boundCoord2, boundCoord3, and boundCoord4.

return;
}

HepPoint3D FilterAlg::findStripPosition(ITkrGeometrySvc *tkrGeoSvc,int tower,
                                           int layer, int view, double stripId){
  //stripId is in first ladder
  if(stripId<384)
    return tkrGeoSvc->getStripPosition(tower,layer,view,stripId);

  //stripId is in second ladder
  if(stripId>392 && stripId<777)
    return tkrGeoSvc->getStripPosition(tower,layer,view,stripId-9);

  //stripId is in third ladder
  if(stripId>786 && stripId<1171)
    return tkrGeoSvc->getStripPosition(tower,layer,view,stripId-9-10);

  //stripId is in fourth ladder
  if(stripId>1179)
    return tkrGeoSvc->getStripPosition(tower,layer,view,stripId-9-10-9);

  //stripId is in a gap. We need to compute the position manually
  int below;
  int numstrips;
  //stripId is in first gap
  if(stripId>383 && stripId<393){
    below=383;
    numstrips=10;
  }
  //stripId is in second gap
  if(stripId>776 && stripId<787){
    below=776;
    numstrips=11;
  }
  //stripId is in third gap
  if(stripId>1170 && stripId<1180){
    below=1170;
    numstrips=10;
  }
  HepPoint3D pointBelow=tkrGeoSvc->getStripPosition(tower,layer,view,below);
  HepPoint3D pointAbove=tkrGeoSvc->getStripPosition(tower,layer,view,below+1);
  HepPoint3D incPoint=pointAbove-pointBelow;
  incPoint/=numstrips;
  for(int counter=below;counter<stripId;counter++)
    pointBelow+=incPoint;
  return pointBelow;
}

void FilterAlg::convertId(int tile, int face, int &row, int &col){

	if (face == 0){//front
		if      (tile < 5)  row = 0;
		else if (tile < 10) row = 1;
		else if (tile < 15) row = 2;
		else if (tile < 20) row = 3;
		else if (tile < 25) row = 4;
		col = tile - (5*row);
		return;
	}
		else if ((face == 0x1) || (face == 0x4)){//x minus, y plus
		if      (tile < 5)  row = 0;
		else if (tile < 10) row = 1;
		else if (tile < 15) row = 2;
		else if (tile < 16) row = 3;
		col = tile - (5*row);
		if (row == 4) col = 0;
		return;
	}
	else if ((face == 0x2) || (face == 0x3)){//y minus, x plus
		if      (tile < 5)  row = 0;
		else if (tile < 10) row = 1;
		else if (tile < 15) row = 2;
		else if (tile < 16) row = 3;
		col = tile - (5*row);
		if (row == 4) col = 0;
		return;
	}
	return;
}

void FilterAlg::createTileList(int face, int row, int col, int rowlist[], int collist[]){
	int rowit, colit;
	if (face == 0){
		for (rowit=0;rowit<5;rowit++){//set row list
			rowlist[rowit] = 5*row + rowit;
		}
		for (colit=0;colit<5;colit++){//set column list
		    collist[colit] = col + 5*colit;
		}
		return;
	}
	else if ( (face == 0x1) || (face == 0x4) ){//x minus and y plus faces
		if (row < 3){//set row list
		    for (rowit=0;rowit<5;rowit++){
		        rowlist[rowit] = rowit + 5*row;
		    }
		}
		else{
			for (rowit=0;rowit<5;rowit++){
				rowlist[rowit] = 15;
			}
		}
		for (colit=0;colit<3;colit++){//set column list
		    collist[colit] = (4 - col) + 5*colit;
		}
		collist[3] = 15;
		return;
	}
	else if ( (face == 0x2) || (face == 0x3) ){//y minus and x plus faces
	    if (row < 3){//set row list
		    for (rowit=0;rowit<5;rowit++){
		        rowlist[rowit] = rowit + 5*row;
		    }
		}
		else{
			for (rowit=0;rowit<5;rowit++){
				rowlist[rowit] = 15;
			}
		}
		for (colit=0;colit<3;colit++){//set column list
		    collist[colit] = col + 5*colit;
		}
		collist[3] = 15;
		return;
	}
    return;
}

int FilterAlg::reconstructTileNumber(int tile, short face){
	if      (face == 0)   return tile;       //top face
	else if (face == 0x4) return (tile + 25);//y plus face
	else if (face == 0x2) return (tile + 41);//y minus face (25+16)
	else if (face == 0x3) return (tile + 57);//x plus face (25+16+16)
	else if (face == 0x1) return (tile + 73);//x minus face (25+16+16+16)
	return 89;
}

int FilterAlg::evaluateTiles(int tile){
   const int TOPFACE = 0;
   const int ROW01 = 1;
   const int ROW23 = 2;
   if (tile < 25) return TOPFACE;
   if ( ((tile>=25)&&(tile<=34)) ||
	    ((tile>=41)&&(tile<=50)) ||
		((tile>=57)&&(tile<=66)) ||
		((tile>=73)&&(tile<=82))   )
	    return ROW01;
   else return ROW23;
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

       if (valid_tower >=0) start_layer = valid_tower;

	   if (valid_tower>=0){
	       if ((m_acd_Front & m_maskTop) && (start_layer > 14)){
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

/*
	Into TDS: xy00, xy11, xy22, xy33 from line 814 and following
	Into root file: Have to do some manipulations.  xy1 = (xy00 & xy11 & xy 22), xy2 = (xy00 & xy11 & xy33).
	xy1 has bits that represent coincidences that begin in odd or even layers, depending on whether one is
	looking for 2 bi-layer coincidences or 3 bi-layer coincidences:
	  bit position	 8  7  6  5  4  3  2  1  0
	  value		     1  3  5  7  9  11 13 15 17 when looking at 2 bi-layer coincidences
	  value          2  4  6  8  10 12 14 16 -- when looking at 3 bi-layer coincidences

	xy2 has bits that represent coincidences that begin in even or odd layers, depending on whether one is
	looking for 2 bi-layer coincidences or 3 bi-layer coincidences:
	  bit position	 8  7  6  5  4  3  2  1  0
	  value		     2  4  6  8  10 12 14 16 -- when looking at 2 bi-layer coincidences
	                 1  3  5  7  9  11 13 15 17 when looking at 3 bi-layer coincidences

        We need to form a new word that merges these bits like this:
          bit position 10  F  E  D  C  B  A 9 8 7 6 5 4 3 2 1 0
          value	       16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
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

