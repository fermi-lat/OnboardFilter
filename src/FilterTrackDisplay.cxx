#include "FilterTrackDisplay.h"
#include <vector>

FilterTrackDisplay::FilterTrackDisplay(IDataProviderSvc *service){
  dps=service;
}

void FilterTrackDisplay::update(){
  OnboardFilterTds::FilterStatus *status=SmartDataPtr
    <OnboardFilterTds::FilterStatus>(dps,"/Event/Filter/FilterStatus");
  if(status){
    setColor("green");
    vector<OnboardFilterTds::track> tracks=status->getTracks();
    for(int counter=0;counter<tracks.size();counter++){
      set_line_style(SOLID_LINE);
      moveTo(tracks[counter].lowCoord[0],tracks[counter].lowCoord[1],
	     tracks[counter].lowCoord[2]);
      lineTo(tracks[counter].highCoord[0],tracks[counter].highCoord[1],
	     tracks[counter].highCoord[2]);
    }
  }
}
