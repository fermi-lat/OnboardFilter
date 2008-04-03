#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "DFCFilterOutput.h"

#include "GaudiKernel/MsgStream.h"

#include "EFC/EFC_edsFw.h"
#include "EDS/EBF_dir.h"
#include "XFC/DFC_status.h"

// Constructor
DFCFilterOutput::DFCFilterOutput(int offset, bool passThrough) : 
       m_offset(offset), m_passThrough(passThrough)
{
    // zero our counters
    memset(m_vetoBits,   0, 17*sizeof(int));
    memset(m_statusBits, 0, 15*sizeof(int));

    return;
}
/* ---------------------------------------------------------------------- *//*!

  \fn  void extractFilterInfo(Stream *ostream, EDS_fwIxb *ixb)
                            
  \brief         Extracts info from the onboard filter algorithm. 
  \param ostream dummy var
  \param     ixb The information exchange block.

   This routine extracts both the results and the some of the intermediate
   info used by the filter in deciding if an event should be vetoed or not.
                                                                          */
/* ---------------------------------------------------------------------- */
void DFCFilterOutput::eovProcessing(void* callBackParm, EDS_fwIxb* ixb)
{
    // Cast callBackParm
    ObfOutputCallBackParm* tdsPointers = reinterpret_cast<ObfOutputCallBackParm*>(callBackParm);

    // Recover pointers to the TDS objects
    OnboardFilterTds::ObfFilterStatus* obfFilterStatus = tdsPointers->m_obfFilterStatus;

    // Retrieve the Gamma Filter Status Word
    EDS_rsdDsc*   rsdDsc       = ixb->rsd.dscs + m_offset;
    unsigned char sb           = rsdDsc->sb;
    unsigned int* dscPtr       = (unsigned int*)rsdDsc->ptr;
    unsigned int  statusWord   = *dscPtr++;

    // If we are running pass through mode then we need to manually set the veto bit
    // in the event their was a veto
    if (m_passThrough)
    {
        if (statusWord & DFC_STATUS_M_VETO_DEF)
        {
            statusWord |= DFC_STATUS_M_VETOED;
            sb         |= EDS_RSD_SB_M_VETOED;
        }
    }

    // Create a new DFC Status TDS sub object
    OnboardFilterTds::ObfDFCStatus* dfcStat = new OnboardFilterTds::ObfDFCStatus(rsdDsc->id, statusWord, sb);

    // Add it to the TDS object
    obfFilterStatus->addFilterStatus(OnboardFilterTds::ObfFilterStatus::DFCFilter, dfcStat);

    // Increment counters accordingly
    if((statusWord & DFC_STATUS_M_STAGE_GEM) != 0)      m_statusBits[0]++;

    if((statusWord & DFC_STATUS_M_GEM_CLASSES) != 0)    m_vetoBits[0]++;

    if((statusWord & DFC_STATUS_M_VETOED) != 0)         m_vetoBits[16]++;

    return;
   
}

void DFCFilterOutput::eorProcessing(MsgStream& log)
{
    // Output the bit frequency table
    log << MSG::INFO << "-- DFC Filter bit frequency table -- \n" 
        << "    Status Bit                         Value\n"
        << "    DFC_STATUS_M_STAGE_GEM             " << m_statusBits[0] << "\n"       
    
        << "    Veto Bit Summary" << "\n"
        << "    Trigger Name                           Count\n" << "\n"
        << "    DFC_STATUS_M_GEM_CLASSES           " << m_vetoBits[0] << "\n"
        << "    DFC_STATUS_M_VETOED                " << m_vetoBits[16] << "\n"
        << endreq;

    return;
}

