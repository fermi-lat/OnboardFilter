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
  class FilterStatus : public DataObject{
  public:
    FilterStatus();
    FilterStatus(const unsigned int code, const int energey=0, const int ids=0,
                 const int xz=0, const int yz=0, const int xy=0, const int acdstatus=0,
                 const int *layerCode=NULL);
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
    int getCalEnergy() const;

    ///Set the Code specifying the towers with triggers or possible triggers
    void setTcids(const int ids);
    ///Return the Code specifying the towers with triggers or possible triggers
    int getTcids() const;

    ///Set the ACD hit map results
    void setAcdMap(const int xz, const int yz, const int xy);
    ///Return the ACD hit map results
    void getAcdMap(int &xz, int &yz, int &xy) const;

    ///Set the ACD faces intersected by projections
    void setAcdStatus(const int status);
    ///Return the ACD faces intersected by projections
    int getAcdStatus() const;

    ///Set what layers were hit in each tower
    void setLayers(const int *layerCode);
    ///Return what layers were hit in each tower
    int *getLayers();

    virtual std::ostream& fillStream(std::ostream &s) const;
    friend std::ostream& operator << (std::ostream &s, const FilterStatus &obj);
  private:
    ///Filter status code
    unsigned int status;
    ///Energy in CAL
    int CalEnergy;
    ///Towers with triggers
    int tcids;
    ///ACD hit map
    int acd_xz;
    int acd_yz;
    int acd_xy;
    ///ACD faces
    int acdStatus;
    ///Layers hit in each tower
    int layers[16];
  };
};
#endif
