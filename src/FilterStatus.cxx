#include "OnboardFilter/FilterStatus.h"

using namespace OnboardFilterTds;

FilterStatus::FilterStatus(){
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

FilterStatus::FilterStatus(const unsigned int code, const int energy,
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

FilterStatus::~FilterStatus(){
}

void FilterStatus::set(const unsigned int code){
    m_status=code;
}

void FilterStatus::setCalEnergy(const int energy){
    m_calEnergy=energy;
}

int FilterStatus::getCalEnergy() const{
    return m_calEnergy;
}

void FilterStatus::setTcids(const int ids){
    m_tcids=ids;
}

int FilterStatus::getTcids()const{
    return m_tcids;
}

void FilterStatus::setAcdMap(const int xz, const int yz, const int xy){
    m_acd_xz=xz;
    m_acd_yz=yz;
    m_acd_xy=xy;
}

void FilterStatus::getAcdMap(int &xz, int &yz, int &xy) const {
    xz=m_acd_xz;
    yz=m_acd_yz;
    xy=m_acd_xy;
}


void FilterStatus::setAcdStatus(const int tower, const int status){
	if(tower<16)
        m_acdStatus[tower]=status;
}

void FilterStatus::getAcdStatus(int *copy) const {
    memcpy(copy,m_acdStatus,sizeof(m_acdStatus)*16);
}

void FilterStatus::setLayers(const int *layerCode){
    for(int counter=0;counter<16;counter++)
        m_layers[counter]=layerCode[counter];
}

int * FilterStatus::getLayers(){
    return m_layers;
}

void FilterStatus::setProjection(const int tower,const TFC_projections prjs){
	if(tower<16){
	    m_prjs[tower].clear();
	    for(int counter=0;counter<prjs.curCnt;counter++)
		    m_prjs[tower].push_back(prjs.prjs[counter]);
	}
}

std::vector<TFC_projection> FilterStatus::getProjection(const int tower){
	return m_prjs[tower];
}

inline std::ostream& FilterStatus::fillStream(std::ostream &s) const{
    s<<"Filter Return Code: "<<m_status<<std::endl;
    s<<"Filter code for Energy in CAL: "<<m_calEnergy<<std::endl;
    return s;
}

std::ostream& operator<<(std::ostream &s, const FilterStatus &obj){
    return obj.fillStream(s);
}
