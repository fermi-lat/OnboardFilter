#ifndef FILTER_STATUS_H
#define FILTER_STATUS_H

#include <iostream>

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/ObjectVector.h"

#include "Event/TopLevel/Definitions.h"

/**
 * @class FilterStatus
 * @brief TDS for storing the information returned by the filter
 */
extern const CLID& CLID_FilterStatus;

namespace Event{
  class FilterStatus : public DataObject{
  public:
    FilterStatus();
    FilterStatus(const unsigned int code);
    virtual ~FilterStatus();
    static const CLID& classID() { return CLID_FilterStatus; }
    virtual const CLID& clID() { return classID(); }
    
    ///Set the statuscode of the filter
    void set(const unsigned int code);
    
    ///Return the statuscode of the filter
    unsigned int get() const;
    
    ///Returns the 16 most significant bits of the filter code
    unsigned int getHigh() const;

    ///Returns the 16 least signifcat bits of the filter code
    unsigned int getLow() const;

    virtual std::ostream& fillStream(std::ostream &s) const;
    friend std::ostream& operator << (std::ostream &s, const FilterStatus &obj);
  private:
    ///Filter status code
    unsigned int status;
  };
};
#endif
