#ifndef FILTER_STATUS_H
#define FILTER_STATUS_H

#include <iostream>
#include <vector>

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/ObjectVector.h"

#include "Event/TopLevel/Definitions.h"
#include "Event/TopLevel/EventModel.h"

#include "TkrUtil/ITkrGeometrySvc.h"
#include "EFC/TFC_projectionDef.h"
#include "EDS/EDR_tkr.h"
#include "EFC/GFC_status.h"
#include "OnboardFilter/LogInfoDef.h"
/**
 * @class FilterStatus
 * @brief TDS for storing the information returned by the filter
 */
//extern const CLID& CLID_FilterStatus;

class OnboardFilter;


namespace OnboardFilterTds{
  class TowerHits : public DataObject{
  public:
    struct towerRecord{
      unsigned char lcnt[2];
      int layerMaps[2];
      unsigned char cnt[36];
      short int *beg[36];
    };
    inline TowerHits(){
      for(int counter=0;counter<16;counter++){
        for(int layerCounter=0;layerCounter<36;layerCounter++){
          m_hits[counter].cnt[layerCounter]=0;
          m_hits[counter].beg[layerCounter]=NULL;
        }
      }
    };
    inline ~TowerHits(){
      for(int counter=0;counter<16;counter++){
        for(int layerCounter=0;layerCounter<36;layerCounter++){
          if(m_hits[counter].beg[layerCounter]){
            delete[] m_hits[counter].beg[layerCounter];
          }
        }
      }
    };
    inline TowerHits(const TowerHits &toCopy){
      for(int counter=0;counter<16;counter++){
        m_hits[counter].lcnt[0]=toCopy.m_hits[counter].lcnt[0];
        m_hits[counter].lcnt[1]=toCopy.m_hits[counter].lcnt[1];
        m_hits[counter].layerMaps[0]=toCopy.m_hits[counter].layerMaps[0];
        m_hits[counter].layerMaps[1]=toCopy.m_hits[counter].layerMaps[1];
        for(int layerCounter=0;layerCounter<36;layerCounter++){
          m_hits[counter].cnt[layerCounter]=toCopy.m_hits[counter].cnt[layerCounter];
          m_hits[counter].beg[layerCounter]=new short int[m_hits[counter].cnt[layerCounter]];
          memcpy(m_hits[counter].beg[layerCounter],toCopy.m_hits[counter].beg[layerCounter],m_hits[counter].cnt[layerCounter]*sizeof(short int));
        }
      }
    };
    inline const towerRecord* get(){
      return m_hits;
    };
    inline void set(towerRecord* hits){
      for(int counter=0;counter<16;counter++){
        m_hits[counter].lcnt[0]=hits[counter].lcnt[0];
        m_hits[counter].lcnt[1]=hits[counter].lcnt[1];
        m_hits[counter].layerMaps[0]=hits[counter].layerMaps[0];
        m_hits[counter].layerMaps[1]=hits[counter].layerMaps[1];
        for(int layerCounter=0;layerCounter<36;layerCounter++){
          m_hits[counter].cnt[layerCounter]=hits[counter].cnt[layerCounter];
          if(m_hits[counter].beg[layerCounter]){
            delete[] m_hits[counter].beg[layerCounter];
          }
          m_hits[counter].beg[layerCounter]=new short int[m_hits[counter].cnt[layerCounter]];
          memcpy(m_hits[counter].beg[layerCounter],hits[counter].beg[layerCounter],m_hits[counter].cnt[layerCounter]*sizeof(short int));
        }
      }
    };
    towerRecord m_hits[16];
  };
  class track{
  public:
    inline track(){};
    inline track(const track &copy){
      lowCoord=copy.lowCoord;
      highCoord=copy.highCoord;
      exLowCoord=copy.exLowCoord;
      exHighCoord=copy.exHighCoord;
      length=copy.length;
      phi_rad=copy.phi_rad;
      theta_rad=copy.theta_rad;
      numhits=copy.numhits;
    };
    int numhits;
    double phi_rad,theta_rad;
    std::vector<double> lowCoord;
    std::vector<double> highCoord;
    std::vector<double> exLowCoord;
    std::vector<double> exHighCoord;
    double length;
  };


  class FilterStatus : public DataObject{
  public:
    virtual ~FilterStatus();


    ///Return the statuscode of the filter
    inline unsigned int get() const;
    ///Returns the 16 most significant bits of the filter code
    unsigned int getHigh() const;
    ///Returns the 16 least signifcat bits of the filter code
    unsigned int getLow() const;
    ///Returns the value stored in CalEnergy
    float getCalEnergy() const;
    const int getNumLogsHit() const;
    const LogInfo* getLogInfo() const;
    int getEbfSize() const;
    int getStageEnergy() const;
    ///Return the Code specifying the towers with triggers or possible triggers
    int getTcids() const;
    ///Return the ACD hit map results
    void getAcdMap(int &xz, int &yz, int &xy) const;
    ///Return the ACD faces intersected by projections
    void getAcdStatus(int *copy) const;
    ///Return pointer to array of layers that were hit in each tower
    const int *getLayers()const ;
    ///Return the projections for a specific tower
    //const projections *getProjection(int tower)const ;
    TFC_projections *getProjections() ;
    EDR_tkr *getTkr() ;
    ///Return all available tracks
    std::vector<track> getTracks()const;
    ///Return angular separation between best track and incomming particle
    double getSeparation() const;
    bool tracksExist() const;
    const int* getXcapture() const;
    const int* getYcapture() const;
    ///return the Cal Layer Energy
    const float* getLayerEnergy() const;
    ///Return xy00
    const int* getXY00() const;
    ///Return xy11
    const int* getXY11() const;
    ///Return xy22
    const int* getXY22() const;
    ///Return xy33
    const int* getXY33() const;
    ///Return tmsk
    int getTmsk() const;
    int getVetoWord() const;
    void setVetoWord(const int vetoword);
    virtual std::ostream& fillStream(std::ostream &s) const;
    friend std::ostream& operator << (std::ostream &s, const FilterStatus &obj);
    // only this guy can create one, or modify it
    friend class OnboardFilter;
    FilterStatus();

    ///Set the statuscode of the filter
    void set(const unsigned int code);
    ///Set the Energy in CAL
    ///Set the Energy in CAL          
    void setStageEnergy(const int energy);
    void setEbfSize(const int ids);
    ///Set the Code specifying the towers with triggers or possible triggers
    void setTcids(const int ids);
    ///Set the ACD hit map results
    void setAcdMap(const int xz, const int yz, const int xy);
    ///Set the ACD faces intersected by projections
    void setAcdStatus(const int tower, const int status);
    ///Set what layers were hit in each tower
    void setLayers(const int *layerCode);
    ///Set the projection of a specific tower
    //void setProjection(const int tower,const projections &projections);
    void setProjections(const TFC_projections &projections);
    void setTkr(const EDR_tkr &tkr);
    ///Add a new track to the list of tracks
    void setTrack(const track &newTrack);
    void setSeparation(const double sep);
    void setLayerEnergy(const int energy[8]);
    void setLogData(int num, LogInfo tempLogData[]);

    void setCapture(const int xcapture[16], const int ycapture[16]);
    void setXY(const int xy00[16], const int xy11[16], const int xy22[16], const int xy33[16]);
    void setTmsk(int tmsk);
    //****TEMP
    void setHoughSep(const double houghsep);
    void setHighLayer(const double highestlayer);

    void setSeparation2(const double sep2);
    void setXlongest(const double x_report);
    void setYlongest(const double y_report);
    void setXavg(const double xavg);
    void setYavg(const double yavg);
    void setXslopeL(const double xslopeL);
    void setYslopeL(const double yslopeL);
    void setXslopeAvg(const double xslopeAvg);
    void setYslopeAvg(const double yslopeAvg);
    void setXfirst(const double long_firstanglex);
    void setYfirst(const double long_firstangley);
    void setXtower(const double xslopetower);
    void setYtower(const double yslopetower);
    void setXangleL(const double xangleL);
    void setYangleL(const double yangleL);
    void setXlongestB(const double xlongestB);
    void setYlongestB(const double ylongestB);
    void setBestTrack(int xHits,int yHits,
                              double slopeXZ,double slopeYZ,
                              double intXZ,double intYZ);

    void getBestTrack(int &xHits,int &yHits,
                              double &slopeXZ,double &slopeYZ,
                              double &intXZ,double &intYZ);
 
    double getHoughSep() const;
    double getHighLayer() const;

    double getSeparation2() const;
    double getXlongest() const;
    double getYlongest() const;
    double getXavg() const;
    double getYavg() const;
    double getXslopeL() const;
    double getYslopeL() const;
    double getXslopeAvg() const;
    double getYslopeAvg() const;
    double getXfirst() const;
    double getYfirst() const;
    double getXtower() const;
    double getYtower() const;
    double getXangleL() const;
    double getYangleL() const;
    double getXlongestB() const;
    double getYlongestB() const;

   int getGemThrTkr()const;
   
    int getGemCalHiLo() const;
    int getGemCondsumCno() const;
    int getGemAcd_vetoes_XZ() const;
    int getGemAcd_vetoes_YZ() const;
    int getGemAcd_vetoes_XY() const;
    int getGemAcd_vetoes_RU() const;
    int getGemLivetime() const;
    int getGemTrgtime() const;
    int getGemPpstime() const;
    int getGemDiscarded() const;
    int getGemPrescaled() const;
    int getGemSent()const;

    
   void setGemThrTkr(const int val);
   void setGemCalHiLo(const int val);
   void setGemCondsumCno(const int val);
   void setGemAcd_vetoes_XZ(const int val);
   void setGemAcd_vetoes_YZ(const int val);
   void setGemAcd_vetoes_XY(const int val);
   void setGemAcd_vetoes_RU(const int val);
   void setGemLivetime(const int val);
   void setGemTrgtime(const int val);
   void setGemPpstime(const int val);
   void setGemDiscarded(const int val);
   void setGemPrescaled(const int val);
   void setGemSent(const int val);


    //****TEMP

  private:
    ///Filter status code
    unsigned int m_status;
    ///Energy in CAL
    int m_stageEnergy;
    ///Towers with triggers
    int m_tcids;

   int m_ebfsize;
    ///Gem info
   int m_gem_thrTkr;  
   int m_gem_calHiLo;  
   int m_gem_condsumCno;
   int m_gem_acd_vetoes_XZ;
   int m_gem_acd_vetoes_YZ;
   int m_gem_acd_vetoes_XY;
   int m_gem_acd_vetoes_RU;
   int m_gem_livetime;
   int m_gem_trgtime;   
   int m_gem_ppstime;
   int m_gem_discarded; 
   int m_gem_prescaled; 
   int m_gem_sent;
    LogInfo m_logData[16*8*12];    // 16 towers * 8 layers * 12 logs

//
// BestTRack info
    int m_xHits;
    int m_yHits;
    double m_slopeXZ;
    double m_slopeYZ;
    double m_intXZ;
    double m_intYZ;

   int m_numLogsHit;
    ///ACD hit map
    int m_acd_xz;
    int m_acd_yz;
    int m_acd_xy;
    int m_vetoword;
    ///ACD faces
    int m_acdStatus[16];
    ///Layers hit in each tower
    int m_layers[16];
    ///Projections for the towers
    //projections m_prjs[16];
    TFC_projections m_prjs;
    EDR_tkr         m_tkr;
    ///Tracks found for this event
    std::vector<track> m_tracks;
    ///Angular separation between best track and incomming particle
    double m_separation;

    float m_layerEnergy[8];
    int m_xcapture[16];
    int m_ycapture[16];
    int m_xy00[16],m_xy11[16],m_xy22[16],m_xy33[16];
    int m_tmsk;
    //****TEMP
    double m_separation2;
    double m_x_report;  double m_y_report;
    double m_xavg;      double m_yavg;
    double m_xslopeL;   double m_yslopeL;
    double m_xslopeAvg; double m_yslopeAvg;
    double m_long_firstanglex; double m_long_firstangley;
    double m_xslopetower; double m_yslopetower;
    double m_xangleL;    double m_yangleL;
    double m_xlongestB;  double m_ylongestB;
    double m_highestlayer;

    double m_houghsep;
    //****TEMP

  };

  // inline the public get methods for clients besides OnboardFilter.

  inline unsigned int FilterStatus::get() const{
    return m_status;
  }

  inline unsigned int FilterStatus::getHigh() const{
    return m_status>>15;
  }

  inline unsigned int FilterStatus::getLow() const{
    return m_status & 0x7FFF;
  }

   inline int FilterStatus::getGemThrTkr()const{
    return m_gem_thrTkr;
  }
   
   inline int FilterStatus::getGemCalHiLo()const{
    return m_gem_calHiLo;
  } 
   inline int FilterStatus::getGemCondsumCno()const{
    return m_gem_condsumCno;
  }
   inline int FilterStatus::getGemAcd_vetoes_XZ()const{
    return m_gem_acd_vetoes_XZ;
  }
   inline int FilterStatus::getGemAcd_vetoes_YZ()const{
    return m_gem_acd_vetoes_YZ;
  }
   inline int FilterStatus::getGemAcd_vetoes_XY()const{
    return m_gem_acd_vetoes_XY;
  }
   inline int FilterStatus::getGemAcd_vetoes_RU()const{
    return m_gem_acd_vetoes_RU;
  }
   inline int FilterStatus::getGemLivetime()const{
    return m_gem_livetime;
  }
   inline int FilterStatus::getGemTrgtime()const{
    return m_gem_trgtime;
  }  
   inline int FilterStatus::getGemPpstime()const{
    return m_gem_ppstime;
  }
   inline int FilterStatus::getGemDiscarded()const{
    return m_gem_discarded;
  } 
   inline int FilterStatus::getGemPrescaled()const{
    return m_gem_prescaled;
  } 
   inline int FilterStatus::getGemSent()const{
    return m_gem_sent;
  }


  inline void FilterStatus::getAcdMap(int &xz, int &yz, int &xy) const {
    xz=m_acd_xz;
    yz=m_acd_yz;
    xy=m_acd_xy;
  }
  inline void FilterStatus::getAcdStatus(int *copy) const {
    memcpy(copy,m_acdStatus,sizeof(m_acdStatus)*16);
  }

  inline TFC_projections * FilterStatus::getProjections(){
    return &m_prjs;
  }
  inline EDR_tkr * FilterStatus::getTkr(){
    return &m_tkr;
  }
  inline float FilterStatus::getCalEnergy() const{
    return (float)((m_stageEnergy & GFC_STAGE_M_ENERGY)/4.0);
  }
  inline int FilterStatus::getStageEnergy() const{
    return m_stageEnergy;
  }
  inline int FilterStatus::getTcids()const{
    return m_tcids;
  }
  inline int FilterStatus::getEbfSize()const{
    return m_ebfsize;
  }
  inline const int * FilterStatus::getLayers()const {
    return m_layers;
  }

  inline std::vector<track> FilterStatus::getTracks()const{
    return m_tracks;
  }

  inline double FilterStatus::getSeparation() const{
    return m_separation;
  }

  inline bool FilterStatus::tracksExist() const{
    if(m_tracks.size()!=0)
      return true;
    return false;
  }

  inline const int* FilterStatus::getXcapture() const{
    return m_xcapture;
  }

  inline const int* FilterStatus::getYcapture() const{
    return m_ycapture;
  }
   
     inline const int FilterStatus::getNumLogsHit() const{
         return m_numLogsHit;
           }
     inline const LogInfo* FilterStatus::getLogInfo() const{
         return m_logData;
           }
           
  inline const float* FilterStatus::getLayerEnergy() const{
    return m_layerEnergy;
  }

  inline const int* FilterStatus::getXY00() const{
    return m_xy00;
  }

  inline const int* FilterStatus::getXY11() const{
    return m_xy11;
  }

  inline const int* FilterStatus::getXY22() const{
    return m_xy22;
  }

  inline const int* FilterStatus::getXY33() const{
    return m_xy33;
  }

  inline int FilterStatus::getTmsk() const{
    return m_tmsk;
  }

  inline int FilterStatus::getVetoWord() const{
    return m_vetoword;
  }

  inline void FilterStatus::setVetoWord(const int vetoword){
    m_vetoword=vetoword;
  }

  inline FilterStatus::FilterStatus(){
    m_status=0;
    m_stageEnergy=0;
    m_tcids=0;
    m_ebfsize=0;
    m_acd_xy=0;
    m_acd_xz=0;
    m_acd_yz=0;
    m_numLogsHit = 0;
    for(int counter=0;counter<16;counter++){
      m_acdStatus[counter]=0;
      m_layers[counter]=0;
      m_prjs.curCnt=0;
      m_prjs.twrMsk=0;
    }
    m_separation=-1;
    m_tracks.clear();
  }

  inline FilterStatus::~FilterStatus(){
  }

  inline void FilterStatus::set(const unsigned int code){
    m_status=code;
  }

  inline void FilterStatus::setBestTrack(
                              int xHits,int yHits,
                              double slopeXZ,double slopeYZ,
                              double intXZ,double intYZ){
    m_xHits    = xHits;
    m_yHits    = yHits;
    m_slopeXZ  = slopeXZ;
    m_slopeYZ  = slopeYZ;
    m_intXZ    = intXZ;
    m_intYZ    = intYZ;
  }

  inline void FilterStatus::getBestTrack(
                              int &xHits,int &yHits,
                              double &slopeXZ,double &slopeYZ,
                              double &intXZ,double &intYZ){
    xHits    = m_xHits;
    yHits    = m_yHits;
    slopeXZ  = m_slopeXZ;
    slopeYZ  = m_slopeYZ;
    intXZ    = m_intXZ;
    intYZ    = m_intYZ;
  }


  inline void FilterStatus::setStageEnergy(const int stageEnergy){
    m_stageEnergy=stageEnergy;//must divide by 4 to get MeV units
  }

  inline void FilterStatus::setEbfSize(const int ebfsize){
    m_ebfsize = ebfsize;
  }

  inline void FilterStatus::setTcids(const int ids){
    m_tcids=ids;
  }


  inline void FilterStatus::setGemThrTkr(const int val){
    m_gem_thrTkr = val;
  }
   
   inline void FilterStatus::setGemCalHiLo(const int val){
    m_gem_calHiLo = val;
  } 
   inline void FilterStatus::setGemCondsumCno(const int val){
    m_gem_condsumCno = val;
  }
   inline void FilterStatus::setGemAcd_vetoes_XZ(const int val){
    m_gem_acd_vetoes_XZ = val;
  }
   inline void FilterStatus::setGemAcd_vetoes_YZ(const int val){
    m_gem_acd_vetoes_YZ = val;
  }
   inline void FilterStatus::setGemAcd_vetoes_XY(const int val){
    m_gem_acd_vetoes_XY = val;
  }
   inline void FilterStatus::setGemAcd_vetoes_RU(const int val){
    m_gem_acd_vetoes_RU = val;
  }
   inline void FilterStatus::setGemLivetime(const int val){
    m_gem_livetime = val;
  }
   inline void FilterStatus::setGemTrgtime(const int val){
    m_gem_trgtime = val;
  }  
   inline void FilterStatus::setGemPpstime(const int val){
    m_gem_ppstime = val;
  }
   inline void FilterStatus::setGemDiscarded(const int val){
    m_gem_discarded = val;
  } 
   inline void FilterStatus::setGemPrescaled(const int val){
    m_gem_prescaled = val;
  } 
   inline void FilterStatus::setGemSent(const int val){
    m_gem_sent = val;
  }


  inline void FilterStatus::setAcdMap(const int xz, const int yz, const int xy){
    m_acd_xz=xz;
    m_acd_yz=yz;
    m_acd_xy=xy;
  }

  inline void FilterStatus::setAcdStatus(const int tower, const int status){
    if(tower<16)
      m_acdStatus[tower]=status;
  }

  inline void FilterStatus::setLayers(const int *layerCode){
    for(int counter=0;counter<16;counter++)
      m_layers[counter]=layerCode[counter];
  }

  inline void FilterStatus::setProjections(const TFC_projections &prjs){
    memcpy(&m_prjs, &prjs,sizeof(prjs));
  }

  inline void FilterStatus::setTkr(const EDR_tkr &tkr){
    memcpy(&m_tkr, &tkr,sizeof(tkr));
  }

  inline void FilterStatus::setSeparation(const double sep){
    m_separation=sep;
  }

  inline void FilterStatus::setTrack(const track &newTrack){
    m_tracks.push_back(newTrack);
  }

  inline void FilterStatus::setLayerEnergy(const int energy[8]){
    for(int counter=0;counter<8;counter++)
      m_layerEnergy[counter]=((float)energy[counter])/4.;//must divide by 4 to get MeV units
  }
   
   inline void FilterStatus::setLogData(int num, LogInfo tempLogData[]){
      m_numLogsHit = num;
      for (int i=0; i<num; i++) {
         m_logData[i] = tempLogData[i];
      }
   }
                   
  inline void FilterStatus::setCapture(const int xcapture[16], const int ycapture[16]){
    for(int counter=0;counter<16;counter++){
      m_xcapture[counter]=xcapture[counter];
      m_ycapture[counter]=ycapture[counter];
    }
  }

  inline void FilterStatus::setXY(const int xy00[16], const int xy11[16], const int xy22[16], const int xy33[16]){
    for(int counter=0;counter<16;counter++){
      m_xy00[counter]=xy00[counter];
      m_xy11[counter]=xy11[counter];
      m_xy22[counter]=xy22[counter];
      m_xy33[counter]=xy33[counter];
    }
  }

  inline void FilterStatus::setTmsk(int tmsk){
    m_tmsk=tmsk;
  }
  //****TEMP
  inline void FilterStatus::setHoughSep(const double houghsep){
      m_houghsep=houghsep;
  }
  inline double FilterStatus::getHoughSep() const{
      return m_houghsep;
  }

  inline void FilterStatus::setHighLayer(const double highestlayer){
      m_highestlayer=highestlayer;
  }
  inline double FilterStatus::getHighLayer() const{
      return m_highestlayer;
  }

  inline void FilterStatus::setSeparation2(const double sep2){
    m_separation2=sep2;
  }
  inline double FilterStatus::getSeparation2() const{
    return m_separation2;
  }
  inline void FilterStatus::setXlongest(const double x_report){
    m_x_report=x_report;
  }
  inline void FilterStatus::setYlongest(const double y_report){
    m_y_report=y_report;
  }
  inline void FilterStatus::setXavg(const double xavg){
    m_xavg=xavg;
  }
  inline void FilterStatus::setYavg(const double yavg){
    m_yavg=m_yavg;
  }
  inline void FilterStatus::setXfirst(const double long_firstanglex){
    m_long_firstanglex=long_firstanglex;
  }
  inline void FilterStatus::setYfirst(const double long_firstangley){
    m_long_firstangley=long_firstangley;
  }

  inline void FilterStatus::setXslopeL(const double xslopeL){
    m_xslopeL=xslopeL;
  }
  inline void FilterStatus::setYslopeL(const double yslopeL){
    m_yslopeL=yslopeL;
  }
  inline void FilterStatus::setXslopeAvg(const double xslopeAvg){
    m_xslopeAvg=xslopeAvg;
  }
  inline void FilterStatus::setYslopeAvg(const double yslopeAvg){
    m_yslopeAvg=yslopeAvg;
  }
  inline void FilterStatus::setXtower(const double xslopetower){
    m_xslopetower=xslopetower;
  }
  inline void FilterStatus::setYtower(const double yslopetower){
    m_yslopetower=yslopetower;
  }
  inline void FilterStatus::setXangleL(const double xangleL){
    m_xangleL=xangleL;
  }
  inline void FilterStatus::setYangleL(const double yangleL){
    m_yangleL=yangleL;
  }
  inline void FilterStatus::setXlongestB(const double xlongestB){
    m_xlongestB=xlongestB;
  }
  inline void FilterStatus::setYlongestB(const double ylongestB){
    m_ylongestB=ylongestB;
  }

  inline double FilterStatus::getXlongest() const{
    return m_x_report;
  }
  inline double FilterStatus::getYlongest() const{
    return m_y_report;
  }
  inline double FilterStatus::getXavg() const{
    return m_xavg;
  }
  inline double FilterStatus::getYavg() const{
    return m_yavg;
  }

  inline double FilterStatus::getXslopeL() const{
    return m_xslopeL;
  }
  inline double FilterStatus::getYslopeL() const{
    return m_yslopeL;
  }
  inline double FilterStatus::getXslopeAvg() const{
    return m_xslopeAvg;
  }
  inline double FilterStatus::getYslopeAvg() const{
    return m_yslopeAvg;
  }
  inline double FilterStatus::getXtower() const{
    return m_xslopetower;
  }
  inline double FilterStatus::getYtower() const{
    return m_yslopetower;
  }

  inline double FilterStatus::getXfirst() const{
    return m_long_firstanglex;
  }
  inline double FilterStatus::getYfirst() const{
    return m_long_firstangley;
  }
  inline double FilterStatus::getXangleL() const{
    return m_xangleL;
  }
  inline double FilterStatus::getYangleL() const{
    return m_yangleL;
  }
  inline double FilterStatus::getXlongestB() const{
    return m_xlongestB;
  }
  inline double FilterStatus::getYlongestB() const{
    return m_ylongestB;
  }
  //****TEMP

  inline std::ostream& FilterStatus::fillStream(std::ostream &s) const{
    s<<"Filter Return Code: "<<m_status<<std::endl;
    s<<"Filter code for StageEnergy in CAL: "<<m_stageEnergy<<std::endl;
    return s;
  }


  inline std::ostream& operator<<(std::ostream &s, const FilterStatus &obj){
    return obj.fillStream(s);
  }

}//namespace OnboradFilterTds


#endif
