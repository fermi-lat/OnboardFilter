/** @file TkrFilterOutput.h

* @class TkrFilterOutput
*
* @brief This class handles the End of Event and End of Run "output" processing for the Gamma Filter 
*
* last modified 12/04/2006
*
* @authors T. Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/TkrRecon/src/PatRec/VectorLinks/VecPoint.h,v 0.1 2006/03/21 01:12:37 usher Exp $
*/

#ifndef __TkrFilterOutput_H
#define __TkrFilterOutput_H

#include "OutputRtn.h"

class trackProj;

class TkrFilterOutput : virtual public OutputRtn
{
public:
    TkrFilterOutput();
    virtual ~TkrFilterOutput() {}

    // This defines the method called for end of event processing
    void eovProcessing(void* callBackParm, EDS_fwIxb* ixb);

    // This for end of run processing
    void eorProcessing(MsgStream& log);
private:
    // Local functions
    void extractFilterTkrInfo(OnboardFilterTds::FilterStatus* filterStatus, EDS_fwIxb *ixb);
    void extractBestTrackInfo(OnboardFilterTds::FilterStatus* filterStatus, EDS_fwIxb *ixb);
    void extractTkrTwrHitInfo(OnboardFilterTds::TowerHits* towerHits, EDS_fwIxb *ixb);

    trackProj* m_trackProj;
};

#endif // __ObfInterface_H
