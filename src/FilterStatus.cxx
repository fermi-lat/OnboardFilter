#include "OnboardFilter/FilterStatus.h"

using namespace OnboardFilterTds;

FilterStatus::FilterStatus(){
    m_status=0;
    m_calEnergy=0;
    m_tcids=0;
    m_acd_xy=0;
    m_acd_xz=0;
    m_acd_yz=0;
    m_acdStatus=0;
    for(int counter=0;counter<16;counter++)
        m_layers[counter]=0;
}

FilterStatus::FilterStatus(const unsigned int code, const int energy,
						   const int ids, const int xz, const int yz,
						   const int xy, const int acdstatus,
						   const int *layerCode){
    m_status=code;
    m_calEnergy=energy;
    m_tcids=ids;
    m_acd_xz=xz;
    m_acd_xy=xy;
    m_acd_yz=yz;
    m_acdStatus=acdstatus;
    if(m_layers!=NULL){
        for(int counter=0;counter<16;counter++)
            layers[counter]=layerCode[counter];
    }
}

FilterStatus::~FilterStatus(){
}

void FilterStatus::set(const unsigned int code){
    m_status=code;
}

unsigned int FilterStatus::get() const{
    return m_status;
}

unsigned int FilterStatus::getLow() const{
    return m_status & 0x7FFF;
}

unsigned int FilterStatus::getHigh() const{
    return m_status>>15;
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
    m_xz=acd_xz;
    m_yz=acd_yz;
    m_xy=acd_xy;
}


void FilterStatus::setAcdStatus(const int status){
    m_acdStatus=status;
}

int FilterStatus::getAcdStatus() const {
    return m_acdStatus;
}

void FilterStatus::setLayers(const int *layerCode){
    for(int counter=0;counter<16;counter++)
        m_layers[counter]=layerCode[counter];
}

int * FilterStatus::getLayers(){
    return m_layers;
}

inline std::ostream& FilterStatus::fillStream(std::ostream &s) const{
    s<<"Filter Return Code: "<<m_status<<std::endl;
    s<<"Filter code for Energy in CAL: "<<m_calEnergy<<std::endl;
    return s;
}

std::ostream& operator<<(std::ostream &s, const FilterStatus &obj){
    return obj.fillStream(s);
}
