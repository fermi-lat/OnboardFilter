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

/**
* @class FilterStatus
* @brief TDS for storing the information returned by the filter
*/
//extern const CLID& CLID_FilterStatus;

class OnboardFilter;


namespace OnboardFilterTds{
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
        };
        double phi_rad,theta_rad;
        std::vector<double> lowCoord;
        std::vector<double> highCoord;
        std::vector<double> exLowCoord;
        std::vector<double> exHighCoord;
        double length;
    };

    struct projection{
        unsigned char       min; /*!< Beginning layer number of the projection */
        unsigned char       max; /*!< Ending    layer number of the projection */
        unsigned         layers; /*!< Bit mask representing the struck layers  */
        unsigned char     nhits; /*!< Number of hits assigned                  */
        short int      hits[18]; /*!< Hits assigned to proj                    */
    };

    struct projections{
        unsigned short int curCnt; /*!< Current number of projections in use   */
        unsigned short int  xy[2]; /*!< Count of X/Y projections               */
        projection prjs[1000];     /*!< List of projections                    */
    };


    class FilterStatus : public DataObject{
    public:
        virtual ~FilterStatus();


        ///Return the statuscode of the filter
        unsigned int get() const;

        ///Returns the 16 most significant bits of the filter code
        unsigned int getHigh() const;

        ///Returns the 16 least signifcat bits of the filter code
        unsigned int getLow() const;

        ///Returns the value stored in CalEnergy
        int getCalEnergy() const;

        ///Return the Code specifying the towers with triggers or possible triggers
        int getTcids() const;

        ///Return the ACD hit map results
        void getAcdMap(int &xz, int &yz, int &xy) const;

        ///Return the ACD faces intersected by projections
        void getAcdStatus(int *copy) const;
        
        ///Return pointer to array of layers that were hit in each tower
        const int *getLayers()const ;

        ///Return the projections for a specific tower
        const projections *getProjection(int tower)const ;

	///Return all available tracks
        std::vector<track> getTracks()const;

	///Return angular seperation between best track and incomming particle
        double getSeperation() const;

        virtual std::ostream& fillStream(std::ostream &s) const;
        friend std::ostream& operator << (std::ostream &s, const FilterStatus &obj);

        // only this guy can create one, or modify it
        friend class OnboardFilter;

        FilterStatus();

        ///Set the statuscode of the filter
        void set(const unsigned int code);

        ///Set the Energy in CAL
        void setCalEnergy(const int energy);

        ///Set the Code specifying the towers with triggers or possible triggers
        void setTcids(const int ids);

        ///Set the ACD hit map results
        void setAcdMap(const int xz, const int yz, const int xy);
        ///Set the ACD faces intersected by projections
        void setAcdStatus(const int tower, const int status);

        ///Set what layers were hit in each tower
        void setLayers(const int *layerCode);
        ///Set the projection of a specific tower
        void setProjection(const int tower,const projections &projections);
	
	///Add a new track to the list of tracks
	void setTrack(const track &newTrack);

        void setSeperation(const double sep);

    private:
        ///Filter status code
        unsigned int m_status;
        ///Energy in CAL
        int m_calEnergy;
        ///Towers with triggers
        int m_tcids;
        ///ACD hit map
        int m_acd_xz;
        int m_acd_yz;
        int m_acd_xy;
        ///ACD faces
        int m_acdStatus[16];
        ///Layers hit in each tower
        int m_layers[16];
        ///Projections for the towers
        projections m_prjs[16];
	///Tracks found for this event
	std::vector<track> m_tracks;
	///Angular seperation between best track and incomming particle
	double m_seperation;
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
    inline void FilterStatus::getAcdMap(int &xz, int &yz, int &xy) const {
        xz=m_acd_xz;
        yz=m_acd_yz;
        xy=m_acd_xy;
    }
    inline void FilterStatus::getAcdStatus(int *copy) const {
        memcpy(copy,m_acdStatus,sizeof(m_acdStatus)*16);
    }
    inline const projections * FilterStatus::getProjection(int tower)const{
        return &m_prjs[tower];
    }
    inline int FilterStatus::getCalEnergy() const{
        return m_calEnergy;
    }
    inline int FilterStatus::getTcids()const{
        return m_tcids;
    }
    inline const int * FilterStatus::getLayers()const {
        return m_layers;
    }

    inline std::vector<track> FilterStatus::getTracks()const{
      return m_tracks;
    }

    inline double FilterStatus::getSeperation() const{
      return m_seperation;
    }

    // the put methods, here for now
    inline FilterStatus::FilterStatus(){
      m_status=0;
      m_calEnergy=0;
      m_tcids=0;
      m_acd_xy=0;
      m_acd_xz=0;
      m_acd_yz=0;
      for(int counter=0;counter<16;counter++){
	m_acdStatus[counter]=0;
	m_layers[counter]=0;
	m_prjs[counter].xy[0]=0;
	m_prjs[counter].xy[1]=0;
	m_prjs[counter].curCnt=0;
      }
      m_seperation=-1;
      m_tracks.clear();
    }

    inline FilterStatus::~FilterStatus(){
    }

    inline void FilterStatus::set(const unsigned int code){
      m_status=code;
    }

    inline void FilterStatus::setCalEnergy(const int energy){
      m_calEnergy=energy;
    }

    inline void FilterStatus::setTcids(const int ids){
      m_tcids=ids;
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

    inline void FilterStatus::setProjection(const int tower,const projections &prjs){
      if(tower<16){
	memcpy(&m_prjs[tower], &prjs,sizeof(prjs));
      }
    }

    inline void FilterStatus::setTrack(const track &newTrack){
      m_tracks.push_back(newTrack);
    }

    inline void FilterStatus::setSeperation(const double sep){
      m_seperation=sep;
    }

    inline std::ostream& FilterStatus::fillStream(std::ostream &s) const{
      s<<"Filter Return Code: "<<m_status<<std::endl;
      s<<"Filter code for Energy in CAL: "<<m_calEnergy<<std::endl;
      return s;
    }


    inline std::ostream& operator<<(std::ostream &s, const FilterStatus &obj){
      return obj.fillStream(s);
    }

}//namespace OnboradFilterTds 


#endif
