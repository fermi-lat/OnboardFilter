/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  DFC_ctlAfc.c
   \brief Routines to size and initialize the AFC filtering control
          parameters
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/DFC_ctl.h"
#include "DFC_ctlAfcDef.h"


#ifdef __cplusplus
extern "C" {
#endif

    
/* ---------------------------------------------------------------------- *//*!

  \var    const struct _AFC_splashMap AFC_SplashMap1x1
  \brief  Defines the 'far' neighbors of a given filter tile to be
          those tiles outside the tiles that touch that tile.

                                                                          */
/* ---------------------------------------------------------------------- */
extern const struct _AFC_splashMap AFC_SplashMap1x1;
/* ---------------------------------------------------------------------- */

    
#ifdef __cplusplus
}
#endif

/* ---------------------------------------------------------------------- *//*!

  \fn     int DFC_ctlAfcSizeof (void)
  \brief  Returns the size, in bytes, of the AFC filtering control
          parameters
  \return The size, int bytes of the AFC filtering control parameters        
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_ctlAfcSizeof (void)
{
   return sizeof (DFC_ctlAfc);
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!
   
  \fn int DFC_ctlAfcInit (struct _DFC_ctlAfc *afc)
  \brief      Initializes the AFC filtering control parameters
  \param  atf Pointer to the AFC filtering parameters to initialize
  \return     Status
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_ctlAfcInit (struct _DFC_ctlAfc *afc)
{
   afc->splashMap = AFC_SplashMap1x1;
   return 0;
}












