#ifndef FILTER_STATUS_H
#define FILTER_STATUS_H

#include <iostream>

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/ObjectVector.h"

#include "Event/TopLevel/Definitions.h"
#include "Event/TopLevel/EventModel.h"

/**
 * @class FilterStatus
 * @brief TDS for storing the information returned by the filter
 */
//extern const CLID& CLID_FilterStatus;


namespace OnboardFilterTds{
  static const std::string Path = EventModel::EventHeader + "/Filter" + "/FilterStats";

  class FilterStatus : public DataObject{
  public:
    FilterStatus();
    FilterStatus(const unsigned int code,const int energy=0);
    virtual ~FilterStatus();
    
    ///Set the statuscode of the filter
    void set(const unsigned int code);
    
    ///Return the statuscode of the filter
    unsigned int get() const;
    
    ///Returns the 16 most significant bits of the filter code
    unsigned int getHigh() const;

    ///Returns the 16 least signifcat bits of the filter code
    unsigned int getLow() const;

    ///Set the Energy in CAL
      void setCalEnergy(const int energy);

    ///Returns the value stored in CalEnergy
    int getCalEnergy();
	
    virtual std::ostream& fillStream(std::ostream &s) const;
    friend std::ostream& operator << (std::ostream &s, const FilterStatus &obj);
  private:
    ///Filter status code
    unsigned int status;
    ///Energy in CAL
    int CalEnergy;
  };
};
#endif
