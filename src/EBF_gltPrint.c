/* ---------------------------------------------------------------------- *//*!
   
   \file   EBF_gltPrint.c
   \brief  Produces an ASCII display of the GLT information
   \author JJRussell - russell@slac.stanford.edu
   
verbatim
    CVS $Id$
    
\endverbatim
                                                                         */
/* --------------------------------------------------------------------- */


#include <stdio.h>
#include "DFC/EBF_glt.h"
#include "DFC/EBF_gltPrint.h"



/* --------------------------------------------------------------------- *//*!

  \fn    void EBF_gltPrint (const struct _EBF_glt *glt)
  \brief Prints an ASCII display of the GLT record

  \param glt  Point to the GLT data
                                                                         */
/* --------------------------------------------------------------------- */
void EBF_gltPrint (const struct _EBF_glt *glt)
{
   printf (
  " GLT Record\n"
  " ----------\n"
  " Trigger :    ThrTkr %8.8x   CalHiLo %8.8x CnoTreq %8.8x\n"
  " Acd     :        XZ %8.8x        YZ %8.8x      XY %8.8x   RU %8.8x\n"
  " Times   :      Dead %8.8x     Event %8.8x     PPS %8.8x\n"
  " Counters: Throttled %8.8x Prescaled %8.8x    Sent %8.8x\n",
  glt->thrTkr,   glt->calHiLo,  glt->cnoReqvec,
  glt->acd.vetoes[EBF_K_GLT_ACD_VETO_XZ],
  glt->acd.vetoes[EBF_K_GLT_ACD_VETO_YZ],
  glt->acd.vetoes[EBF_K_GLT_ACD_VETO_XY],
  glt->acd.vetoes[EBF_K_GLT_ACD_VETO_RU],
  glt->deadtime,  glt->evttime,    glt->ppstime,
  glt->throttled, glt->prescaled,  glt->sent);

   return;
}
/* --------------------------------------------------------------------- */






