#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "GemFilterOutput.h"
#include "OnboardFilterTds/FilterStatus.h"

#include "GaudiKernel/MsgStream.h"

#include "EFC/EFC_edsFw.h"
#include "EDS/EBF_dir.h"
#include "EDS/EBF_cid.h"
#include "EDS/EBF_gem.h"

// Constructor
GemFilterOutput::GemFilterOutput()
{
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
void GemFilterOutput::eovProcessing(void* callBackParm, EDS_fwIxb* ixb)
{
    // Extract output pointers info
    ObfOutputCallBackParm* tdsPointers = reinterpret_cast<ObfOutputCallBackParm*>(callBackParm);

    // Recover pointers to the TDS objects
    OnboardFilterTds::FilterStatus* filterStatus = tdsPointers->m_filterStatus;

//  Get a pointer to the directory
    EDS_fwEvt      *evt =  &ixb->blk.evt;
    const EBF_dir  *dir = evt->dir;

//  Get the GEM info
    const EBF_gem *gem;
    gem = (const EBF_gem *)dir->ctbs[EBF_CID_K_GEM].ctb->dat;

    filterStatus->setGemThrTkr(gem->thrTkr);
    filterStatus->setGemCalHiLo(gem->calHiLo);
    filterStatus->setGemCondsumCno(gem->condsumCno);
    filterStatus->setGemAcd_vetoes_XZ(gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XZ]);
    filterStatus->setGemAcd_vetoes_YZ(gem->acd.vetoes[EBF_GEM_ACD_VETO_K_YZ]);
    filterStatus->setGemAcd_vetoes_XY(gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XY]);
    filterStatus->setGemAcd_vetoes_RU(gem->acd.vetoes[EBF_GEM_ACD_VETO_K_RU]);
    filterStatus->setGemLivetime(gem->livetime);
    filterStatus->setGemTrgtime(gem->trgtime);
    filterStatus->setGemPpstime(gem->ppstime);
    filterStatus->setGemDiscarded(gem->discarded);
    filterStatus->setGemPrescaled(gem->prescaled);

    // This appears to be missing?
    //m_filterStatus->setGemSent(myFilterInfo.sent);

    unsigned int acd_top = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XY];
    unsigned int acd_x   = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_YZ];
    unsigned int acd_y   = gem->acd.vetoes[EBF_GEM_ACD_VETO_K_XZ];

    filterStatus->setAcdMap(acd_y,acd_x,acd_top);

    return;
   
}

void GemFilterOutput::eorProcessing(MsgStream& log)
{
    return;
}
