#include "OnboardFilter/FilterStatus.h"

using namespace OnboardFilterTds;

FilterStatus::FilterStatus(){
  status=0;
  CalEnergy=0;
  tcids=0;
  acd_xy=0;
  acd_xz=0;
  acd_yz=0;
  acdStatus=0;
  for(int counter=0;counter<16;counter++)
    layers[counter]=0;
}

FilterStatus::FilterStatus(const unsigned int code, const int energy, const int ids, 
                           const int xz, const int yz, const int xy, const int acdstatus, 
                           const int *layerCode){
  status=code;
  CalEnergy=energy;
  tcids=ids;
  acd_xz=xz;
  acd_xy=xy;
  acd_yz=yz;
  acdStatus=acdstatus;
  if(layers!=NULL){
    for(int counter=0;counter<16;counter++)
      layers[counter]=layerCode[counter];
  }
}

FilterStatus::~FilterStatus(){
}

void FilterStatus::set(const unsigned int code){
  std::cout<<"Setting FilterStatus in TDS to: "<<code<<std::endl;
  status=code;
}

unsigned int FilterStatus::get() const{
  return status;
}

unsigned int FilterStatus::getLow() const{
  return status & 0x7FFF;
}

unsigned int FilterStatus::getHigh() const{
  return status>>15;
}

void FilterStatus::setCalEnergy(const int energy){
  CalEnergy=energy;
}

int FilterStatus::getCalEnergy() const{
  return CalEnergy;
}

void FilterStatus::setTcids(const int ids){
  tcids=ids;
}

int FilterStatus::getTcids()const{
  return tcids;
}

void FilterStatus::setAcdMap(const int xz, const int yz, const int xy){
  acd_xz=xz;
  acd_yz=yz;
  acd_xy=xy;
}

void FilterStatus::getAcdMap(int &xz, int &yz, int &xy) const {
  xz=acd_xz;
  yz=acd_yz;
  xy=acd_xy;
}


void FilterStatus::setAcdStatus(const int status){
  acdStatus=status;
}

int FilterStatus::getAcdStatus() const {
  return acdStatus;
}

void FilterStatus::setLayers(const int *layerCode){
  for(int counter=0;counter<16;counter++)
    layers[counter]=layerCode[counter];
}

int * FilterStatus::getLayers(){
  return layers;
}

inline std::ostream& FilterStatus::fillStream(std::ostream &s) const{
  s<<"Filter Return Code: "<<status<<std::endl;
  s<<"Filter code for Energy in CAL: "<<CalEnergy<<std::endl;
  return s;
}

std::ostream& operator<<(std::ostream &s, const FilterStatus &obj){
  return obj.fillStream(s);
}
