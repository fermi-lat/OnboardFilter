/* ---------------------------------------------------------------------- *//*!
   
   \file   EBF_gemPrint.c
   \brief  Produces an ASCII display of the GEM information
   \author JJRussell - russell@slac.stanford.edu
   
\verbatim
    CVS $Id: EBF_gemPrint.c,v 1.1 2004/07/12 18:09:12 golpa Exp $
    
\endverbatim
                                                                         */
/* --------------------------------------------------------------------- */


#include <stdio.h>
#include "DFC/EBF_gem.h"
#include "DFC/EBF_gemPrint.h"



/* --------------------------------------------------------------------- *//*!

  \fn    void EBF_gemPrint (const struct _EBF_gem *gem)
  \brief Prints an ASCII display of the GEM record

  \param gem  Point to the GEM data
                                                                         */
/* --------------------------------------------------------------------- */
void EBF_gemPrint (const struct _EBF_gem *gem)
{
   printf (
  " GEM Record\n"
  " ----------\n"
  " Trigger :    ThrTkr %8.8x   CalHiLo %8.8x CnoTreq %8.8x\n"
  " Acd     :        XZ %8.8x        YZ %8.8x      XY %8.8x   RU %8.8x\n"
  " Times   :      Live %8.8x     Event %8.8x     PPS %8.8x\n"
  " Counters: Throttled %8.8x Prescaled %8.8x    Sent %8.8x\n",
  gem->thrTkr,   gem->calHiLo,  gem->cnoReqvec,
  gem->acd.vetoes[EBF_K_GEM_ACD_VETO_XZ],
  gem->acd.vetoes[EBF_K_GEM_ACD_VETO_YZ],
  gem->acd.vetoes[EBF_K_GEM_ACD_VETO_XY],
  gem->acd.vetoes[EBF_K_GEM_ACD_VETO_RU],
  gem->deadtime,  gem->evttime,    gem->ppstime,
  gem->throttled, gem->prescaled,  gem->sent);

   return;
}
/* --------------------------------------------------------------------- */






