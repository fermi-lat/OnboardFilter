#ifndef _FILTEREXTENDEDDISPLAY_H_
#define _FILTEREXTENDEDDISPLAY_H_

#include "OnboardFilter/FilterStatus.h"
#include "gui/DisplayRep.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"

class FilterExtendedDisplay : public gui::DisplayRep{
 public:
  FilterExtendedDisplay(IDataProviderSvc *service);
  virtual ~FilterExtendedDisplay(){};
  void update();
 private:
  IDataProviderSvc *dps;
};

#endif
