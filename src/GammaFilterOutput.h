/** @file GammaFilterOutput.h

* @class GammaFilterOutput
*
* @brief This class handles the End of Event and End of Run "output" processing for the Gamma Filter 
*
* last modified 12/04/2006
*
* @authors T. Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/TkrRecon/src/PatRec/VectorLinks/VecPoint.h,v 0.1 2006/03/21 01:12:37 usher Exp $
*/

#ifndef __GammaFilterOutput_H
#define __GammaFilterOutput_H

#include "OutputRtn.h"

class GammaFilterOutput : virtual public OutputRtn
{
public:
    GammaFilterOutput(int offset);
    virtual ~GammaFilterOutput() {}

    // This defines the method called for end of event processing
    void eovProcessing(void* callBackParm, EDS_fwIxb* ixb);

    // This for end of run processing
    void eorProcessing(MsgStream& log);
private:
    int m_offset;   // Offset into ixb event desriptor block for this information

    int m_vetoBits[17];      //array to count # of times each veto bit was set
    int m_statusBits[15];    //array to count # of times each veto bit was set
};

#endif // __ObfInterface_H
