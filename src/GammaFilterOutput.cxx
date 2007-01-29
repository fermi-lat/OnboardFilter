#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "GammaFilterOutput.h"
#include "OnboardFilterTds/FilterStatus.h"

#include "GaudiKernel/MsgStream.h"

#include "EFC/EFC_edsFw.h"

// Constructor
GammaFilterOutput::GammaFilterOutput(int offset, unsigned bitsToIgnore, bool passThrough) : 
     m_offset(offset), m_bitsToIgnore(bitsToIgnore),m_passThrough(passThrough)
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
    EDS_rsdDsc*   rsdDsc        = ixb->rsd.dscs + m_offset;
    unsigned int* dscPtr        = (unsigned int*)rsdDsc->ptr;
    unsigned int  oldStatusWord = *dscPtr++;
    unsigned int  newStatusWord = oldStatusWord;

    // If we are running pass through mode then we need to manually set the veto bit
    // in the event their was a veto
    if (m_passThrough)
    {
        // Remove any undesired veto bits from the "old school" status word **** 
        oldStatusWord = ~m_bitsToIgnore & oldStatusWord;

        // If any unmasked veto bits are set then set the general event vetoed bit 
        if (oldStatusWord & GFC_STATUS_M_VETOES)
        {
            oldStatusWord |= GFC_STATUS_M_VETOED;
            newStatusWord |= GFC_STATUS_M_VETOED;
        }
    }

    filterStatus->set(oldStatusWord);
    filterStatus->setStageEnergy(*dscPtr);

    // Create a new Gamma Status TDS sub object
    OnboardFilterTds::ObfGammaStatus* gamStat = new OnboardFilterTds::ObfGammaStatus(newStatusWord);

    // Add it to the TDS object
    obfFilterStatus->addFilterStatus(OnboardFilterTds::ObfFilterStatus::GammaFilter, gamStat);

    // Increment counters accordingly
    if((oldStatusWord & GFC_STATUS_M_GEM_THROTTLE) != 0)        m_statusBits[0]++;
    if((oldStatusWord & GFC_STATUS_M_GEM_TKR) != 0)             m_statusBits[1]++;
    if((oldStatusWord & GFC_STATUS_M_GEM_CALLO) != 0)           m_statusBits[2]++;
    if((oldStatusWord & GFC_STATUS_M_GEM_CALHI) != 0)           m_statusBits[3]++;
    if((oldStatusWord & GFC_STATUS_M_GEM_CNO) != 0)             m_statusBits[4]++;
    if((oldStatusWord & GFC_STATUS_M_ACD_TOP) != 0)             m_statusBits[5]++;
    if((oldStatusWord & GFC_STATUS_M_ACD_SIDE) != 0)            m_statusBits[6]++;
    if((oldStatusWord & GFC_STATUS_M_ACD_SIDE_FILTER) != 0)     m_statusBits[7]++;
    if((oldStatusWord & GFC_STATUS_M_TKR_EQ_1) != 0)            m_statusBits[8]++;
    if((oldStatusWord & GFC_STATUS_M_TKR_GE_2) != 0)            m_statusBits[9]++;

    if((oldStatusWord & GFC_STATUS_M_TKR_LT_2_ELO) != 0)        m_vetoBits[0]++;
    if((oldStatusWord & GFC_STATUS_M_TKR_SKIRT) != 0)           m_vetoBits[1]++;
    if((oldStatusWord & GFC_STATUS_M_TKR_EQ_0) != 0)            m_vetoBits[2]++;
    if((oldStatusWord & GFC_STATUS_M_TKR_ROW2) != 0)            m_vetoBits[3]++;
    if((oldStatusWord & GFC_STATUS_M_TKR_ROW01) != 0)           m_vetoBits[4]++;
    if((oldStatusWord & GFC_STATUS_M_TKR_TOP) != 0)             m_vetoBits[5]++;
    if((oldStatusWord & GFC_STATUS_M_ZBOTTOM) != 0)             m_vetoBits[6]++;
    if((oldStatusWord & GFC_STATUS_M_EL0_ETOT_90) != 0)         m_vetoBits[7]++;
    if((oldStatusWord & GFC_STATUS_M_EL0_ETOT_01) != 0)         m_vetoBits[8]++;
    if((oldStatusWord & GFC_STATUS_M_SIDE) != 0)                m_vetoBits[9]++;
    if((oldStatusWord & GFC_STATUS_M_TOP) != 0)                 m_vetoBits[10]++;
    if((oldStatusWord & GFC_STATUS_M_SPLASH_1) != 0)            m_vetoBits[11]++;
    if((oldStatusWord & GFC_STATUS_M_E350_FILTER_TILE) != 0)    m_vetoBits[12]++;
    if((oldStatusWord & GFC_STATUS_M_E0_TILE) != 0)             m_vetoBits[13]++;
    if((oldStatusWord & GFC_STATUS_M_SPLASH_0) != 0)            m_vetoBits[14]++;
    if((oldStatusWord & GFC_STATUS_M_NOCALLO_FILTER_TILE) != 0) m_vetoBits[15]++;

    if((oldStatusWord & GFC_STATUS_M_VETOED) != 0)              m_vetoBits[16]++;

    return;
   
}

void GammaFilterOutput::eorProcessing(MsgStream& log)
{
    // Output the bit frequency table
    log << MSG::INFO << "-- Gamma Filter bit frequency table -- \n"
        << "    Status Bit                         Value \n" 
        << "    EFC_GAMMA_STATUS_M_ACD             " << m_statusBits[0] << "\n"       
        << "    EFC_GAMMA_STATUS_M_DIR             " << m_statusBits[1] << "\n"       
        << "    EFC_GAMMA_STATUS_M_ATF             " << m_statusBits[2] << "\n"      
        << "    EFC_GAMMA_STATUS_M_CAL1            " << m_statusBits[3] << "\n"
        << "    EFC_GAMMA_STATUS_M_TKR             " << m_statusBits[4] << "\n"
        << "    EFC_GAMMA_STATUS_M_ACD_TOP         " << m_statusBits[5] << "\n"
        << "    EFC_GAMMA_STATUS_M_ACD_SIDE        " << m_statusBits[6] << "\n"
        << "    EFC_GAMMA_STATUS_M_ACD_SIDE_FILTER " << m_statusBits[7] << "\n"
        << "    EFC_GAMMA_STATUS_M_TKR_POSSIBLE    " << m_statusBits[8] << "\n"
        << "    EFC_GAMMA_STATUS_M_TKR_TRIGGER     " << m_statusBits[9] << "\n"
        << "    EFC_GAMMA_STATUS_M_CAL_LO          " << m_statusBits[10] << "\n"
        << "    EFC_GAMMA_STATUS_M_CAL_HI          " << m_statusBits[11] << "\n"
        << "    EFC_GAMMA_STATUS_M_TKR_EQ_1        " << m_statusBits[12] << "\n"
        << "    EFC_GAMMA_STATUS_M_TKR_GE_2        " << m_statusBits[13] << "\n"
        << "    EFC_GAMMA_STATUS_M_TKR_THROTTLE    " << m_statusBits[14] << "\n"
    
        << "    Veto Bit Summary" << "\n"
        << "    Trigger Name                           Count\n" << "\n"
        << "    EFC_GAMMA_STATUS_M_TKR_LT_2_ELO        " << m_vetoBits[0] << "\n"
        << "    EFC_GAMMA_STATUS_M_TKR_SKIRT           " << m_vetoBits[1] << "\n"
        << "    EFC_GAMMA_STATUS_M_TKR_EQ_0            " << m_vetoBits[2] << "\n"
        << "    EFC_GAMMA_STATUS_M_TKR_ROW2            " << m_vetoBits[3] << "\n"
        << "    EFC_GAMMA_STATUS_M_TKR_ROW01           " << m_vetoBits[4] << "\n"
        << "    EFC_GAMMA_STATUS_M_TKR_TOP             " << m_vetoBits[5] << "\n"
        << "    EFC_GAMMA_STATUS_M_ZBOTTOM             " << m_vetoBits[6] << "\n"
        << "    EFC_GAMMA_STATUS_M_EL0_ETOT_90         " << m_vetoBits[7] << "\n"
        << "    EFC_GAMMA_STATUS_M_EL0_ETOT_01         " << m_vetoBits[8] << "\n"
        << "    EFC_GAMMA_STATUS_M_SIDE                " << m_vetoBits[9] << "\n"
        << "    EFC_GAMMA_STATUS_M_TOP                 " << m_vetoBits[10] << "\n"
        << "    EFC_GAMMA_STATUS_M_SPLASH_1            " << m_vetoBits[11] << "\n"
        << "    EFC_GAMMA_STATUS_M_E350_FILTER_TILE    " << m_vetoBits[12] << "\n"
        << "    EFC_GAMMA_STATUS_M_E0_TILE             " << m_vetoBits[13] << "\n"
        << "    EFC_GAMMA_STATUS_M_SPLASH_0            " << m_vetoBits[14] << "\n"
        << "    EFC_GAMMA_STATUS_M_NOCALLO_FILTER_TILE " << m_vetoBits[15] << "\n"
        << "    EFC_GAMMA_STATUS_M_VETOED              " << m_vetoBits[16] << "\n"
        << endreq;

    return;
}

