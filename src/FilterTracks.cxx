#include "OnboardFilter/FilterTracks.h"

static const AlgFactory<FilterTracks>  Factory;
const IAlgFactory& FilterTracksFactory = Factory;

FilterTracks::FilterTracks(const std::string& name, ISvcLocator* pSvcLocator)
:Algorithm(name, pSvcLocator){

    declareProperty("UseNumHits",     m_usenumhits = 0);

}

StatusCode FilterTracks::initialize(){
    MsgStream log(msgSvc(),name());
    log<<MSG::DEBUG<<"Initializing"<<endreq;
    m_x.push_back(0);
    m_x.push_back(0);
    m_x.push_back(0);
    m_y=m_xz=m_yz=m_zAvg=m_pointHigh=
      m_extendHigh=m_extendLow=m_x;
    m_pi = 3.14159265358979323846;
    return StatusCode::SUCCESS;
}

StatusCode FilterTracks::finalize(){
    MsgStream log(msgSvc(),name());
    log<<MSG::DEBUG<<"Finalizing"<<endreq;
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
                        computeAngles();
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
			startPrj+=prjs->curCnt[tower];//I moved this 06/14/04 - DW
        }
    }
    std::vector<OnboardFilterTds::track> tracks=status->getTracks();
    double maxLength=0;
	int   maxnumhits=0;
    unsigned int currMax,currMaxHits;
    log<<MSG::DEBUG<<"Processing "<<tracks.size()<<" tracks"<<endreq;
    if(tracks.size()>0){
        for(unsigned int counter=0;counter<tracks.size();counter++){//currently using the track that is the
            if(tracks[counter].length>maxLength){                   //longest, not the track with the most
                maxLength=tracks[counter].length;                   //hits.  Usually, these will be the same,
                currMax=counter;                                    //but it is possible to have a long track
            }                                                       //at a shallow angle with only 3 hits,
		}                                                           //and a short track with more than this.
		for(unsigned int counter2=0;counter2<tracks.size();counter2++){//This is an alternative method to picking
		    if(tracks[counter2].numhits>maxnumhits){                   //the track to use.  This one picks out
		        maxnumhits=tracks[counter2].numhits;                   //the track that has the most hits, not
				currMaxHits=counter2;                                  //the longest track
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
	        theta_rad_mc=m_pi - acos(-MC_zdir);
            double x_cord=-MC_xdir;
            double y_cord=-MC_ydir;
            if(y_cord==0)
                phi_rad_mc=m_pi/2;
            if ((x_cord>0)&&(y_cord>0)){
                phi_rad_mc = atan(y_cord/x_cord);
                if(MC_zdir<0)
                    phi_rad_mc = 3*m_pi/2 - atan(x_cord/y_cord);
            }
            if ((x_cord<0)&&(y_cord<0)){
                phi_rad_mc = 3*m_pi/2 - atan(x_cord/y_cord);
                if(MC_zdir<0)
                    phi_rad_mc = atan(y_cord/x_cord);
            }
            if ((x_cord>0)&&(y_cord<0)){
                phi_rad_mc = 2*m_pi-atan(-y_cord/x_cord);
                if(MC_zdir<0)
                    phi_rad_mc=m_pi/2 - atan(x_cord/y_cord);
            }
            if ((x_cord<0)&&(y_cord>0)){
                phi_rad_mc = m_pi/2 - atan(y_cord/(-x_cord));
                if(MC_zdir<0)
                    phi_rad_mc = 2*m_pi-atan(-y_cord/x_cord);
            }
            //Compute angular seperation
            double xone=sin(theta_rad_mc)*cos(phi_rad_mc);
            double yone=sin(theta_rad_mc)*sin(phi_rad_mc);
            double zone=cos(theta_rad_mc);
            double xtwo=sin(tracks[currMax].theta_rad)*cos(tracks[currMax].phi_rad);
            double ytwo=sin(tracks[currMax].theta_rad)*sin(tracks[currMax].phi_rad);
            double ztwo=cos(tracks[currMax].theta_rad);
            double dot_product=xone*xtwo+yone*ytwo+zone*ztwo;
            double seperation_rad=acos(dot_product);
			if (seperation_rad > m_pi/2.) 
				seperation_rad = m_pi - seperation_rad; //we always assume that events come from "above"
            status->setSeperation(seperation_rad*180/m_pi);
        }
        else{
            log<<MSG::ERROR <<"Unable to obtain McParticleCol from TDS"<<endreq;
            return StatusCode::FAILURE;
        }
    }
    else{
        status->setSeperation(-1);
        log<<MSG::DEBUG<<"No tracks found"<<endreq;
    }
    return StatusCode::SUCCESS;
}

void FilterTracks::computeAngles(){
    double x_h = m_x[1]-m_x[0];
    double x_v = m_xz[0]-m_xz[1];
    double y_h = m_y[1]-m_y[0];
    double y_v = m_yz[0]-m_yz[1];
    double z_v = m_zAvg[0]-m_zAvg[1];
    double t_h_ave = 1;
    if(x_h == 0 && y_h==0){
        m_phi_rad=0;
        m_theta_rad=0;
    }
    else{
        if(x_h ==0){
            m_theta_rad=m_pi/2-atan(y_v/y_h);
            if(y_h>0)
                 m_phi_rad=m_pi/2;
            else
                 m_phi_rad=3*m_pi/2;
        }
        else{
            if(y_h==0){
                m_theta_rad=m_pi/2-atan(x_v/x_h);
                if(x_h>0)
                    m_phi_rad=0;
                else
                    m_phi_rad=m_pi;
            }
            else{
                if((x_h>0) && (y_h>0)){
                    m_phi_rad=atan(y_h/x_h);
                    t_h_ave=((x_h/cos(m_phi_rad)) + (y_h/sin(m_phi_rad)))/2;
                    m_theta_rad = m_pi - atan(t_h_ave/z_v);
                }
                else{
                    if((x_h<0) && (y_h>0)){
                        m_phi_rad=m_pi/2-atan(x_h/y_h);
                        t_h_ave = ( -x_h/sin(m_phi_rad - m_pi/2) + y_h/cos(m_phi_rad - m_pi/2) )/2;
                        m_theta_rad = m_pi - atan(t_h_ave/z_v);
                    }
                    else{
                        if((x_h<0) && (y_h<0)){
                            m_phi_rad=3*m_pi/2 - atan(x_h/y_h);
                            t_h_ave = ( (-x_h/sin(3*m_pi/2 - m_phi_rad)) + (-y_h/cos(3*m_pi/2 - m_phi_rad)) )/2;
                            m_theta_rad = m_pi - atan(t_h_ave/z_v);
                        }
                        else{
                            if ((x_h>0) && (y_h<0)){
                                m_phi_rad=2*m_pi-atan(-y_h/x_h);
                                t_h_ave = ((x_h/cos(2*m_pi-m_phi_rad)) + (-y_h/sin(2*m_pi-m_phi_rad)))/2;
                                m_theta_rad = m_pi - atan(t_h_ave/z_v);
                            }
                            else
                                m_phi_rad=3*m_pi/2-atan(y_h/x_h);
                        }
                    }
                }
            }
        }
    }
    m_theta=m_theta_rad*180/m_pi;
    m_phi=m_phi_rad*180/m_pi;
}

void FilterTracks::computeLength(){
    double t_v=m_zAvg[0]-m_zAvg[2];
    double t_h = t_v*tan(m_pi - m_theta_rad);
    m_length=sqrt(t_v*t_v+t_h*t_h);
    m_pointHigh[0]=t_h*cos(m_phi_rad) + m_x[0];
    m_pointHigh[1]=t_h*sin(m_phi_rad) + m_y[0];
    m_pointHigh[2]=m_zAvg[2];
}

void FilterTracks::computeExtension(){
    const double length=1000;
    m_extendLow[0]=length*sin(m_pi-m_theta_rad)*cos(m_pi+m_phi_rad) + m_x[0];
    m_extendLow[1]=length*sin(m_pi-m_theta_rad)*sin(m_pi+m_phi_rad) + m_y[0];
    m_extendLow[2]=length*cos(m_pi-m_theta_rad) + m_zAvg[0];

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

