/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  DFC_ctlAtf.c
   \brief Routines to size and initialize the ATF filtering control
          parameters
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/DFC_ctl.h"

#include "ATF_shadowedTowersCtl.h"
#include "DFC_ctlAtfDef.h"


#ifdef __cplusplus
extern "C" {
#endif

#if 0
/* ---------------------------------------------------------------------- *//*!

  \var    ATF_ShadowedTowers2x2;
  \brief  Defines the shadowing towers to be consistent with the overlapping
          2 x 2 set of ACD tiles. Given an ACD tile, this data structure
          returns the set of shadowed towers.
                                                                          */
/* ---------------------------------------------------------------------- */
#endif    
extern const ATF_shadowedTowersCtl ATF_ShadowedTowers2x2;
/* ---------------------------------------------------------------------- */

    
#ifdef __cplusplus
}
#endif

/* ---------------------------------------------------------------------- *//*!

  \fn DFC_ctlAtfSizeof (void)
  \brief  Returns the size, in bytes, of the ATF filtering control
          parameters
  \return The size, int bytes of the ATF filtering control parameters        
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_ctlAtfSizeof (void)
{
   return sizeof (DFC_ctlAtf);
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!
   
  \fn int DFC_ctlAtfInit (struct _DFC_ctlAtf *atf)
  \brief      Initializes the ATF filtering control parameters
  \param  atf Pointer to the ATF filtering parameters to initialize
  \return     Status
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_ctlAtfInit (struct _DFC_ctlAtf *atf)
{
   atf->shadowed = ATF_ShadowedTowers2x2;
   return 0;
}












