#ifndef FILTER_DISPLAY_H
#define FILTER_DISPLAY_H
/**
 * @file DetectorDisplay.h
 * @brief Classes to display computed tracks in the Gui
 * $Header$
 */

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/IDataProviderSvc.h"

#include "GuiSvc/IGuiTool.h"

#include "gui/DisplayControl.h"
#include "GuiSvc/IGuiSvc.h"
#include "gui/GuiMgr.h"

#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

class FilterDisplay : public AlgTool, virtual public IGuiTool{
public:
    FilterDisplay(const std::string &type, const std::string &name, const IInterface *parent);
    virtual ~FilterDisplay(){};
    StatusCode initialize(gui::GuiMgr *);
};

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
