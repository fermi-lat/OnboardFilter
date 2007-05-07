/** @file DFCFilterOutput.h

* @class DFCFilterOutput
*
* @brief This class handles the End of Event and End of Run "output" processing for the Gamma Filter 
*
* last modified 12/04/2006
*
* @authors T. Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/DFCFilterOutput.h,v 1.1 2007/03/29 19:50:46 usher Exp $
*/

#ifndef __DFCFilterOutput_H
#define __DFCFilterOutput_H

#include "OutputRtn.h"

class DFCFilterOutput : virtual public OutputRtn
{
public:
    DFCFilterOutput(int offset, bool passThrough=false);
    virtual ~DFCFilterOutput() {}

    // This defines the method called for end of event processing
    void eovProcessing(void* callBackParm, EDS_fwIxb* ixb);

    // This for end of run processing
    void eorProcessing(MsgStream& log);
private:
    // Local functions

    int m_offset;         // Offset into ixb event desriptor block for this information
    bool m_passThrough;   // Running filter in pass through mode

    int m_vetoBits[17];   //array to count # of times each veto bit was set
    int m_statusBits[15]; //array to count # of times each veto bit was set
};

#endif // __ObfInterface_H
