#ifndef _FILTERTRACKDISPLAY_H_
#define _FILTERTRACKDISPLAY_H_

#include "OnboardFilter/FilterStatus.h"
#include "gui/DisplayRep.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"

class FilterTrackDisplay : public gui::DisplayRep{
 public:
  FilterTrackDisplay(IDataProviderSvc *service);
  virtual ~FilterTrackDisplay(){};
  void update();
 private:
  IDataProviderSvc *dps;
};

#endif
