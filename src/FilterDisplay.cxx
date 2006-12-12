#include "OnboardFilter/FilterDisplay.h"
#include <vector>

static ToolFactory <FilterDisplay> factory;
const IToolFactory &FilterDisplayFactory = factory;

FilterDisplay::FilterDisplay(const std::string &type,
                             const std::string &name,
                             const IInterface *parent)
                             :AlgTool(type, name, parent){
    declareInterface<IGuiTool>(this);
}

StatusCode FilterDisplay::initialize(gui::GuiMgr *guiMgr){
    MsgStream log(msgSvc(),name());
    gui::DisplayControl &display = guiMgr->display();
    gui::DisplayControl::DisplaySubMenu &fltrmenu = display.subMenu("OnboardFilter");
    IDataProviderSvc *eventSvc;
    if(service("EventDataSvc",eventSvc,true).isFailure()){
        log<<MSG::ERROR<<"Unable to retrieve EventDataSvc"<<endreq;
        return StatusCode::FAILURE;
    }
    fltrmenu.add(new FilterTrackDisplay(eventSvc),"Tracks",false);
    fltrmenu.add(new FilterExtendedDisplay(eventSvc),"Extended Tracks",false);
    return StatusCode::SUCCESS;
}

FilterTrackDisplay::FilterTrackDisplay(IDataProviderSvc *service){
    dps=service;
}

void FilterTrackDisplay::update(){
    OnboardFilterTds::FilterStatus *status=SmartDataPtr
        <OnboardFilterTds::FilterStatus>(dps,"/Event/Filter/FilterStatus");
    if(status){
        set_line_style(SOLID_LINE);
        setColor("green");
        std::vector<OnboardFilterTds::track> tracks=status->getTracks();
        for(int counter=0;counter<tracks.size();counter++){
             moveTo(tracks[counter].lowCoord[0],tracks[counter].lowCoord[1],
                    tracks[counter].lowCoord[2]);
             lineTo(tracks[counter].highCoord[0],tracks[counter].highCoord[1],
                    tracks[counter].highCoord[2]);
        }
    }
}

FilterExtendedDisplay::FilterExtendedDisplay(IDataProviderSvc *service){
    dps=service;
}

void FilterExtendedDisplay::update(){
    OnboardFilterTds::FilterStatus *status=SmartDataPtr
        <OnboardFilterTds::FilterStatus>(dps,"/Event/Filter/FilterStatus");
    if(status){
        set_line_style(DOTTED_LINE);
        setColor("green");
        std::vector<OnboardFilterTds::track> tracks=status->getTracks();
        for(int counter=0;counter<tracks.size();counter++){
            moveTo(tracks[counter].exLowCoord[0],tracks[counter].exLowCoord[1],
	           tracks[counter].exLowCoord[2]);
            lineTo(tracks[counter].lowCoord[0],tracks[counter].lowCoord[1],
                   tracks[counter].lowCoord[2]);
            moveTo(tracks[counter].exHighCoord[0],tracks[counter].exHighCoord[1],
                   tracks[counter].exHighCoord[2]);
            lineTo(tracks[counter].highCoord[0],tracks[counter].highCoord[1],
                   tracks[counter].highCoord[2]);
        }
    }
}
