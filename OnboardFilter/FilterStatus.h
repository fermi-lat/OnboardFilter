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

//#include "TFC_projectionDef.h"
/**
* @class FilterStatus
* @brief TDS for storing the information returned by the filter
*/
//extern const CLID& CLID_FilterStatus;

class OnboardFilter;


namespace OnboardFilterTds{
    struct TFC_projection{
        int           intercept; /*!< Intercept at the beginning layer         */
        int               slope; /*!< Slope                                    */
        int          acdTopMask; /*!< ACD top tile candidates                  */
        int            acdXMask; /*!< ACD x facing candidates                  */
        int            acdYMask; /*!< ACD y facing candidates                  */
        unsigned char skirtMask; /*!< Mask of which skirt region prj strikes   */
        unsigned char       min; /*!< Beginning layer number of the projection */
        unsigned char       max; /*!< Ending    layer number of the projection */
        unsigned char     nhits; /*!< Number of hits assigned                  */
        unsigned         layers; /*!< Bit mask representing the struck layers  */
        short int      hits[18]; /*!< Hits assigned to proj                    */
    };

    struct TFC_projections{
        unsigned short int maxCnt; /*!< Maximum number of projections available*/
        unsigned short int curCnt; /*!< Current number of projections in use   */
        unsigned short int  xy[2]; /*!< Count of X/Y projections               */
        TFC_projection prjs[1000]; /*!< List of projections                    */
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
        std::vector<TFC_projection> getProjection(int tower)const ;

        virtual std::ostream& fillStream(std::ostream &s) const;
        friend std::ostream& operator << (std::ostream &s, const FilterStatus &obj);

        // only this guy can create one, or modify it
        friend class OnboardFilter;
    private:
        FilterStatus();
        FilterStatus(const unsigned int code, const int energey=0,
            const int ids=0, const int xz=0, const int yz=0,
            const int xy=0, const int *acdstatus=NULL,
            const int *layerCode=NULL,
            const TFC_projections *projections=NULL);
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
        void setProjection(const int tower,const TFC_projections projections);

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
        std::vector<TFC_projection> m_prjs[16];
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
    inline std::vector<TFC_projection> FilterStatus::getProjection(int tower)const{
        return m_prjs[tower];
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
      }
    }
    inline FilterStatus::FilterStatus(const unsigned int code, const int energy,
						   const int ids, const int xz, const int yz,
						   const int xy, const int *acdstatus,
						   const int *layerCode,
						   const TFC_projections *projections){
      m_status=code;
      m_calEnergy=energy;
      m_tcids=ids;
      m_acd_xz=xz;
      m_acd_xy=xy;
      m_acd_yz=yz;
      for(int counter=0;counter<16;counter++){
		m_acdStatus[counter]=0;
		m_layers[counter]=0;
      }
      if(acdstatus!=NULL){
	for(int counter=0;counter<16;counter++)
	  m_acdStatus[counter]=acdstatus[counter];
      }
      if(layerCode!=NULL){
        for(int counter=0;counter<16;counter++)
	  m_layers[counter]=layerCode[counter];
      }
      if(projections!=NULL){
	for(int counter=0;counter<16;counter++){
	  for(int trackCounter=0;trackCounter<projections[counter].curCnt;trackCounter++){
	    m_prjs[counter].push_back(projections[counter].prjs[trackCounter]);
	  }
	}
      }
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


    inline void FilterStatus::setProjection(const int tower,const TFC_projections prjs){
      if(tower<16){
	m_prjs[tower].clear();
	for(int counter=0;counter<prjs.curCnt;counter++)
	  m_prjs[tower].push_back(prjs.prjs[counter]);
      }
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
