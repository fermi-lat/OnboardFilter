/** @file OutputRtn.h

* @class OutputRtn
*
* @brief Virtual class definition for filter ouptut routines 
*
* last modified 12/04/2006
*
* @authors T. Usher
*
* $Header: /nfs/slac/g/glast/ground/cvs/TkrRecon/src/PatRec/VectorLinks/VecPoint.h,v 0.1 2006/03/21 01:12:37 usher Exp $
*/

#ifndef __OutputRtn_H
#define __OutputRtn_H

#include "OnboardFilterTds/FilterStatus.h"
#include "OnboardFilterTds/ObfFilterStatus.h"

// Define a utility class for the callback routines 
class FilterTdsPointers
{
public:
    OnboardFilterTds::FilterStatus*    m_filterStatus;
    OnboardFilterTds::ObfFilterStatus* m_obfFilterStatus;
    OnboardFilterTds::TowerHits*       m_towerHits;
    FilterTdsPointers() : m_filterStatus(0), m_towerHits(0) {}
    ~FilterTdsPointers() {}
};

// Forward declarations
#ifndef EDS_fwIxb 
    typedef struct _EDS_fwIxb EDS_fwIxb;
#endif

// This for Gaudi output (should be a better way?)
class MsgStream;

// Virtual Class definition for the output routines
class OutputRtn
{
public:
    // This defines the method called for end of event processing
    virtual void eovProcessing(void* callBackParm, EDS_fwIxb* ixb) = 0;

    // This for end of run processing
    virtual void eorProcessing(MsgStream& log) = 0;
};

// Typedef a vector of the above for use in call back control
typedef std::vector<OutputRtn*> OutputRtnVec;

#endif // __ObfInterface_H
