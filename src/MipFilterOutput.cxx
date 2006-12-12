#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "MipFilterOutput.h"
#include "OnboardFilterTds/FilterStatus.h"

#include "GaudiKernel/MsgStream.h"

#include "EFC/EFC_edsFw.h"
#include "EDS/EBF_dir.h"
#include "XFC/XFC/MFC_status.h"

// Constructor
MipFilterOutput::MipFilterOutput(int offset) : m_offset(offset)
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
void MipFilterOutput::eovProcessing(void* callBackParm, EDS_fwIxb* ixb)
{
    // Cast callBackParm
    FilterTdsPointers* tdsPointers = (FilterTdsPointers*)callBackParm;

    // Recover pointers to the TDS objects
    OnboardFilterTds::ObfFilterStatus* obfFilterStatus = tdsPointers->m_obfFilterStatus;

    // Retrieve the Gamma Filter Status Word
    EDS_rsdDsc*   rsdDsc       = ixb->rsd.dscs + m_offset;
    unsigned int* dscPtr       = (unsigned int*)rsdDsc->ptr;
    unsigned int  statusWord   = *dscPtr++;

    // Create a new MIP Status TDS sub object
    OnboardFilterTds::ObfMipStatus* mipStat = new OnboardFilterTds::ObfMipStatus(statusWord);

    // Add it to the TDS object
    obfFilterStatus->addFilterStatus(OnboardFilterTds::ObfFilterStatus::MipFilter, mipStat);

    // Increment counters accordingly
    if((statusWord & MFC_STATUS_M_STAGE_GEM) != 0)  m_statusBits[0]++;
    if((statusWord & MFC_STATUS_M_STAGE_ACD) != 0)  m_statusBits[1]++;
    if((statusWord & MFC_STATUS_M_STAGE_DIR) != 0)  m_statusBits[2]++;
    if((statusWord & MFC_STATUS_M_STAGE_CAL) != 0)  m_statusBits[3]++;
    if((statusWord & MFC_STATUS_M_STAGE_XCAL) != 0) m_statusBits[4]++;
    if((statusWord & MFC_STATUS_M_MULTI_PKT) != 0)  m_statusBits[5]++;
    if((statusWord & MFC_STATUS_M_ERR_CAL) != 0)    m_statusBits[6]++;
    if((statusWord & MFC_STATUS_M_ERR_CTB) != 0)    m_statusBits[7]++;
    if((statusWord & MFC_STATUS_M_ERR_DIR) != 0)    m_statusBits[8]++;

    if((statusWord & MFC_STATUS_M_NO_TKR_ADJ) != 0) m_vetoBits[0]++;
    if((statusWord & MFC_STATUS_M_LYR_COUNTS) != 0) m_vetoBits[1]++;
    if((statusWord & MFC_STATUS_M_NO_ACD_TKR) != 0) m_vetoBits[2]++;
    if((statusWord & MFC_STATUS_M_GEM_NOTKR) != 0)  m_vetoBits[3]++;
    if((statusWord & MFC_STATUS_M_GEM_CNO) != 0)    m_vetoBits[4]++;

    if((statusWord & MFC_STATUS_M_VETOED) != 0)     m_vetoBits[16]++;

    return;
   
}

void MipFilterOutput::eorProcessing(MsgStream& log)
{
    // Output the bit frequency table
    log << MSG::INFO << "-- Mip Filter bit frequency table -- " << endreq;
    log << MSG::INFO << "Status Bit                         Value" << endreq;
    log << MSG::INFO << "MFC_STATUS_M_STAGE_GEM             " << m_statusBits[0] << endreq;       
    log << MSG::INFO << "MFC_STATUS_M_STAGE_ACD             " << m_statusBits[1] << endreq;       
    log << MSG::INFO << "MFC_STATUS_M_STAGE_DIR             " << m_statusBits[2] << endreq;      
    log << MSG::INFO << "MFC_STATUS_M_STAGE_CAL             " << m_statusBits[3] << endreq;
    log << MSG::INFO << "MFC_STATUS_M_STAGE_XCAL            " << m_statusBits[4] << endreq;
    log << MSG::INFO << "MFC_STATUS_M_MULTI_PKT             " << m_statusBits[5] << endreq;
    log << MSG::INFO << "MFC_STATUS_M_ERR_CAL               " << m_statusBits[6] << endreq;
    log << MSG::INFO << "MFC_STATUS_M_ERR_CTB               " << m_statusBits[7] << endreq;
    log << MSG::INFO << "MFC_STATUS_M_ERR_DIR               " << m_statusBits[8] << endreq;
    
    log << MSG::INFO << "Veto Bit Summary" << endreq;
    log << MSG::INFO << "Trigger Name                           Count\n" << endreq;
    log << MSG::INFO << "MFC_STATUS_M_NO_TKR_ADJ            " << m_vetoBits[0] << endreq;
    log << MSG::INFO << "MFC_STATUS_M_LYR_COUNTS            " << m_vetoBits[1] << endreq;
    log << MSG::INFO << "MFC_STATUS_M_NO_ACD_TKR            " << m_vetoBits[2] << endreq;
    log << MSG::INFO << "MFC_STATUS_M_GEM_NOTKR             " << m_vetoBits[3] << endreq;
    log << MSG::INFO << "MFC_STATUS_M_GEM_CNO               " << m_vetoBits[4] << endreq;
    log << MSG::INFO << "MFC_STATUS_M_VETOED                " << m_vetoBits[16] << endreq;
    log << MSG::INFO << endreq;

    return;
}

