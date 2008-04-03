#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "HFCFilterOutput.h"

#include "GaudiKernel/MsgStream.h"

#include "EFC/EFC_edsFw.h"
#include "EDS/EBF_dir.h"
#include "XFC/HFC_status.h"

// Constructor
HFCFilterOutput::HFCFilterOutput(int offset, bool passThrough) : 
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
void HFCFilterOutput::eovProcessing(void* callBackParm, EDS_fwIxb* ixb)
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
        if (statusWord & HFC_STATUS_M_VETO_DEF)
        {
            statusWord |= HFC_STATUS_M_VETOED;
            sb         |= EDS_RSD_SB_M_VETOED;
        }
    }

    // Create a new HFC status TDS sub object
    OnboardFilterTds::ObfHFCStatus* hfcStat = new OnboardFilterTds::ObfHFCStatus(rsdDsc->id, statusWord, sb);

    // Add it to the TDS object
    obfFilterStatus->addFilterStatus(OnboardFilterTds::ObfFilterStatus::HFCFilter, hfcStat);

    // Increment counters accordingly
    if((statusWord & HFC_STATUS_M_STAGE_GEM) != 0)      m_statusBits[0]++;
    if((statusWord & HFC_STATUS_M_STAGE_DIR) != 0)      m_statusBits[1]++;
    if((statusWord & HFC_STATUS_M_STAGE_CAL) != 0)      m_statusBits[2]++;
    if((statusWord & HFC_STATUS_M_STAGE_CAL_ECHK) != 0) m_statusBits[3]++;
    if((statusWord & HFC_STATUS_M_STAGE_CAL_LCHK) != 0) m_statusBits[4]++;
    if((statusWord & HFC_STATUS_M_MULTI_PKT) != 0)      m_statusBits[5]++;
    if((statusWord & HFC_STATUS_M_ERR_CAL) != 0)        m_statusBits[6]++;
    if((statusWord & HFC_STATUS_M_ERR_CTB) != 0)        m_statusBits[7]++;
    if((statusWord & HFC_STATUS_M_ERR_DIR) != 0)        m_statusBits[8]++;

    if((statusWord & HFC_STATUS_M_LYR_ENERGY) != 0)     m_vetoBits[0]++;
    if((statusWord & HFC_STATUS_M_LYR_COUNTS) != 0)     m_vetoBits[1]++;
    if((statusWord & HFC_STATUS_M_GEM_NOTKR) != 0)      m_vetoBits[2]++;
    if((statusWord & HFC_STATUS_M_GEM_NOCALLO) != 0)    m_vetoBits[3]++;
    if((statusWord & HFC_STATUS_M_GEM_NOCNO) != 0)      m_vetoBits[4]++;

    if((statusWord & HFC_STATUS_M_VETOED) != 0)         m_vetoBits[16]++;

    return;
   
}

void HFCFilterOutput::eorProcessing(MsgStream& log)
{
    // Output the bit frequency table
    log << MSG::INFO << "-- HFC Filter bit frequency table -- \n" 
        << "    Status Bit                         Value\n"
        << "    HFC_STATUS_M_STAGE_GEM             " << m_statusBits[0] << "\n"       
        << "    HFC_STATUS_M_STAGE_DIR             " << m_statusBits[1] << "\n"       
        << "    HFC_STATUS_M_STAGE_CAL             " << m_statusBits[2] << "\n"      
        << "    HFC_STATUS_M_STAGE_CAL_ECHK        " << m_statusBits[3] << "\n"
        << "    HFC_STATUS_M_STAGE_CAL_LCHK        " << m_statusBits[4] << "\n"
        << "    HFC_STATUS_M_MULTI_PKT             " << m_statusBits[5] << "\n"
        << "    HFC_STATUS_M_ERR_CAL               " << m_statusBits[6] << "\n"
        << "    HFC_STATUS_M_ERR_CTB               " << m_statusBits[7] << "\n"
        << "    HFC_STATUS_M_ERR_DIR               " << m_statusBits[8] << "\n"
    
        << "    Veto Bit Summary" << "\n"
        << "    Trigger Name                           Count\n" << "\n"
        << "    HFC_STATUS_M_LYR_ENERGY            " << m_vetoBits[0] << "\n"
        << "    HFC_STATUS_M_LYR_COUNTS            " << m_vetoBits[1] << "\n"
        << "    HFC_STATUS_M_GEM_NOTKR             " << m_vetoBits[2] << "\n"
        << "    HFC_STATUS_M_GEM_NOCALLO           " << m_vetoBits[3] << "\n"
        << "    HFC_STATUS_M_NOCNO                 " << m_vetoBits[4] << "\n"
        << "    HFC_STATUS_M_VETOED                " << m_vetoBits[16] << "\n"
        << endreq;

    return;
}

