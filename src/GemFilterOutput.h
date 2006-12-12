/** @file GemFilterOutput.h

* @class GemFilterOutput
*
* @brief This class handles the End of Event and End of Run "output" processing for the Gamma Filter 
*
* last modified 12/04/2006
*
* @authors T. Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/TkrRecon/src/PatRec/VectorLinks/VecPoint.h,v 0.1 2006/03/21 01:12:37 usher Exp $
*/

#ifndef __GemFilterOutput_H
#define __GemFilterOutput_H

#include "OutputRtn.h"

class GemFilterOutput : virtual public OutputRtn
{
public:
    GemFilterOutput();
    virtual ~GemFilterOutput() {}

    // This defines the method called for end of event processing
    void eovProcessing(void* callBackParm, EDS_fwIxb* ixb);

    // This for end of run processing
    void eorProcessing(MsgStream& log);
private:
};

#endif // __ObfInterface_H
