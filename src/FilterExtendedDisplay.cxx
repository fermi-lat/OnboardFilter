#include "FilterExtendedDisplay.h"
#include <vector>

FilterExtendedDisplay::FilterExtendedDisplay(IDataProviderSvc *service){
  dps=service;
}

void FilterExtendedDisplay::update(){
  OnboardFilterTds::FilterStatus *status=SmartDataPtr
    <OnboardFilterTds::FilterStatus>(dps,"/Event/Filter/FilterStatus");
  if(status){
    setColor("green");
	std::vector<OnboardFilterTds::track> tracks=status->getTracks();
    for(int counter=0;counter<tracks.size();counter++){
      set_line_style(DOTTED_LINE);
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
