/**
 * @class OnboardFilter
 * @brief Driver program to test filtering code
 * @author JJRussell - russell@slac.stanford.edu
 * @author David Wren - dnwren@milkyway.gsfc.nasa.gov
 * @author Navid Golpayegani - golpa@milkyway.gsfc.nasa.gov
 * $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/OnboardFilter.cxx,v 1.25 2003/08/28 22:58:13 golpa Exp $
 */
   
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include "DFC/EBF_fileIn-Gleam.h"
#include "DFC/EBF_fileOut.h"
#include "DFC/DFC_ss.h"
#include "DFC/DFC_ctl.h"
#include "DFC/DFC_ctlSysDef.h"
#include "DFC/DFC_results.h"
#include "DFC/DFC_status.h"
#include "DFC/DFC_statistics.h"
#include "DFC/DFC_filter.h"
#include "DFC/TFC_triggerFill.h"
#include "DFC/DFC_latRecord.h"
#include "DFC/DFC_latUnpack.h"


#include "DFC/DFC_display.h"
#include "DFC/TFC_geometryPrint.h"
#include "DFC_resultsDef.h"

#include "DFC_ctlCfcDef.h"
#include "DFC/CFC_constants.h"

// Gaudi system includes
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
#include "OnboardFilter/FilterStatus.h"

//Gui display
#include "gui/DisplayControl.h"
#include "GuiSvc/IGuiSvc.h"
#include "gui/GuiMgr.h"
#include "FilterTrackDisplay.h"
#include "FilterExtendedDisplay.h"

#include <string.h>
#ifdef   __linux
#include <getopt.h>
#endif
#include <iomanip>

/**
 * Data structure defining the standard geometry used in the filtering process.
 */
struct _TFC_geometry;
extern const struct _TFC_geometry TFC_GeometryStd;
    
/**
 * Collection of all the control parameters together. These are
 * essentially the interpretted command line options.
 */
typedef struct _Ctl
{
    const char  *ifile;  /*!< The name of the input file (Evt Bld  format)*/
    const char  *ofile;  /*!< The name of the output file (optional)      */
    unsigned int *data;  /*!< Pointer to a EBD data structure             */
    int     to_process;  /*!< The number of events to process.            */
    int        to_skip;  /*!< The number of events to initially skip.     */
    int       to_print;  /*!< The number of events to print.              */
    int    ss_to_print;  /*!< The mask of which subsystems to print       */
    int          quiet;  /*!< Minimal summary information                 */
    int           list;  /*!< Print list of events not rejected           */
    int       esummary;  /*!< Event results summary flags                 */
    int       geometry;  /*!< Print the detector geometry                 */
}
Ctl;


class OnboardFilter:public Algorithm{
public:
    OnboardFilter(const std::string& name, ISvcLocator* pSvcLocator);

	/**
     * @brief Count number of events stored in control structure
     * @param evts  Pointer to the list of events.
     * @param size  Total size, in bytes of the list of events.
     * @return      The number of events in the event list.
     */
    int countEvts  (const unsigned int *evts, int size);
    /**
     * Retrieves the GLEAM Monte Carlo Event Sequence Number
     * @param   evt Pointer to the event
     * @param esize The event size
     * @return      The GLEAM Monte Carlo Event Sequence Number.
     */
    static inline int getMCsequence (const unsigned int *evt, int esize){
        return evt[0x10];
    }
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();
private:
    /**
     * Converts an integer into Binary representation
     * @param number The number to represent in Binary
     * @return A string represent the number in Binary
     */
    std::string convertBase(unsigned int number);
    /**
     * Convert the projections stored in TDS into x,y,z coordinates
     * @param status Pointer to the TDS object containing the projections
     */
    StatusCode computeCoordinates(OnboardFilterTds::FilterStatus *status);
    /**
     * Compute Angles for a given track
     * @param x The x-coordinates for hits 0,1, and Last
     * @param y The y-coordinates for hits 0,1, and Last
     * @param xz The z-coordinates for hits 0,1, and Last as computed from 
     *           the x projections
     * @param yz The z-coordinates for hits 0,1, and Last as computed from
     *           the y projections
     */
    void computeAngles(std::vector<double> x,std::vector <double> y,
		       std::vector<double> xz,std::vector<double> yz,std::vector<double> z,
		       double &phi,double &phi_rad, double &theta,double &theta_rad);
  
    /**
     * Compute Length of a track
     * @param z The average z-coordinates 
     * @param theata_rad The Theta angle obtained by computeAngles() in radians
     * @param phi_rad The phi angle obtained by computeAngles() in radians
     */
    void computeLength(std::vector<double> x, std::vector<double> y, std::vector<double> z, double theta_rad, double phi_rad,
                       double &length, std::vector<double> &endPoint);
    /**
     * Compute the extensions to the tracks
     * @param x 3 Element array containing the x coordinates
     * @param y 3 Element array containing the y coordinates
     * @param z 3 Element array containing the average z coordinates
     * @param phi_rad The phi angle of the track in radians
     * @param theta_rad The theta angle of the track in raidans
     * @param extendLow 3 Element array that will contain the xyz of the extension
     * @param extendHigh 3 Element array that will contain the xyz of the extension
     */
    void OnboardFilter::computeExtension(std::vector<double> x,std::vector<double> y,
                                         std::vector<double> z, double phi_rad,
                                         double theta_rad, 
                                         std::vector<double> &extendLow,
                                         std::vector<double> &extendHigh);
    //Structures that control the Behaviour of the Filter Code
    Ctl m_ctl_buf;
    Ctl *m_ctl;

   IntegerProperty m_mask; // mask for setting filter to reject
   int  m_rejected;
};

static const AlgFactory<OnboardFilter>  Factory;
const IAlgFactory& OnboardFilterFactory = Factory;

StatusCode OnboardFilter::finalize(){
      MsgStream log(msgSvc(),name());
      log  << MSG::INFO << "Rejected " << m_rejected << " triggers " << endreq;
  return StatusCode::SUCCESS;
}

OnboardFilter::OnboardFilter(const std::string& name, ISvcLocator* pSvcLocator)
:Algorithm(name, pSvcLocator), m_rejected(0){

    declareProperty("mask"     ,  m_mask=0xffffffff); // trigger mask: select bits for rejection
}

StatusCode OnboardFilter::initialize()
{
  MsgStream log(msgSvc(),name());
  setProperties();

  log<< MSG::INFO << "Initializing Filter Settings"<<endreq;
  //Set up default values for the control structure
  m_ctl_buf.ifile="test.ebf";
  m_ctl_buf.ofile=NULL;
  m_ctl_buf.data=NULL;
  m_ctl_buf.to_process=1;
  m_ctl_buf.to_skip=0;
  m_ctl_buf.to_print=0;
  m_ctl_buf.ss_to_print=0;
  m_ctl_buf.quiet=1;
  m_ctl_buf.list=0;
  m_ctl_buf.esummary=0;
  m_ctl_buf.geometry=0;
  m_ctl    = &m_ctl_buf;
  //Set up GUI display
  IGuiSvc *guiSvc;
  if(!service("GuiSvc",guiSvc).isFailure()){
    gui::DisplayControl& display = guiSvc->guiMgr()->display();
    gui::DisplayControl::DisplaySubMenu& fltrmenu = display.subMenu("OnboardFilter");
    //Add Display Objects Here
    fltrmenu.add(new FilterTrackDisplay(eventSvc()),"Tracks");
    fltrmenu.add(new FilterExtendedDisplay(eventSvc()),"Extended Tracks");
  }
  else{
    log<<MSG::WARNING<<"No GuiSvc. Not displaying tracks"<<endreq;
  }
  return StatusCode::SUCCESS;
}

StatusCode OnboardFilter::execute()
{
    TMR_tick                  beg;
    TMR_tick                  end;
    unsigned int          elapsed;
    int                nprocessed;
    struct _DFC_ctl       *dfcCtl;
    int                   dfcSize;
    EBF_file                 *ebf;
    int                     nevts;
    int                    evtCnt;
    int                      size;
    const unsigned int       *evt;
    const unsigned int    *begevt;
    int                       idx;
    int                       iss;
    unsigned int           nprint;
    unsigned int            nskip;
    int                to_process;
    DFC_ctlSys            *dfcSys;
    int               resultsSize;
    struct _DFC_results  *results;
    struct _DFC_results   *result;
    DFC_statistics     statistics;
    struct _DFC_latRecord *dfcEvt;
    const struct _DFC_ctlCfc *cfc;
    
    //Initialize variables that will temporarily store data to be put in TDS
    for(int counter=0;counter<16;counter++)
      TDS_layers[counter]=0;
    TDS_variables.tcids=0;
    TDS_variables.acd_xz=0;
    TDS_variables.acd_yz=0;
    TDS_variables.acd_xy=0;
    for(int counter=0;counter<16;counter++){
      TDS_variables.acdStatus[counter]=0;
      memset(&TDS_variables.prjs[counter],0,sizeof(TDS_variables.prjs[counter]));
    }

    MsgStream log(msgSvc(),name());
	//Initialize the TDS object
    OnboardFilterTds::FilterStatus *newStatus=
        new OnboardFilterTds::FilterStatus;
    eventSvc()->registerObject("/Event/Filter/FilterStatus",newStatus);
	
    /* Initialize the time base */
    TMR_initialize ();
	
	//Initialize the control structures
	dfcSize = DFC_ctlSizeof ();
    dfcCtl  = (struct _DFC_ctl *)malloc (dfcSize);
    if (dfcCtl == NULL)
    {
        printf ("Cannot allocate %8.8x(%d) bytes for control structure\n",
                dfcSize, dfcSize);
        return StatusCode::FAILURE;
    }
    DFC_ctlInit (dfcCtl);
	
    dfcSize  = DFC_latRecordSizeof ();
    //dfcEvt   = (struct _DFC_latRecord *)malloc (dfcSize);
    dfcEvt = new _DFC_latRecord;
    if (dfcEvt == NULL)
    {
        printf ("Cannot allocate %8.8x(%d) bytes for DFC unpack record\n",
                dfcSize, dfcSize);
        return StatusCode::FAILURE;
    }
    DFC_latRecordInit (dfcEvt);
	SmartDataPtr<EbfWriterTds::Ebf> ebfData(eventSvc(),"/Event/Filter/Ebf");
	
	//Retrieve event from TDS and store into control structures
	if(!ebfData){
      return StatusCode::FAILURE;
    }
	unsigned int length;
    char *data=ebfData->get(length);
    ebf   = EBF_openGleam   (length);
    iss   = EBF_readGleam   (ebf,(unsigned int *)data);
    iss   = EBF_closeGleam  (ebf);
    size  = EBF_esizeGleam  (ebf);
    evt   = EBF_edataGleam  (ebf);
    nevts = countEvts       (evt, size);
	
    /* If number of events not specified, use all */
    to_process = m_ctl->to_process;
    if (to_process < 0) to_process = nevts;
    
    nskip       = m_ctl->to_skip;
    nprint      = m_ctl->to_print;
    to_process += nskip;
    nevts       = to_process > nevts ? nevts : to_process;

    
    /* Can't print more than there are */
    if (nskip  > nevts) nskip  = nevts;
    if (nprint > nevts) nprint = nevts;
    evtCnt = nevts - nskip;

	
    /* Allocate the memory for the results vector */
    resultsSize     = DFC_resultsSizeof ();
    //results         = (struct _DFC_results *)malloc (resultsSize * evtCnt);
    results = new _DFC_results[evtCnt];
    if (results == NULL)
    {
        printf ("Cannot allocate %d bytes for % d result vectors\n",
                resultsSize * evtCnt, evtCnt);
        return StatusCode::FAILURE;
    }

    
    /* Skip the request number of events */
    for (idx = 0; idx < nskip; idx++)
    {
        int esize = *evt;
        size -= esize;
        evt = (const unsigned int *)((char *)(evt)+esize);
    }
    
	
    begevt          = evt;
    result          = results;
    cfc             = DFC_ctlCfcLocate (dfcCtl);
    dfcSys          = DFC_ctlSysLocate (dfcCtl);
    dfcSys->toPrint = nprint;
    DFC_statisticsClear (&statistics);


    /* Invalidate the cache where the events are stored */
    //CACHE_invalidateData (evt, size);
    

	
    beg             = TMR_GET ();
    for (idx = nskip; idx < nevts; idx++)
    {
        int status;
        int  esize;

     
        /* Perform any diagnostic print-out of the event */
        if (dfcSys->toPrint > 0)
        {
            dfcSys->toPrint -= 1;
            DFC_latRecordUnpack (dfcEvt, evt, dfcCtl);
            DFC_display (dfcEvt);
        }

        
        esize   = *evt;
        status  = DFC_filter (dfcCtl, result, dfcEvt, evt, esize);

        evt = (const unsigned int *)((char *)(evt)+esize);
        result  = (struct _DFC_results *)((unsigned char *)result
                                                         + resultsSize);
    }
    end        = TMR_GET ();
    elapsed    = TMR_DELTA_IN_NSECS (beg, end);

    
	
    /* Ensure that the energy status is filled in */
    evt    = begevt;
    result = results;
    for (idx = nskip; idx < nevts; idx++)
    {
        int   size = *evt;
        int   status;
        
        
        DFC_filterComplete (dfcCtl, result, dfcEvt, evt, size);
        
        status = result->status;

        //use the status word to set the filter
        if( m_mask!=0 && (m_mask& (status>>15)) !=0 ){
            this->setFilterPassed(false);
            m_rejected++; // count the number rejected
        }
        newStatus->set(status);
        newStatus->setCalEnergy(result->energy);
        newStatus->setTcids(TDS_variables.tcids);
        newStatus->setAcdMap(TDS_variables.acd_xz,TDS_variables.acd_yz,
			TDS_variables.acd_xy);
        for(int counter=0;counter<16;counter++){
          newStatus->setAcdStatus(counter,TDS_variables.acdStatus[counter]);
	  OnboardFilterTds::projections prjs;
	  prjs.curCnt=TDS_variables.prjs[counter].curCnt;
	  prjs.xy[0]=TDS_variables.prjs[counter].xy[0];
	  prjs.xy[1]=TDS_variables.prjs[counter].xy[1];
	  for(int prjCounter=0;prjCounter<prjs.curCnt;prjCounter++){
	    prjs.prjs[prjCounter].layers=TDS_variables.prjs[counter].prjs[prjCounter].layers;
            prjs.prjs[prjCounter].min=TDS_variables.prjs[counter].prjs[prjCounter].min;
            prjs.prjs[prjCounter].max=TDS_variables.prjs[counter].prjs[prjCounter].max;
            prjs.prjs[prjCounter].nhits=TDS_variables.prjs[counter].prjs[prjCounter].nhits;
	    for(int hitCounter=0;hitCounter<18;hitCounter++)
	      prjs.prjs[prjCounter].hits[hitCounter]=TDS_variables.prjs[counter].prjs[prjCounter].hits[hitCounter];
	  }
	  newStatus->setProjection(counter,prjs);
        }
        newStatus->setLayers(TDS_layers);
        log << MSG::DEBUG;
        if(log.isActive()) { 
            log.stream()<< "FilterStatus Code: "
                << std::setbase(16) << (unsigned int)status<<" : "
                << convertBase(status);
            if (m_ctl->list && (result->status & DFC_M_STATUS_VETOES) == 0)
            {
                //TODO: write this to the stream instead
                printf ("0000 %8d %8d\n", getMCsequence (evt, size), idx);
            }
        }
        log << endreq;
        result = (struct _DFC_results   *)((char *)(result)+resultsSize);
        evt = (const unsigned int *)((char *)(evt)+size);
    }
    
    
    nprocessed = nevts - nskip;
    DFC_resultsPrint (results, evtCnt, m_ctl->esummary);
#if 0 // disable this for now
    printf ("Elapsed Time: %10d / %5d = %7d nsecs\n",
            elapsed,
            nprocessed,
            elapsed/(nprocessed ? nprocessed : 1));
        
    if (m_ctl->quiet == 0)
    {
        DFC_statisticsAccumulate (&statistics, results, evtCnt);
        DFC_statisticsPrint      (&statistics);
    }
#endif
    

    EBF_free (ebf);
    delete[] results;
    free(dfcCtl);
    delete dfcEvt;
    return computeCoordinates(newStatus);
}

int OnboardFilter::countEvts (const unsigned int *evts, int size)
{
    int nevts = 0;
 
    while (size > 0) 
    {
        int esize = *evts;
        size     -= esize;
        nevts    += 1;
        
        evts = (const unsigned int *)((char *)(evts)+esize);
    }
    
    return nevts;
}


std::string OnboardFilter::convertBase(unsigned int number){
    std::string output;
    int count=1;
    do{
        if(number%2)
            output = "1" + output;
        else
            output = "0" + output;
        number/=2;
        if(!(count%4))
            output = " " + output;
        count++;
    }while(number);
    for(int counter=count;counter<=32;counter++){
        output = "0" + output;
        if(!(counter%4))
            output = " " + output;
    }
    return output;
}

StatusCode OnboardFilter::computeCoordinates(OnboardFilterTds::FilterStatus *status){
  const double pi = 3.14159265358979323846;
  ITkrGeometrySvc* tkrGeoSvc=0;
  MsgStream log(msgSvc(),name());
  if( service( "TkrGeometrySvc", tkrGeoSvc, true).isFailure() ) {
    log << MSG::ERROR << "Couldn't set up TkrGeometrySvc!" << endreq;
    return StatusCode::FAILURE;
  }
  //Loop over the towers
  for(int tower=0;tower<16;tower++){
    const OnboardFilterTds::projections *prjs=status->getProjection(tower);
    HepPoint3D point;
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> xz;
    std::vector<double> yz;
    std::vector<double> zAvg;
    std::vector<double> pointHigh;
    std::vector<double> extendedLow;
    std::vector<double> extendedHigh;
    if(prjs->xy[0]>0 && prjs->xy[1]>0){
      //Loop over the x projections
      for(int xprj=0;xprj<prjs->xy[0];xprj++){
	//Get x,z coordinates for hits 0 and 1
	log << MSG::DEBUG << "Obtaining X and XZ for hits 0 and 1" << endreq;
	point=tkrGeoSvc->getStripPosition(tower,prjs->prjs[xprj].max,0,prjs->prjs[xprj].hits[0]);
	x.push_back(point.x());
	xz.push_back(point.z());
	point=tkrGeoSvc->getStripPosition(tower, prjs->prjs[xprj].max-1, 0, prjs->prjs[xprj].hits[1]);
	x.push_back(point.x());
	xz.push_back(point.z());
	//Loop over the y projections
	for(int yprj=prjs->xy[1];yprj<prjs->xy[1]+prjs->xy[0];yprj++){
	  y.clear();
	  yz.clear();
	  if(prjs->prjs[xprj].max==prjs->prjs[yprj].max){
	    log << MSG::DEBUG << "Obtaining Y and YZ for hits 0 and 1" << endreq;
	    point=tkrGeoSvc->getStripPosition(tower,prjs->prjs[yprj].max,1,prjs->prjs[yprj].hits[0]);
	    y.push_back(point.y());
	    yz.push_back(point.z());
	    point=tkrGeoSvc->getStripPosition(tower,prjs->prjs[yprj].max-1,1,prjs->prjs[yprj].hits[1]);
	    y.push_back(point.y());
	    yz.push_back(point.z());
	    unsigned char maxhits;
	    if(prjs->prjs[xprj].nhits<prjs->prjs[yprj].nhits)
	      maxhits=prjs->prjs[xprj].nhits;
	    else
	      maxhits=prjs->prjs[yprj].nhits;
	    log << MSG::DEBUG << "Obtaining X,Y,XZ,YZ for max hits"<<endreq;
	    point=tkrGeoSvc->getStripPosition(tower,prjs->prjs[xprj].max-(maxhits-1),0,prjs->prjs[xprj].hits[maxhits-1]);
	    x.push_back(point.x());
	    xz.push_back(point.z());
	    point=tkrGeoSvc->getStripPosition(tower,prjs->prjs[yprj].max-(maxhits-1),1,prjs->prjs[yprj].hits[maxhits-1]);
	    y.push_back(point.y());
	    yz.push_back(point.z());
	    for(int counter=0;counter<3;counter++)
	      zAvg.push_back((xz[counter]+yz[counter])/2);
	    double phi,phi_rad;
	    double theta,theta_rad;
	    double length;
	    computeAngles(x,y,xz,yz,zAvg,phi,phi_rad,theta,theta_rad);
	    computeLength(x,y,zAvg,theta_rad,phi_rad,length,pointHigh);
	    computeExtension(x,y,zAvg,phi_rad,theta_rad, extendedLow, extendedHigh);
	    //Add track to TDS
	    OnboardFilterTds::track newTrack;
	    newTrack.phi_rad=phi_rad;
	    newTrack.theta_rad=theta_rad;
	    newTrack.lowCoord.push_back(x[0]);
	    newTrack.lowCoord.push_back(y[0]);
	    newTrack.lowCoord.push_back(zAvg[0]);
	    newTrack.highCoord=pointHigh;
	    newTrack.exLowCoord=extendedLow;
	    newTrack.exHighCoord=extendedHigh;
	    status->setTrack(newTrack);
	    x.clear();
	    y.clear();
	    xz.clear();
	    yz.clear();
	    zAvg.clear();
	    pointHigh.clear();
	    extendedLow.clear();
	    extendedHigh.clear();
	  }
	}
      }
    }
  }
  std::vector<OnboardFilterTds::track> tracks=status->getTracks();
  double maxLength=0;
  unsigned int currMax;
  if(tracks.size()>0){
    for(unsigned int counter=0;counter<tracks.size();counter++){
      if(tracks[counter].length>maxLength){
	maxLength=tracks[counter].length;
	currMax=counter;
      }
    }
    //Obtain McZDir, McXDir, McYDir (copied from McValsTools.cxx)
    SmartDataPtr<Event::McParticleCol> pMcParticle(eventSvc(), EventModel::MC::McParticleCol);
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
	theta_rad_mc=pi - acos(-MC_zdir);
      double x_cord=-MC_xdir;
      double y_cord=-MC_ydir;
      if(y_cord==0)
	phi_rad_mc=pi/2;
      if ((x_cord>0)&&(y_cord>0)){
	phi_rad_mc = atan(y_cord/x_cord);
	if(MC_zdir<0)
	  phi_rad_mc = 3*pi/2 - atan(x_cord/y_cord);
      }
      if ((x_cord<0)&&(y_cord<0)){
	phi_rad_mc = 3*pi/2 - atan(x_cord/y_cord);
	if(MC_zdir<0)
	  phi_rad_mc = atan(y_cord/x_cord);
      }
      if ((x_cord>0)&&(y_cord<0)){
	phi_rad_mc = 2*pi-atan(-y_cord/x_cord);
	if(MC_zdir<0)
	  phi_rad_mc=pi/2 - atan(x_cord/y_cord);
      }
      if ((x_cord<0)&&(y_cord>0)){
	phi_rad_mc = pi/2 - atan(y_cord/(-x_cord));
	if(MC_zdir<0)
	  phi_rad_mc = 2*pi-atan(-y_cord/x_cord);
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
      status->setSeperation(seperation_rad*180/pi);
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

void OnboardFilter::computeAngles(std::vector<double> x,std::vector<double> y,
				  std::vector<double> xz,std::vector<double> yz,std::vector<double> z,
                                  double &phi,double &phi_rad,double &theta,
                                  double &theta_rad){
  const double pi = 3.14159265358979323846;
  double x_h = x[1]-x[0];
  double x_v = xz[0]-xz[1];
  double y_h = y[1]-y[0];
  double y_v = yz[0]-yz[1];
  double z_v = z[0]-z[1];
  double t_h_ave = 1;
  if(x_v== 0 && y_v==0){
    phi_rad=0;
    theta_rad=0;
  }
  else{
    if(x_v ==0){
      phi_rad=pi/2;
      theta_rad=pi/2-atan(y_v/y_h);;
    }
    else{
      if(y_v==0){
	phi_rad=0;
	theta_rad=pi/2-atan(x_v/x_h);
      }
      else{
	if((x_h>0) && (y_h>0)){
	  phi_rad=atan(y_h/x_h);
	  t_h_ave=((x_h/cos(phi_rad)) + (y_h/sin(phi_rad)))/2;
	  theta_rad = pi - atan(t_h_ave/z_v);
	}
	else{
	  if((x_h<0) && (y_h>0)){
	    phi_rad=pi/2-atan(x_h/y_h);
	    t_h_ave = ( -x_h/sin(phi_rad - pi/2) + y_h/cos(phi_rad - pi/2) )/2;
	    theta_rad = pi - atan(t_h_ave/z_v);
	  }
	  else{
	    if((x_h<0) && (y_h<0)){
	      phi_rad=3*pi/2 - atan(x_h/y_h);
	      t_h_ave = ( (-x_h/sin(3*pi/2 - phi_rad)) + (-y_h/cos(3*pi/2 - phi_rad)) )/2;
	      theta_rad = pi - atan(t_h_ave/z_v);
	    }
	    else{
	      if ((x_h>0) && (y_h<0)){
		phi_rad=2*pi-atan(-y_h/x_h);
		t_h_ave = ((x_h/cos(2*pi-phi_rad)) + (-y_h/sin(2*pi-phi_rad)))/2;
		theta_rad = pi - atan(t_h_ave/z_v);
	      }
	      else
		phi_rad=3*pi/2-atan(y_h/x_h);
	    }
	  }
	}
      }
    }
  }
  theta=theta_rad*180/pi;
  phi=phi_rad*180/pi;
}

void OnboardFilter::computeLength(std::vector<double> x,std::vector<double> y, std::vector<double> z,double theta_rad,double phi_rad,
				  double &length, std::vector<double> &endPoint){
  const double pi = 3.14159265358979323846;
  double t_v=z[0]-z[2];
  double t_h = t_v*tan(pi - theta_rad);
  length=sqrt(t_v*t_v+t_h*t_h);
  endPoint.push_back(t_h*cos(phi_rad) + x[0]);
  endPoint.push_back(t_h*sin(phi_rad) + y[0]);
  endPoint.push_back(z[2]);
}

void OnboardFilter::computeExtension(std::vector<double> x,std::vector<double> y,
				     std::vector<double> z, double phi_rad, 
				     double theta_rad, std::vector<double> &extendLow,
 				     std::vector<double> &extendHigh){
  const double pi = 3.14159265358979323846;
  const double length=1000;
  extendLow.push_back(length*sin(pi-theta_rad)*cos(pi+phi_rad) + x[0]);
  extendLow.push_back(length*sin(pi-theta_rad)*sin(pi+phi_rad) + y[0]);
  extendLow.push_back(length*cos(pi-theta_rad) + z[0]);

  extendHigh.push_back(length*sin(theta_rad)*cos(phi_rad)+x[2]);
  extendHigh.push_back(length*sin(theta_rad)*sin(phi_rad)+y[2]);
  extendHigh.push_back(length*cos(theta_rad)+z[2]);
}
