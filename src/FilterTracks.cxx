#include "OnboardFilter/FilterTracks.h"

static const AlgFactory<FilterTracks>  Factory;
const IAlgFactory& FilterTracksFactory = Factory;

FilterTracks::FilterTracks(const std::string& name, ISvcLocator* pSvcLocator)
:Algorithm(name, pSvcLocator){

    declareProperty("UseNumHits",     m_usenumhits = 0);
	declareProperty("WriteHits",      m_writehits = 0);
	declareProperty("Scattering",     m_scattering = 0);
	declareProperty("TrackSelect",    m_trackselect = 1);

}

StatusCode FilterTracks::initialize(){
    MsgStream log(msgSvc(),name());
    log<<MSG::DEBUG<<"Initializing"<<endreq;
    m_x.push_back(0);
    m_x.push_back(0);
    m_x.push_back(0);
    m_y=m_xz=m_yz=m_zAvg=m_pointHigh=
      m_extendHigh=m_extendLow=m_x;

    if (m_writehits){
	    m_outfile.open("hits.txt");
        if (m_outfile.is_open()){
			m_outfile << "Event\tTower\tView\tLayer\tStripId\tX Coord\tY Coord\tZ Coord"<<std::endl;
			m_outfile << std::endl;
		}
	}

    return StatusCode::SUCCESS;
}

StatusCode FilterTracks::finalize(){
  MsgStream log(msgSvc(),name());
  log<<MSG::DEBUG<<"Finalizing"<<endreq;

  if (m_writehits){
    m_outfile.close();
  }

  return StatusCode::SUCCESS;
}

StatusCode FilterTracks::execute(){
  MsgStream log(msgSvc(),name());
  log<<MSG::DEBUG<<"execute()"<<endreq;
  ITkrGeometrySvc *tkrGeoSvc=NULL;
  if(service("TkrGeometrySvc",tkrGeoSvc,true).isFailure()){
    log<<MSG::ERROR<<"Couldn't set up TkrGeometrySvc!"<<endreq;
    return StatusCode::FAILURE;
  }
  SmartDataPtr<OnboardFilterTds::FilterStatus> status(eventSvc(),
                                                      "/Event/Filter/FilterStatus");
  if(!status){
    log<<MSG::ERROR<<"Unable to retrieve FilterStatus from TDS"<<endreq;
    return StatusCode::FAILURE;
  }
  //Loop over the towers

  const OnboardFilterTds::projections *prjs=status->getProjection();
  int startPrj=0;
  for(int tower=0;tower<16;tower++){
    HepPoint3D point;
    if(prjs->xy[tower][0]>0 && prjs->xy[tower][1]>0){
      //Loop over the x projections
      for(int xprj=startPrj;xprj<prjs->xy[tower][0];xprj++){

        log<<MSG::DEBUG<<"Obtaining X and XZ for hits 0 and 1"<<endreq;
        point=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max,0,
                                prjs->prjs[xprj].hits[0]);
        m_x[0]=point.x();
        m_xz[0]=point.z();
        point=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max-1,0,
                                prjs->prjs[xprj].hits[1]);
        m_x[1]=point.x();
        m_xz[1]=point.z();
        //Loop over the y projections
        for(int yprj=startPrj+prjs->xy[tower][0];yprj<startPrj+prjs->xy[tower][1]+prjs->xy[tower][0];yprj++){

          if(prjs->prjs[xprj].max==prjs->prjs[yprj].max){//if they start in the same layer
            log<<MSG::DEBUG<<"Obtaining Y and YZ for hits 0 and 1"
               << endreq;
            point=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max,
                                    1,prjs->prjs[yprj].hits[0]);
            m_y[0]=point.y();
            m_yz[0]=point.z();
            point=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max-1,
                                    1,prjs->prjs[yprj].hits[1]);
            m_y[1]=point.y();
            m_yz[1]=point.z();
            unsigned char maxhits;
            if(prjs->prjs[xprj].nhits<prjs->prjs[yprj].nhits)//they don't need the same number
              maxhits=prjs->prjs[xprj].nhits;              //of layers.  use the smaller number
            else                                             //of hits.
              maxhits=prjs->prjs[yprj].nhits;
            log << MSG::DEBUG << "Obtaining X,Y,XZ,YZ for max hits"<<endreq;
            point=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max-(maxhits-1),
                                    0,prjs->prjs[xprj].hits[maxhits-1]);
            m_x[2]=point.x();
            m_xz[2]=point.z();
            point=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max-(maxhits-1),
                                    1,prjs->prjs[yprj].hits[maxhits-1]);
            m_y[2]=point.y();
            m_yz[2]=point.z();
            for(int counter=0;counter<3;counter++)
              m_zAvg[counter]=(m_xz[counter]+m_yz[counter])/2;
            computeAngles(m_x[1]-m_x[0], m_xz[0]-m_xz[1], m_y[1]-m_y[0], 
						  m_yz[0]-m_yz[1], m_zAvg[0]-m_zAvg[1]);
            computeLength();
            computeExtension();
            //Add track to TDS
            OnboardFilterTds::track newTrack;
            newTrack.phi_rad=m_phi_rad;
            newTrack.theta_rad=m_theta_rad;
            newTrack.lowCoord.push_back(m_x[0]);
            newTrack.lowCoord.push_back(m_y[0]);
            newTrack.lowCoord.push_back(m_zAvg[0]);
            newTrack.highCoord=m_pointHigh;
            newTrack.exLowCoord=m_extendLow;
            newTrack.exHighCoord=m_extendHigh;
            newTrack.length=m_length;
            newTrack.numhits=maxhits;
            status->setTrack(newTrack);
          }
        }
      }
    }
    startPrj+=prjs->curCnt[tower];//I moved this 06/14/04 - DW
  }
  std::vector<OnboardFilterTds::track> tracks=status->getTracks();
  double maxLength=0;
  int   maxnumhits=0;
  unsigned int currMax,currMaxHits;
  log<<MSG::DEBUG<<"Processing "<<tracks.size()<<" tracks"<<endreq;
  if(tracks.size()>0){
    for(unsigned int counter=0;counter<tracks.size();counter++){//currently using the track that is the
      if(tracks[counter].length>maxLength){                     //longest, not the track with the most
	maxLength=tracks[counter].length;                           //hits.  Usually, these will be the same,
	currMax=counter;                                            //but it is possible to have a long track
      }                                                         //at a shallow angle with only 3 hits,
    }                                                           //and a short track with more than this.
    for(unsigned int counter2=0;counter2<tracks.size();counter2++){//This is an alternative method to picking
      if(tracks[counter2].numhits>maxnumhits){                     //the track to use.  This one picks out
	maxnumhits=tracks[counter2].numhits;                           //the track that has the most hits, not
	currMaxHits=counter2;                                          //the longest track
      }
    }
    if (m_usenumhits){      //if the jobOptions say to use the number of hits to pick out the track
      currMax=currMaxHits;//this is executed and currMax is replaced with a different track selector
    }
    log<<MSG::DEBUG<<"Using track "<<currMax<<" out of "<<tracks.size()<<endreq;
    //Obtain McZDir, McXDir, McYDir (copied from McValsTools.cxx)
    SmartDataPtr<Event::McParticleCol> pMcParticle(eventSvc(),EventModel::MC::McParticleCol);
    if(pMcParticle){
      Event::McParticleCol::const_iterator pMCTrack1 = pMcParticle->begin();
      HepLorentzVector Mc_p0 = (*pMCTrack1)->initialFourMomentum();
      Vector Mc_t0 = Vector(Mc_p0.x(),Mc_p0.y(), Mc_p0.z()).unit();
      double MC_xdir,MC_ydir,MC_zdir;
      MC_xdir   = Mc_t0.x();
      MC_ydir   = Mc_t0.y();
      MC_zdir   = Mc_t0.z();
      //Convert MC dir into theta and phi
      double phi_rad_mc=0;
      double theta_rad_mc=acos(-MC_zdir);
      if(MC_zdir < 0)
	  theta_rad_mc=M_PI - acos(-MC_zdir);
      double x_cord=-MC_xdir;
      double y_cord=-MC_ydir;
      if(y_cord==0)
	      phi_rad_mc=M_PI/2;
      if ((x_cord>0)&&(y_cord>0)){
	      phi_rad_mc = atan(y_cord/x_cord);
	      if(MC_zdir<0)
	          phi_rad_mc = 3*M_PI/2 - atan(x_cord/y_cord);
      }
      if ((x_cord<0)&&(y_cord<0)){
	      phi_rad_mc = 3*M_PI/2 - atan(x_cord/y_cord);
	      if(MC_zdir<0)
	          phi_rad_mc = atan(y_cord/x_cord);
      }
      if ((x_cord>0)&&(y_cord<0)){
	      phi_rad_mc = 2*M_PI-atan(-y_cord/x_cord);
	      if(MC_zdir<0)
	          phi_rad_mc=M_PI/2 - atan(x_cord/y_cord);
      }
      if ((x_cord<0)&&(y_cord>0)){
	      phi_rad_mc = M_PI/2 - atan(y_cord/(-x_cord));
	      if(MC_zdir<0)
	          phi_rad_mc = 2*M_PI-atan(-y_cord/x_cord);
      }
      //Compute angular separation
      double xone=sin(theta_rad_mc)*cos(phi_rad_mc);
      double yone=sin(theta_rad_mc)*sin(phi_rad_mc);
      double zone=cos(theta_rad_mc);
      double xtwo=sin(tracks[currMax].theta_rad)*cos(tracks[currMax].phi_rad);
      double ytwo=sin(tracks[currMax].theta_rad)*sin(tracks[currMax].phi_rad);
      double ztwo=cos(tracks[currMax].theta_rad);
      double dot_product=xone*xtwo+yone*ytwo+zone*ztwo;
      double separation_rad=acos(dot_product);
      //if (separation_rad > M_PI/2.)
      //      separation_rad = M_PI - separation_rad; //we always assume that events come from "above"
      status->setSeparation(separation_rad*180/M_PI);
    }
    else{
      status->setSeparation(-1);
      log<<MSG::INFO <<"Unable to obtain McParticleCol from TDS"<<endreq;
    }
  }
  else{
    status->setSeparation(-1);
    log<<MSG::DEBUG<<"No tracks found"<<endreq;
  }

  StatusCode sc;
  if (m_scattering){
    sc = MultipleScattering();
  }
  if (m_writehits){
    sc = WriteHits();
  }
  if (m_trackselect){
  	sc = TrackSelect();
  }

  return StatusCode::SUCCESS;
}

void FilterTracks::computeAngles(double x_h, double x_v, double y_h, double y_v, double z_v){
    //double x_h = m_x[1]-m_x[0];
    //double x_v = m_xz[0]-m_xz[1];
    //double y_h = m_y[1]-m_y[0];
    //double y_v = m_yz[0]-m_yz[1];
    //double z_v = m_zAvg[0]-m_zAvg[1];
  double t_h_ave = 1;
  if(x_h == 0 && y_h==0){
    m_phi_rad=0;
    m_theta_rad=0;
  }
  else{
    if(x_h ==0){
      m_theta_rad=M_PI/2-atan(y_v/y_h);
      if(y_h>0)
        m_phi_rad=M_PI/2;
      else
        m_phi_rad=3*M_PI/2;
    }
    else{
      if(y_h==0){
        m_theta_rad=M_PI/2-atan(x_v/x_h);
        if(x_h>0)
          m_phi_rad=0;
        else
          m_phi_rad=M_PI;
      }
      else{
        if((x_h>0) && (y_h>0)){
          m_phi_rad=atan(y_h/x_h);
          t_h_ave=((x_h/cos(m_phi_rad)) + (y_h/sin(m_phi_rad)))/2;
          m_theta_rad = M_PI - atan(t_h_ave/z_v);
        }
        else{
          if((x_h<0) && (y_h>0)){
            m_phi_rad=M_PI/2-atan(x_h/y_h);
            t_h_ave = ( -x_h/sin(m_phi_rad - M_PI/2) + y_h/cos(m_phi_rad - M_PI/2) )/2;
            m_theta_rad = M_PI - atan(t_h_ave/z_v);
          }
          else{
            if((x_h<0) && (y_h<0)){
              m_phi_rad=3*M_PI/2 - atan(x_h/y_h);
              t_h_ave = ( (-x_h/sin(3*M_PI/2 - m_phi_rad)) + (-y_h/cos(3*M_PI/2 - m_phi_rad)) )/2;
              m_theta_rad = M_PI - atan(t_h_ave/z_v);
            }
            else{
              if ((x_h>0) && (y_h<0)){
                m_phi_rad=2*M_PI-atan(-y_h/x_h);
                t_h_ave = ((x_h/cos(2*M_PI-m_phi_rad)) + (-y_h/sin(2*M_PI-m_phi_rad)))/2;
                m_theta_rad = M_PI - atan(t_h_ave/z_v);
              }
              else
                m_phi_rad=3*M_PI/2-atan(y_h/x_h);
            }
          }
        }
      }
    }
  }
  m_theta=m_theta_rad*180/M_PI;
  m_phi=m_phi_rad*180/M_PI;
}

void FilterTracks::computeLength(){
  double t_v=m_zAvg[0]-m_zAvg[2];
  double t_h = t_v*tan(M_PI - m_theta_rad);
  m_length=sqrt(t_v*t_v+t_h*t_h);
  m_pointHigh[0]=t_h*cos(m_phi_rad) + m_x[0];
  m_pointHigh[1]=t_h*sin(m_phi_rad) + m_y[0];
  m_pointHigh[2]=m_zAvg[2];
}

void FilterTracks::computeExtension(){
  const double length=1000;
  m_extendLow[0]=length*sin(M_PI-m_theta_rad)*cos(M_PI+m_phi_rad) + m_x[0];
  m_extendLow[1]=length*sin(M_PI-m_theta_rad)*sin(M_PI+m_phi_rad) + m_y[0];
  m_extendLow[2]=length*cos(M_PI-m_theta_rad) + m_zAvg[0];

  m_extendHigh[0]=length*sin(m_theta_rad)*cos(m_phi_rad)+m_x[2];
  m_extendHigh[1]=length*sin(m_theta_rad)*sin(m_phi_rad)+m_y[2];
  m_extendHigh[2]=length*cos(m_theta_rad)+m_zAvg[2];
}

HepPoint3D FilterTracks::findStripPosition(ITkrGeometrySvc *tkrGeoSvc,int tower,
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

StatusCode FilterTracks::MultipleScattering(){
  MsgStream log(msgSvc(),name());
  //get geometry access
  ITkrGeometrySvc *tkrGeoSvc=NULL;
  if(service("TkrGeometrySvc",tkrGeoSvc,true).isFailure()){
    log<<MSG::ERROR<<"Couldn't set up TkrGeometrySvc!"<<endreq;
    return StatusCode::SUCCESS;
  }

  SmartDataPtr<OnboardFilterTds::FilterStatus> status(eventSvc(),
                                                      "/Event/Filter/FilterStatus");
  if(!status){
    log<<MSG::ERROR<<"Unable to retrieve FilterStatus from TDS"<<endreq;
    return StatusCode::SUCCESS;
  }

//Loop over the towers

      const OnboardFilterTds::projections *prjs=status->getProjection();
      double startPrj=0;

      double old_longest_x = 0;
	  double old_longest_y = 0;
	  double disp_one_t, disp_one_b, disp_two_t, disp_two_b, proj_two_b, proj_two_t,
		  bottomhit, tophit, xcompare, xsum_comparison, longest_xcompare, long_firstanglex,
		  ycompare, ysum_comparison, longest_ycompare, xmax, xmin, long_firstangley,
		  ymax, ymin, xcompareBottom, ycompareBottom, longest_xcompareBottom, longest_ycompareBottom;
	  int longest_xprj, longest_yprj, tower;
	  int xcount = 0;
	  int ycount = 0;
	  double xprjlength = 0;
	  double yprjlength = 0;
	  double xavg_compare= -2000;
	  double yavg_compare= -2000;;
	  double yoverall_sum=0;
	  double xoverall_sum=0;
	  double yoverall_count=0;
	  double xoverall_count=0;
	  long_firstanglex = -2000;
	  long_firstangley = -2000;
	  xsum_comparison=0;
	  ysum_comparison=0;

	  double xcomparisons[10];
	  double ycomparisons[10];

	  double xslope = -2000;
	  double xslopeL = -2000;
	  double xslopeTotal = 0;
	  double yslope = -2000;
	  double yslopeL = -2000;
	  double yslopeTotal = 0;
	  double xslopeAvg = -2000;
	  double yslopeAvg = -2000;
	  int xslopeCount = 0;
	  int yslopeCount = 0;
	  double xslopetower=-1;
	  double yslopetower=-1;

	  HepPoint3D point1, point2, point3, point4;
	  double zdist1, zdist2, angle1, angle2, xangleL, yangleL;
	  xangleL = -2000;
	  yangleL = -2000;

	  for (int n=0; n<10;n++){
	      xcomparisons[n] = -2000;
		  ycomparisons[n] = -2000;
	  }

      xcount=0;
	  ycount=0;
	  xprjlength = 0;
	  yprjlength = 0;
      longest_xprj = 0;
	  longest_yprj = 0;

	  for(tower=0;tower<16;tower++){
          //loop over projections regardless of whether there are both x and y
          int xhits=0;
		  int yhits=0;

		  //if there are any x projections
		  if(prjs->xy[tower][0]>0){
             //loop over the x projections
			  for(int xprj=startPrj;xprj<prjs->xy[tower][0];xprj++){

				  xhits=prjs->prjs[xprj].nhits;
				    //find the longest x projection
			        if (xhits > xprjlength){
			    	  longest_xprj = xprj;
			          xprjlength = xhits;
					  xslopeL = prjs->prjs[xprj].hits[1] - prjs->prjs[xprj].hits[0];

					  point1=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max,0,
                                                            prjs->prjs[xprj].hits[0]);
					  point2=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max-1,0,
                                                            prjs->prjs[xprj].hits[1]);
					  disp_one_t = point2.x() - point1.x();//the x displacement
					  zdist1 = point1.z() - point2.z();    //the z distance
					  xangleL= 180/3.14159*atan(disp_one_t/zdist1);//the angle from LAT 90 deg

					  xslopetower = tower;
			        }

					xslope = prjs->prjs[xprj].hits[1] - prjs->prjs[xprj].hits[0];
					xslopeTotal += xslope;
					xslopeCount++;

				  if (xhits > 3){ //if there are at least 4 hits
				      xmax=prjs->prjs[xprj].max;
					  xmin=prjs->prjs[xprj].min;
					  tophit = prjs->prjs[xprj].hits[0];
					  bottomhit = prjs->prjs[xprj].hits[xhits-1];

					  //Calculate the first displacement
                      //disp_one_t = prjs->prjs[xprj].hits[1] - tophit;
					  //changing this to be in units of mm instead of strips
					  point1=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max,0,
                                              prjs->prjs[xprj].hits[0]);
					  point2=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max-1,0,
                                                  prjs->prjs[xprj].hits[1]);
					  disp_one_t = point2.x() - point1.x();//the x displacement
					  zdist1 = point1.z() - point2.z();    //the z distance
					  angle1=180/3.14159*atan(disp_one_t/zdist1);//the angle

					  //Calculate the projected displacement to the next level
					  //proj_two_t = prjs->prjs[xprj].hits[1] + disp_one_t;
					  point3=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max-2,0,
                                                  prjs->prjs[xprj].hits[0]);
					  zdist2 = point2.z() - point3.z();
					  proj_two_t = point2.x() + zdist2*tan(angle1/180*3.14159);

					  //Calculate the second displacement - the "angle"
					  //disp_two_t = abs(prjs->prjs[xprj].hits[2] - proj_two_t);
                      point4=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max-2,0,
                                                  prjs->prjs[xprj].hits[2]);
					  disp_two_t = point4.x() - point2.x();
					  angle2=180/3.14159*atan(disp_two_t/zdist2);
					  disp_two_t = fabs(angle2-angle1);

                          //Calculate the next displacement (going up from the bottom this time)
					  	  //Call it disp_one_b, where "b" is for "bottom."
					  	  //disp_one_b = prjs->prjs[xprj].hits[2] - prjs->prjs[xprj].hits[1];
					  	  point1=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max-1,0,
                                                    prjs->prjs[xprj].hits[1]);
					  	  point2=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max-2,0,
                                                        prjs->prjs[xprj].hits[2]);
					  	  disp_one_b = point2.x() - point1.x();
					  	  zdist1 = point1.z() - point2.z();
					  	  angle1=180/3.14159*atan(disp_one_b/zdist1);//the angle

					  	  //Calculate the projected displacement to the next level
					  	  //proj_two_b = prjs->prjs[xprj].hits[(xhits-1)-1] + disp_one_b;
					  	  //proj_two_b = prjs->prjs[xprj].hits[2] + disp_one_b;
                            point3=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max-3,0,
                                                        prjs->prjs[xprj].hits[1]);
					  	  zdist2 = point2.z() - point3.z();
					  	  proj_two_t = point2.x() + zdist2*tan(angle1/180*3.14159);

					  	  //Calculate the last displacement that we need (an "angle")
					  	  //disp_two_b = abs(prjs->prjs[xprj].hits[(xhits-1)] - proj_two_b);
					  	  //disp_two_b = abs(prjs->prjs[xprj].hits[3] - proj_two_b);
                            point4=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max-3,0,
                                                        prjs->prjs[xprj].hits[3]);
					  	  disp_two_b = point4.x() - point2.x();
					  	  angle2=180/3.14159*atan(disp_two_b/zdist2);
					  	  disp_two_b = fabs(angle2-angle1);

				            //Compare the two.  This is what we write out.
					  	  xcompare = disp_two_b - disp_two_t;

					      //**************************************************************
					      //Or if we use the bottommost hits:

					      //Calculate the next displacement (going up from the bottom this time)
						  //Call it disp_one_b, where "b" is for "bottom."
						  //disp_one_b = prjs->prjs[xprj].hits[(xhits-1)-1] - prjs->prjs[xprj].hits[(xhits-1)-2];
						  point1=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].min+2,0,
						                              prjs->prjs[xprj].hits[(xhits-1)-2]);
						  point2=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].min+1,0,
						                                  prjs->prjs[xprj].hits[(xhits-1)-1]);
						  disp_one_b = point2.x() - point1.x();
						  zdist1 = point1.z() - point2.z();
						  angle1=180/3.14159*atan(disp_one_b/zdist1);//the angle

						  //Calculate the projected displacement to the next level
						  //proj_two_b = prjs->prjs[xprj].hits[(xhits-1)-1] + disp_one_b;
						  //proj_two_b = prjs->prjs[xprj].hits[2] + disp_one_b;
						      point3=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].min,0,
						                                  prjs->prjs[xprj].hits[(xhits-1)]);
						  zdist2 = point2.z() - point3.z();
						  proj_two_t = point2.x() + zdist2*tan(angle1/180*3.14159);

						  //Calculate the last displacement that we need (an "angle")
						  //disp_two_b = abs(prjs->prjs[xprj].hits[(xhits-1)] - proj_two_b);
						  //disp_two_b = abs(prjs->prjs[xprj].hits[3] - proj_two_b);
						      point4=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].min,0,
						                                  prjs->prjs[xprj].hits[(xhits-1)]);
						  disp_two_b = point4.x() - point2.x();
						  angle2=180/3.14159*atan(disp_two_b/zdist2);
						  disp_two_b = fabs(angle2-angle1);

						    //Compare the two.  This is what we write out.
					  	  xcompareBottom = disp_two_b - disp_two_t;


					  //save the first 10 that we come to
					  if (xcount < 10){
					  xcomparisons[xcount] = xcompare;
					  }

					  //keep track of the longest one for this tower
					  if (xprj == longest_xprj){
						  longest_xcompare = xcompare;
						  longest_xcompareBottom = xcompareBottom;
						  long_firstanglex = disp_two_t;
					  }

					  //sum up the differences so we can average them later
					  xsum_comparison += xcompare;
					  xcount++;
				  }

			  }
              //xavg_compare = (float)xsum_comparison/(float)xcount;
          }

		  if(prjs->xy[tower][1]>0){//if there are any y projections
		      //loop over the y projections
			  for(int yprj=startPrj+prjs->xy[tower][0];yprj<startPrj+prjs->xy[tower][1]+prjs->xy[tower][0];yprj++){
			      ymax=prjs->prjs[yprj].max;
				  ymin=prjs->prjs[yprj].min;

				  yhits=prjs->prjs[yprj].nhits;
				    //find the longest x projection
			        if (yhits > yprjlength){
			    	  longest_yprj = yprj;
			          yprjlength = yhits;
			          yslopeL = prjs->prjs[yprj].hits[1] - prjs->prjs[yprj].hits[0];

					  point1=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max,1,
                                              prjs->prjs[yprj].hits[0]);
					  point2=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max-1,1,
                                                  prjs->prjs[yprj].hits[1]);
					  disp_one_t = point2.y() - point1.y();//the x displacement
					  zdist1 = point1.z() - point2.z();    //the z distance
					  yangleL=180/3.14159*atan(disp_one_t/zdist1);//the angle

					  yslopetower=tower;
			        }

					yslope = prjs->prjs[yprj].hits[1] - prjs->prjs[yprj].hits[0];
					yslopeTotal += yslope;
					yslopeCount++;

				  if (yhits > 3){ //if there are at least 4 hits
				      tophit = prjs->prjs[yprj].hits[0];
					  bottomhit = prjs->prjs[yprj].hits[yhits-1];

					  //Calculate the first displacement
                      //disp_one_t = prjs->prjs[yprj].hits[1] - tophit;
					  //changing this to be in units of mm instead of strips
					  point1=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max,1,
                                              prjs->prjs[yprj].hits[0]);
					  point2=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max-1,1,
                                                  prjs->prjs[yprj].hits[1]);
					  disp_one_t = point2.y() - point1.y();//the x displacement
					  zdist1 = point1.z() - point2.z();    //the z distance
					  angle1=180/3.14159*atan(disp_one_t/zdist1);//the angle

					  //Calculate the projected displacement to the next level
					  //proj_two_t = prjs->prjs[yprj].hits[1] + disp_one_t;
					  point3=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max-2,1,
                                                  prjs->prjs[yprj].hits[0]);
					  zdist2 = point2.z() - point3.z();
					  proj_two_t = point2.y() + zdist2*tan(angle1/180*3.14159);

					  //Calculate the second displacement - the "angle"
					  //disp_two_t = abs(prjs->prjs[yprj].hits[2] - proj_two_t);
					  point4=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max-2,1,
                                                  prjs->prjs[yprj].hits[2]);
					  disp_two_t = point4.y() - point2.y();
					  angle2=180/3.14159*atan(disp_two_t/zdist2);
					  disp_two_t = fabs(angle2-angle1);

					  	  //Calculate the next displacement (going up from the bottom this time)
					  	  //Call it disp_one_b, where "b" is for "bottom."
					  	  //disp_one_b = prjs->prjs[yprj].hits[(yhits-1)-1] - prjs->prjs[yprj].hits[(yhits-1)-2];
                            //disp_one_b = prjs->prjs[yprj].hits[2] - prjs->prjs[yprj].hits[1];
                            point1=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max-1,1,
                                                    prjs->prjs[yprj].hits[1]);
					  	  point2=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max-2,1,
                                                        prjs->prjs[yprj].hits[2]);
					  	  disp_one_b = point2.y() - point1.y();
					  	  zdist1 = point1.z() - point2.z();
					  	  angle1=180/3.14159*atan(disp_one_t/zdist1);//the angle

					  	  //Calculate the projected displacement to the next level
					  	  //proj_two_b = prjs->prjs[yprj].hits[(yhits-1)-1] + disp_one_b;
					  	  //proj_two_b = prjs->prjs[yprj].hits[2] + disp_one_b;
                            point3=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max-3,1,
                                                        prjs->prjs[yprj].hits[1]);
					  	  zdist2 = point2.z() - point3.z();
					  	  proj_two_t = point2.y() + zdist2*tan(angle1/180*3.14159);

					  	  //Calculate the last displacement that we need (an "angle")
					  	  //disp_two_b = abs(prjs->prjs[yprj].hits[(yhits-1)] - proj_two_b);
					  	  //disp_two_b = abs(prjs->prjs[yprj].hits[3] - proj_two_b);
                            point4=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max-3,1,
                                                        prjs->prjs[yprj].hits[3]);
					  	  disp_two_b = point4.y() - point2.y();
					  	  angle2=180/3.14159*atan(disp_two_b/zdist2);
					  	  disp_two_b = fabs(angle2-angle1);

				            //Compare the two
					  	  ycompare = disp_two_b - disp_two_t;

					  	  //**************************************************************
					  		//Or if we use the bottommost hits:

					  		//Calculate the next displacement (going up from the bottom this time)
					  		//Call it disp_one_b, where "b" is for "bottom."
					  		//disp_one_b = prjs->prjs[yprj].hits[(yhits-1)-1] - prjs->prjs[yprj].hits[(yhits-1)-2];
					  		point1=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].min+2,1,
					  		                            prjs->prjs[yprj].hits[(yhits-1)-2]);
					  		point2=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].min+1,1,
					  		                                prjs->prjs[yprj].hits[(yhits-1)-1]);
					  		disp_one_b = point2.y() - point1.y();
					  		zdist1 = point1.z() - point2.z();
					  		angle1=180/3.14159*atan(disp_one_b/zdist1);//the angle

					  		//Calculate the projected displacement to the next level
					  		//proj_two_b = prjs->prjs[yprj].hits[(yhits-1)-1] + disp_one_b;
					  		//proj_two_b = prjs->prjs[yprj].hits[2] + disp_one_b;
					  		    point3=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].min,1,
					  		                                prjs->prjs[yprj].hits[(yhits-1)]);
					  		zdist2 = point2.z() - point3.z();
					  		proj_two_t = point2.y() + zdist2*tan(angle1/180*3.14159);

					  		//Calculate the last displacement that we need (an "angle")
					  		//disp_two_b = abs(prjs->prjs[yprj].hits[(yhits-1)] - proj_two_b);
					  		//disp_two_b = abs(prjs->prjs[yprj].hits[3] - proj_two_b);
					  		    point4=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].min,0,
					  		                                prjs->prjs[yprj].hits[(yhits-1)]);
					  		disp_two_b = point4.y() - point2.y();
					  		angle2=180/3.14159*atan(disp_two_b/zdist2);
					  		disp_two_b = fabs(angle2-angle1);

					  	  //Compare the two.  This is what we write out.
					  	  ycompareBottom = disp_two_b - disp_two_t;

					  //save the first 10 that we come to
					  if (ycount <10){
					  ycomparisons[ycount] = ycompare;
					  }

					  //keep track of the longest one for this tower
					  if (yprj == longest_yprj){
						  longest_ycompare = ycompare;
						  longest_ycompareBottom = ycompareBottom;
						  long_firstangley = disp_two_t;
					  }

					  //sum up the differences so we can average them later
					  ysum_comparison += ycompare;
					  ycount++;
				  }
			  }
              //yavg_compare = (float)ysum_comparison/(float)ycount;
			  }
		  }

          //Have now looped over all x and all y projections for this tower
		  //Keep track of the difference for the longest track in any tower,
		  //and the average for all towers.

		  //this finds the longest of all the tracks in any tower.
		  //this projection may or may not be in the list of 10

		  if (xcount > 0){
		      //this finds the average of all differences for every projection
		      if (tower>=15){
			      xavg_compare = xsum_comparison/xcount;
		      }
			  log<<MSG::DEBUG<<"About to set longest_xcompare: "<< longest_xcompare<<endreq;
			  status->setXlongest(longest_xcompare);
			  log<<MSG::DEBUG<<"Set longest x"<<endreq;
		      status->setXavg(xavg_compare);
		      status->setXlongestB(longest_xcompareBottom);
		  }
		  else{
			  log<<MSG::DEBUG<<"About to set longest_xcompare empty: "<< longest_xcompare<<endreq;
			  status->setXlongest(-2000);
			  log<<MSG::DEBUG<<"set longest x"<<endreq;
		      status->setXavg(-2000);
		      status->setXlongestB(-2000);
		  }

		  if (xslopeCount >0){
			  xslopeAvg = xslopeTotal/xslopeCount;
		  }
		  status->setXslopeL(xslopeL);
		  status->setXslopeAvg(xslopeAvg);
		  status->setXfirst(long_firstanglex);
		  status->setXtower(xslopetower);
		  status->setXangleL(xangleL);

          if (ycount>0){
		      //this finds the average of all differences for every projection
		      if (tower>=15){
			      yavg_compare = ysum_comparison/ycount;
		      }
			  log<<MSG::DEBUG<<"About to set longest_ycompare: "<< longest_ycompare<<endreq;
			  status->setYlongest(longest_ycompare);
			  log<<MSG::DEBUG<<"set longest y"<<endreq;
			  status->setYavg(yavg_compare);
			  status->setYlongestB(longest_ycompareBottom);
		  }
		  else{
			  log<<MSG::DEBUG<<"About to set longest_ycompare empty: "<< longest_ycompare<<endreq;
			  status->setYlongest(-2000);
			  log<<MSG::DEBUG<<"set empty y "<<endreq;
			  status->setYavg(-2000);
			  status->setYlongestB(-2000);
		  }

		  if (yslopeCount>0){
			  yslopeAvg = yslopeTotal/yslopeCount;
		  }
		  status->setYslopeL(yslopeL);
		  status->setYslopeAvg(yslopeAvg);
		  status->setYfirst(long_firstangley);
		  status->setYtower(yslopetower);
		  status->setYangleL(yangleL);

		  return StatusCode::SUCCESS;

}

StatusCode FilterTracks::WriteHits(){
  MsgStream log(msgSvc(),name());
  //get geometry access
  ITkrGeometrySvc *tkrGeoSvc=NULL;
  if(service("TkrGeometrySvc",tkrGeoSvc,true).isFailure()){
    log<<MSG::ERROR<<"Couldn't set up TkrGeometrySvc!"<<endreq;
    return StatusCode::SUCCESS;
  }

  //get TDS object for the hits
  SmartDataPtr<OnboardFilterTds::TowerHits> status(eventSvc(),
                                                   "/Event/Filter/TowerHits");
  if(!status){
    log<<MSG::ERROR<<"Unable to retrieve FilterStatus from TDS"<<endreq;
    return StatusCode::SUCCESS;
  }

    //get a pointer to the hits
	const OnboardFilterTds::TowerHits::towerRecord *hits = status->get();

    //get pointer to the event so we can get the event number
	SmartDataPtr<Event::MCEvent>     mcheader(eventSvc(),    EventModel::MC::Event);
    double event = mcheader->getSequence();

	//Here is the basic algorithm
    int tower=0;
	int view;
	int layer;
	short stripId;
	HepPoint3D stripcoord;
	double x, y, z;

	//loop over towers
	for (tower=0;tower<16;tower++){
	    if ((hits[tower].lcnt[0]) || (hits[tower].lcnt[1]>0)){

			//loop over the layers to collect the hits
		    for (int i=0;i<36;i++){
		        if (hits[tower].cnt[i] != 0){

					//write the view, tower, layer, and hit number.
					//first get the view and layer
					if (i<18){
						view = 0;   //this may be backwards!
					    layer = i;
					}
					else if (i>=18){
						view = 1;   //this may be backwards!
					    layer = i-18;
					}

					//now loop over all the strips in this layer
					for (int j=0;j<hits[tower].cnt[i];j++){
					    stripId = hits[tower].beg[i][j];
					    //call get strip position
                        stripcoord = findStripPosition(tkrGeoSvc,tower,layer,view,stripId);
					    //get the coordinates
						x = stripcoord.x();
						y = stripcoord.y();
						z = stripcoord.z();
					    //here, write everything out.
					    //write: event, x, y, z, view, tower, layer, strip
						//std::cout << event"  "<<tower<<"  "<<view<<"  "<<layer<<"  "<<stripId<<"  "<<x<<"  "<<y<<"  "<<z<<std::endl;
                        if (m_writehits){
						    if (m_outfile.is_open()){
								    m_outfile << event <<"\t"<< tower <<"\t"<< view <<"\t"<< layer
									  <<"\t"<< stripId  <<"\t"<< x <<"\t"<< y <<"\t"<< z << std::endl;
							}
						}
				    }
			    }//if any hits in layer
		    }//loop over layers
	    }//if any layers hit
		else{//if no layers are hit
			;
		}
	    //there aren't any hits for this event
    }//tower loop

	return StatusCode::SUCCESS;
}


StatusCode FilterTracks::TrackSelect(){
    MsgStream log(msgSvc(),name());
	//get geometry access
	ITkrGeometrySvc *tkrGeoSvc=NULL;
    if(service("TkrGeometrySvc",tkrGeoSvc,true).isFailure()){
      log<<MSG::ERROR<<"Couldn't set up TkrGeometrySvc!"<<endreq;
      return StatusCode::SUCCESS;
    }

	SmartDataPtr<OnboardFilterTds::FilterStatus> status(eventSvc(),
        "/Event/Filter/FilterStatus");
	if(!status){
        log<<MSG::ERROR<<"Unable to retrieve FilterStatus from TDS"<<endreq;
        return StatusCode::SUCCESS;
    }

    //Loop over the towers
    const OnboardFilterTds::projections *prjs=status->getProjection();
    double startPrj=0;
    int tower;
	int xhighlayer[16];
	int yhighlayer[16];
	int xnumprjs[16];
	int ynumprjs[16];
	int xhightower = 0;
	int yhightower = 0;
	int xcount = 0;
	int ycount = 0;
	int xlist[100];
	int ylist[100];
	int xsizes[100];
	int ysizes[100];
	int xmaxhits = 0;
	int ymaxhits = 0;
    int xspot = 0;
	int yspot = 0;
	int xlongcount = 0;
	int ylongcount = 0;

	for (int i=0;i<16;i++){ xhighlayer[i]=0; yhighlayer[i]=0;
		                    xnumprjs[i]=0;    ynumprjs[i]=0;   }


	  int tempxhigh=0;
	  int sethighok=0;
	  for (tower=0;tower<16;tower++){
		//FIRST: find the layer of the higest x and y projection
		//if there are any x projections
		int tempxhigh=0;
		if(prjs->xy[tower][0]>0){
            //loop over the x projections
			for(int xprj=startPrj;xprj<startPrj+prjs->xy[tower][0];xprj++){
                xnumprjs[tower] = prjs->xy[tower][0];
				xcount++;
				//if there are any y projections
		        if (prjs->xy[tower][1]>0){
		            //loop over the y projections
			        for (int yprj=startPrj+prjs->xy[tower][0];
				        yprj<startPrj+prjs->xy[tower][1]+prjs->xy[tower][0];yprj++){
			            //find the layer of the highest prj in each tower
				        sethighok=0;
						if (prjs->prjs[yprj].max == prjs->prjs[xprj].max){
                           sethighok = 1;
			               ynumprjs[tower]   = prjs->xy[tower][1];
                        }
						if (prjs->prjs[xprj].max >= tempxhigh){
							if (sethighok == 1){
								tempxhigh = prjs->prjs[xprj].max;
								sethighok = 0;
							}
						    xhighlayer[tower]=tempxhigh;
							yhighlayer[tower]=tempxhigh;
						}
				        ycount++;
		            }
		        }
			}
		}
		startPrj+=prjs->curCnt[tower];
      }

      //-------------------------------------------------------------

	  if ((xcount>0) && (ycount>0)){//only continue if there are x AND y projections
		                            //somewhere
        //Before continuing, require that the x and y projections found
	    //start in the same layer and the same tower.
	    //Loop to find the towers with the highest shared starting layers.
        int highestlayer = 0;
        for (tower=0; tower<16; tower++){
	 	    //first see if the starting layer is the same for x and y
		    if (yhighlayer[tower] == xhighlayer[tower]){
		        //then find the highest layer for towers that have
				//the same x and y starting layer
	  		    if ( xhighlayer[tower] > highestlayer ){
			        highestlayer = xhighlayer[tower];
			    }
		    }
	    }
		int towers_to_check[16];
		int tower_exists = 0;
		for (int j=0; j<16; j++) { towers_to_check[j] = 0; }
		//loop again to get the towers that have this highest layer
		for (tower=0;tower<16;tower++){
			if ((xhighlayer[tower] == highestlayer) && (highestlayer>1)){//must be a 3inarow
				towers_to_check[tower] = 1;
				tower_exists = 1;
			}
	    }
		//At this point, we have found the highest starting layer
		//of any pair of projections in any tower.
        //-------------------------------------------------------------

		if (tower_exists){//only continue if a valid tower exists

		//SECOND: Now we have a list of towers to check.  Loop over towers,
		//        and if towers_to_check[tower] == 1, continue.
		//        For these towers, which are the towers that have an
		//        x and y projection starting in the same layer, we will
		//        look for the longest projection that starts in the highest
		//        layer.  If there is more than one projection starting in
		//        the highest layer, we will average them.
		//        Here we are making a list of the projections that start
		//        in the highest layer.  Now for each tower that has these
		//        special projections, we want to find the tower with the
		//        highest number and the tower with the longest prj (most hits).  If
		//        two or more towers have the same "most hits," or same
		//        "highest number," we pick the one that has the combined
		//        "highest number" and "most hits" by multiplying them
		//        together to get a "tower selection factor."  If more than
		//        one tower has the same tsf, we'll take the tower
		//        with the most prjs overall.  If THAT is the same, we'll
		//        just take the first one (lowest tower number).
		//
		//        Store this info in an array.  We're assuming there are
		//        not more than 100 projections!  Could have counted the
		//        projections up and allocated memory, but that's time
		//        consuming.
		int xmaxprj = 0;
		int ymaxprj = 0;
		int tempxmaxprj, tempxmaxhits;
		int tempymaxprj, tempymaxhits;
		int sum_maxhits[16];
		int sum_maxprjs[16];
		int sum_product[16];
		for (int r=0;r<16;r++){ sum_maxhits[r]=0; sum_maxprjs[r]=0; sum_product[r];}

		startPrj=0;
		for (tower=0;tower<16;tower++){
			tempxmaxprj = 0;  tempymaxprj = 0;
			tempxmaxhits = 0; tempymaxhits = 0;
			if (towers_to_check[tower] == 1){
				//loop over the x projections
		        for(int xprj=(int)startPrj;xprj<startPrj+prjs->xy[tower][0];xprj++){
				    if (prjs->prjs[xprj].max == highestlayer){
					    //find the greatest number of hits in
						//one of these projections
				        if (prjs->prjs[xprj].nhits > xmaxhits){
					        xmaxhits = prjs->prjs[xprj].nhits;
							tempxmaxhits = xmaxhits;
				        }
						//find the maximum number of x prjs that start in
				        //the highest layer in one of these towers
				        tempxmaxprj++;
				    }
			    }

				if (tempxmaxprj > xmaxprj) { xmaxprj = tempxmaxprj; }

                //loop over the y projections
				for (int yprj=startPrj+prjs->xy[tower][0];
				    yprj<startPrj+prjs->xy[tower][1]+prjs->xy[tower][0];yprj++){
			        if (prjs->prjs[yprj].max == highestlayer){
			            //find the greatest number of hits
				        if (prjs->prjs[yprj].nhits > ymaxhits){
					        ymaxhits = prjs->prjs[yprj].nhits;
							tempymaxhits = ymaxhits;
						}
						//find the maximum number of y prjs that start in
						//the highest layer in one of these towers
						tempymaxprj++;
				    }
			    }

				if (tempymaxprj > ymaxprj) { ymaxprj = tempymaxprj; }

			}//if towers to check = 1
			startPrj+=prjs->curCnt[tower];
			sum_maxhits[tower] = tempxmaxhits+tempymaxhits;
			sum_maxprjs[tower] = tempxmaxprj+tempymaxprj;
			sum_product[tower] = sum_maxhits[tower]*sum_maxprjs[tower];
        }//for tower

		//At this point, we have a list of the sum of the max hits for the longest
		//prjs in each tower, and we have a list of the number of prjs in each tower
		//that start in the highest layer
        //--------------------------------------------------------------

		//THIRD: Now we are in a position to pick out the tower with
		//       the strongest projection pair (using sum_maxhits),
		//       and the tower with the greatest number of potential prj
		//       pairs (sum_maxprjs).
		//       To do this, we loop until we find: the tower with
		//       sum_maxhits, and the tower with sum_maxprjs.
		//       If more than one tower has these values, we choose
		//       the one with the greatest sum_maxhits*sum_maxprjs.
		//       If more than one tower has the same sum*sum value,
		//       we choose the one with the most prjs overall.
		//       If more than one tower has all 3, we choose the first
		//       one in the list.

		//first find the largest values of sum_maxhits, sum_maxprjs, and sum_product.
		int largest_sum_maxhits = 0;
		int largest_sum_maxprjs = 0;
		int largest_sum_product = 0;
		for (tower=0;tower<16;tower++){
		    if (sum_maxhits[tower] > largest_sum_maxhits) largest_sum_maxhits = sum_maxhits[tower];
			if (sum_maxprjs[tower] > largest_sum_maxprjs) largest_sum_maxprjs = sum_maxprjs[tower];
			if (sum_product[tower] > largest_sum_product) largest_sum_product = sum_product[tower];
		}
		//now see how many towers have the largest values
		int hitscount = 0; int prjscount = 0; int productcount = 0;
		int hitstower = 0; int prjstower = 0; int producttower = 0;
		for (tower=0;tower<16;tower++){
			if (sum_maxhits[tower] == largest_sum_maxhits){ hitscount++; hitstower = tower; }
			if (sum_maxprjs[tower] == largest_sum_maxprjs){ prjscount++; prjstower = tower; }
			if (sum_product[tower] == largest_sum_product){ productcount++; producttower=tower;}
		}

        //If more than one tower has a long projection, see if the one with the longest also
		//has the most projections.  If the towers for longest and most are different, pick
		//the one with the highest product.
		int index = 0;
		if (hitscount > 1){
			if (hitstower == prjstower)
				index = hitstower;
			else
				index = producttower;
		}
		else
			index = hitstower;

		//--------------------------------------------------------------
        //FOURTH: We finally have the index of the tower we are going to use.
		//        Now we just need to average the projections that start in the
		//        highest common layer.
		tower = index;
		HepPoint3D point;
		std::vector<double> x;
		std::vector<double> xz;
        std::vector<double> y;
		std::vector<double> yz;
		std::vector<double> zAvg;
		x.push_back(0);
        x.push_back(0);
		xz=y=yz=zAvg=x;
		double x_h, x_v, y_h, y_v, z_v;
		double x_h_sum = 0;
		double y_h_sum = 0;
		int xprjcount=0;
		int yprjcount=0;
		startPrj=0;
		for (int i=0;i<tower;i++){
			startPrj+=prjs->curCnt[i];
		}
		//loop over the x projections in the highest common layer, and get an average slope
        for(int xprj=startPrj;xprj<startPrj+prjs->xy[tower][0];xprj++){
			//make sure it starts in the highest common layer
			if (prjs->prjs[xprj].max == highestlayer){
		        //calculate its slope
                point=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max,0,
                                                  prjs->prjs[xprj].hits[0]);
                x[0]=point.x();
				xz[0]=point.z();
                point=findStripPosition(tkrGeoSvc,tower,prjs->prjs[xprj].max-1,0,
                                                  prjs->prjs[xprj].hits[1]);
                x[1]=point.x();
				xz[1]=point.z();
                x_h = x[1]  - x[0];
				x_v = xz[0] - xz[1];
                //sum the slope componets
                x_h_sum += x_h;
				//keep track of the number of projections
				xprjcount++;
		    }
	    }
		//loop over the y projections in the highest common layer, and get an average slope
		for(int yprj=startPrj+prjs->xy[tower][0];
				yprj<startPrj+prjs->xy[tower][1]+prjs->xy[tower][0];yprj++){
			if (prjs->prjs[yprj].max == highestlayer){
		        //calculate its slope
                point=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max,
                                                          1,prjs->prjs[yprj].hits[0]);
                y[0]=point.y();
                yz[0]=point.z();
                point=findStripPosition(tkrGeoSvc,tower,prjs->prjs[yprj].max-1,
                                                          1,prjs->prjs[yprj].hits[1]);
                y[1]=point.y();
                yz[1]=point.z();
				y_h = y[1]  - y[0];
				y_v = yz[0] - yz[1];
                //sum the slope
                y_h_sum += y_h;
				//keep track of the number of projections
				yprjcount++;
			}
	    }
        //if there were more than one x prj or y prj, average them.  Otherwise, just use the one
		x_h = x_h_sum/xprjcount;
		y_h = y_h_sum/yprjcount;
		//now we have x_h, y_h, x_v, and y_v, and here is zAvg
		for(int counter=0;counter<2;counter++)
            zAvg[counter]=(xz[counter]+yz[counter])/2;
		z_v = zAvg[0] - zAvg[1];
		//now compute theta and phi for these tracks
        computeAngles(x_h, x_v, y_h, y_v, z_v);
		//now compute the seperation
		    //Obtain McZDir, McXDir, McYDir (copied from McValsTools.cxx)
            SmartDataPtr<Event::McParticleCol> pMcParticle(eventSvc(),EventModel::MC::McParticleCol);
            if(pMcParticle){
            Event::McParticleCol::const_iterator pMCTrack1 = pMcParticle->begin();
            HepLorentzVector Mc_p0 = (*pMCTrack1)->initialFourMomentum();
            Vector Mc_t0 = Vector(Mc_p0.x(),Mc_p0.y(), Mc_p0.z()).unit();
            double MC_xdir,MC_ydir,MC_zdir;
            MC_xdir   = Mc_t0.x();
            MC_ydir   = Mc_t0.y();
            MC_zdir   = Mc_t0.z();
            //Convert MC dir into theta and phi
            double phi_rad_mc=0;
            double theta_rad_mc=acos(-MC_zdir);
            if(MC_zdir < 0)
	        theta_rad_mc=M_PI - acos(-MC_zdir);
            double x_cord=-MC_xdir;
            double y_cord=-MC_ydir;
            if(y_cord==0)
                phi_rad_mc=M_PI/2;
            if ((x_cord>0)&&(y_cord>0)){
                phi_rad_mc = atan(y_cord/x_cord);
                if(MC_zdir<0)
                    phi_rad_mc = 3*M_PI/2 - atan(x_cord/y_cord);
            }
            if ((x_cord<0)&&(y_cord<0)){
                phi_rad_mc = 3*M_PI/2 - atan(x_cord/y_cord);
                if(MC_zdir<0)
                    phi_rad_mc = atan(y_cord/x_cord);
            }
            if ((x_cord>0)&&(y_cord<0)){
                phi_rad_mc = 2*M_PI-atan(-y_cord/x_cord);
                if(MC_zdir<0)
                    phi_rad_mc=M_PI/2 - atan(x_cord/y_cord);
            }
            if ((x_cord<0)&&(y_cord>0)){
                phi_rad_mc = M_PI/2 - atan(y_cord/(-x_cord));
                if(MC_zdir<0)
                    phi_rad_mc = 2*M_PI-atan(-y_cord/x_cord);
            }
            //Compute angular seperation
            double xone=sin(theta_rad_mc)*cos(phi_rad_mc);
            double yone=sin(theta_rad_mc)*sin(phi_rad_mc);
            double zone=cos(theta_rad_mc);
            double xtwo=sin(m_theta_rad)*cos(m_phi_rad);
            double ytwo=sin(m_theta_rad)*sin(m_phi_rad);
            double ztwo=cos(m_theta_rad);
            double dot_product=xone*xtwo+yone*ytwo+zone*ztwo;
            double separation_rad=acos(dot_product);
			//if (separation_rad > M_PI/2.)
			//	separation_rad = M_PI - separation_rad; //we always assume that events come from "above"
            status->setSeparation2(separation_rad*180/M_PI);
        }
        else{
            log<<MSG::ERROR <<"Unable to obtain McParticleCol from TDS"<<endreq;
            status->setSeparation2(-1.0);
        }
	  }//if tower_exists (that has an x and y pair starting in the highest layer hit
	  else status->setSeparation2(-1.0);
    }//if (xcount && ycount)
    else status->setSeparation2(-1.0);

	return StatusCode::SUCCESS;

}
