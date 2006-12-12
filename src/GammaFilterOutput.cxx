#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "GammaFilterOutput.h"
#include "OnboardFilterTds/FilterStatus.h"

#include "GaudiKernel/MsgStream.h"

#include "EFC/EFC_edsFw.h"

// Constructor
GammaFilterOutput::GammaFilterOutput(int offset) : m_offset(offset)
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
void GammaFilterOutput::eovProcessing(void* callBackParm, EDS_fwIxb* ixb)
{
//
//  Display event info for debugging
//    EFC_display(evt,0xffffffff);
//
//  Get the result vector information
    // ok these are not here... we need to figure out where to find them in JJ's scheme...
////    TDS_variables.status      = ixb->statusGamma;
////    TDS_variables.stageEnergy = ixb->stageEnergyGamma;

    FilterTdsPointers* tdsPointers = (FilterTdsPointers*)callBackParm;

    // Recover pointers to the TDS objects
    OnboardFilterTds::FilterStatus*    filterStatus    = tdsPointers->m_filterStatus;
    OnboardFilterTds::ObfFilterStatus* obfFilterStatus = tdsPointers->m_obfFilterStatus;
    OnboardFilterTds::TowerHits*       towerHits       = tdsPointers->m_towerHits;

    // Retrieve the Gamma Filter Status Word
    EDS_rsdDsc*   rsdDsc       = ixb->rsd.dscs + m_offset;
    unsigned int* dscPtr       = (unsigned int*)rsdDsc->ptr;
    unsigned int  statusWord   = *dscPtr++;

    filterStatus->set(statusWord);
    filterStatus->setStageEnergy(*dscPtr);

    // Create a new Gamma Status TDS sub object
    OnboardFilterTds::ObfGammaStatus* gamStat = new OnboardFilterTds::ObfGammaStatus(statusWord);

    // Add it to the TDS object
    obfFilterStatus->addFilterStatus(OnboardFilterTds::ObfFilterStatus::GammaFilter, gamStat);

    // Increment counters accordingly
    if((statusWord & GFC_STATUS_M_GEM_THROTTLE) != 0)        m_statusBits[0]++;
    if((statusWord & GFC_STATUS_M_GEM_TKR) != 0)             m_statusBits[1]++;
    if((statusWord & GFC_STATUS_M_GEM_CALLO) != 0)           m_statusBits[2]++;
    if((statusWord & GFC_STATUS_M_GEM_CALHI) != 0)           m_statusBits[3]++;
    if((statusWord & GFC_STATUS_M_GEM_CNO) != 0)             m_statusBits[4]++;
    if((statusWord & GFC_STATUS_M_ACD_TOP) != 0)             m_statusBits[5]++;
    if((statusWord & GFC_STATUS_M_ACD_SIDE) != 0)            m_statusBits[6]++;
    if((statusWord & GFC_STATUS_M_ACD_SIDE_FILTER) != 0)     m_statusBits[7]++;
    if((statusWord & GFC_STATUS_M_TKR_EQ_1) != 0)            m_statusBits[8]++;
    if((statusWord & GFC_STATUS_M_TKR_GE_2) != 0)            m_statusBits[9]++;

    if((statusWord & GFC_STATUS_M_TKR_LT_2_ELO) != 0)        m_vetoBits[0]++;
    if((statusWord & GFC_STATUS_M_TKR_SKIRT) != 0)           m_vetoBits[1]++;
    if((statusWord & GFC_STATUS_M_TKR_EQ_0) != 0)            m_vetoBits[2]++;
    if((statusWord & GFC_STATUS_M_TKR_ROW2) != 0)            m_vetoBits[3]++;
    if((statusWord & GFC_STATUS_M_TKR_ROW01) != 0)           m_vetoBits[4]++;
    if((statusWord & GFC_STATUS_M_TKR_TOP) != 0)             m_vetoBits[5]++;
    if((statusWord & GFC_STATUS_M_ZBOTTOM) != 0)             m_vetoBits[6]++;
    if((statusWord & GFC_STATUS_M_EL0_ETOT_90) != 0)         m_vetoBits[7]++;
    if((statusWord & GFC_STATUS_M_EL0_ETOT_01) != 0)         m_vetoBits[8]++;
    if((statusWord & GFC_STATUS_M_SIDE) != 0)                m_vetoBits[9]++;
    if((statusWord & GFC_STATUS_M_TOP) != 0)                 m_vetoBits[10]++;
    if((statusWord & GFC_STATUS_M_SPLASH_1) != 0)            m_vetoBits[11]++;
    if((statusWord & GFC_STATUS_M_E350_FILTER_TILE) != 0)    m_vetoBits[12]++;
    if((statusWord & GFC_STATUS_M_E0_TILE) != 0)             m_vetoBits[13]++;
    if((statusWord & GFC_STATUS_M_SPLASH_0) != 0)            m_vetoBits[14]++;
    if((statusWord & GFC_STATUS_M_NOCALLO_FILTER_TILE) != 0) m_vetoBits[15]++;

    if((statusWord & GFC_STATUS_M_VETOED) != 0)              m_vetoBits[16]++;

    return;
   
}

void GammaFilterOutput::eorProcessing(MsgStream& log)
{
    // Output the bit frequency table
    log << MSG::INFO << "-- Gamma Filter bit frequency table -- " << endreq;
    log << MSG::INFO << "Status Bit                         Value" << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_ACD             " << m_statusBits[0] << endreq;       
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_DIR             " << m_statusBits[1] << endreq;       
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_ATF             " << m_statusBits[2] << endreq;      
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_CAL1            " << m_statusBits[3] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR             " << m_statusBits[4] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_ACD_TOP         " << m_statusBits[5] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_ACD_SIDE        " << m_statusBits[6] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_ACD_SIDE_FILTER " << m_statusBits[7] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_POSSIBLE    " << m_statusBits[8] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_TRIGGER     " << m_statusBits[9] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_CAL_LO          " << m_statusBits[10] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_CAL_HI          " << m_statusBits[11] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_EQ_1        " << m_statusBits[12] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_GE_2        " << m_statusBits[13] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_THROTTLE    " << m_statusBits[14] << endreq;
    
    log << MSG::INFO << "Veto Bit Summary" << endreq;
    log << MSG::INFO << "Trigger Name                           Count\n" << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_LT_2_ELO        " << m_vetoBits[0] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_SKIRT           " << m_vetoBits[1] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_EQ_0            " << m_vetoBits[2] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_ROW2            " << m_vetoBits[3] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_ROW01           " << m_vetoBits[4] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_TKR_TOP             " << m_vetoBits[5] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_ZBOTTOM             " << m_vetoBits[6] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_EL0_ETOT_90         " << m_vetoBits[7] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_EL0_ETOT_01         " << m_vetoBits[8] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_SIDE                " << m_vetoBits[9] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_TOP                 " << m_vetoBits[10] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_SPLASH_1            " << m_vetoBits[11] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_E350_FILTER_TILE    " << m_vetoBits[12] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_E0_TILE             " << m_vetoBits[13] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_SPLASH_0            " << m_vetoBits[14] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_NOCALLO_FILTER_TILE " << m_vetoBits[15] << endreq;
    log << MSG::INFO << "EFC_GAMMA_STATUS_M_VETOED              " << m_vetoBits[16] << endreq;
    log << MSG::INFO << endreq;

    return;
}

