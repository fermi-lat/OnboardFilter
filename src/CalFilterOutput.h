/** @file CalFilterOutput.h

* @class CalFilterOutput
*
* @brief This class handles the End of Event and End of Run "output" processing for the Gamma Filter 
*
* last modified 12/04/2006
*
* @authors T. Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/TkrRecon/src/PatRec/VectorLinks/VecPoint.h,v 0.1 2006/03/21 01:12:37 usher Exp $
*/

#ifndef __CalFilterOutput_H
#define __CalFilterOutput_H

#include "OutputRtn.h"

class CalFilterOutput : virtual public OutputRtn
{
public:
    CalFilterOutput();
    virtual ~CalFilterOutput() {}

    // This defines the method called for end of event processing
    void eovProcessing(void* callBackParm, EDS_fwIxb* ixb);

    // This for end of run processing
    void eorProcessing(MsgStream& log);
private:
    // Local functions
    void extractFilterCalInfo(OnboardFilterTds::FilterStatus* filterStatus, EDS_fwIxb *ixb);
};

#endif // __ObfInterface_H
