#include "OnboardFilter/FilterStatus.h"

using namespace Event;

FilterStatus::FilterStatus(){
  status=0;
}

FilterStatus::FilterStatus(const unsigned int code){
  std::cout<<"Setting FilterStatus in TDS to: "<<code<<std::endl;
  status=code;
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

inline std::ostream& FilterStatus::fillStream(std::ostream &s) const{
  s<<"Filter Return Code: "<<status<<std::endl;
  return s;
}

std::ostream& operator<<(std::ostream &s, const FilterStatus &obj){
  return obj.fillStream(s);
}
