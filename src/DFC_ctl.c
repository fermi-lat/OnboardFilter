/* ---------------------------------------------------------------------- *//*!
   
   \file  DFC_ctl.c
   \brief Routines to size and locate the filtering control parameters for
          each of the subsystems, Implementation
   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $Id
\endverbatim
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/DFC_ctl.h"
#include "DFC_ctlDef.h"



/* ---------------------------------------------------------------------- *//*!

  \fn     struct _DFC_ctlAfc *DFC_ctlAfcLocate (struct _DFC_ctl *dfc)
  \brief  Locates the AFC filtering control parameters
  \return Returns a pointer to the AFC filtering control parameters
                                                                          */
/* ---------------------------------------------------------------------- */
extern struct _DFC_ctlAfc *DFC_ctlAfcLocate (struct _DFC_ctl *dfc)
{
   return &dfc->afc;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     struct _DFC_ctlAtf *DFC_ctlAtfLocate (struct _DFC_ctl *dfc)
  \brief  Locates the ATF filtering control parameters
  \return Returns a pointer to the ATF filtering control parameters
                                                                          */
/* ---------------------------------------------------------------------- */
extern struct _DFC_ctlAtf *DFC_ctlAtfLocate (struct _DFC_ctl *dfc)
{
   return &dfc->atf;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn     struct _DFC_ctlCfc *DFC_ctlCfcLocate (struct _DFC_ctl *dfc)
  \brief  Locates the CAL filtering control parameters
  \return Returns a pointer to the CAL filtering control parameters
                                                                          */
/* ---------------------------------------------------------------------- */
extern struct _DFC_ctlCfc *DFC_ctlCfcLocate (struct _DFC_ctl *dfc)
{
   return &dfc->cfc;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn     struct _DFC_ctlSys *DFC_ctlSysLocate (struct _DFC_ctl *dfc)
  \brief  Locates the system filtering control parameters
  \return Returns a pointer to the system filtering control parameters
                                                                          */
/* ---------------------------------------------------------------------- */
extern struct _DFC_ctlSys *DFC_ctlSysLocate (struct _DFC_ctl *dfc)
{
   return &dfc->sys;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn     struct _DFC_ctlTfc *DFC_ctlTfcLocate (struct _DFC_ctl *dfc)
  \brief  Locates the TKR filtering control parameters
  \return Returns a pointer to the TKR filtering control parameters
                                                                          */
/* ---------------------------------------------------------------------- */
extern struct _DFC_ctlTfc *DFC_ctlTfcLocate (struct _DFC_ctl *dfc)
{
   return &dfc->tfc;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn     int DFC_ctlSizeof (void)
  \brief  Sizes the filtering control parameters
  \return Returns the size of the filtering control parameters
                                                                          */
/* ---------------------------------------------------------------------- */
extern int DFC_ctlSizeof (void)
{
   return sizeof (DFC_ctl);
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn     int DFC_ctlSysSizeof (void)
  \brief  Sizes the system filtering control parameters
  \return Returns the size of the system filtering control parameters
                                                                          */
/* ---------------------------------------------------------------------- */
extern int DFC_ctlSysSizeof (void)
{
   return sizeof (DFC_ctlSys);
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn        int DFC_ctlInit (struct _DFC_ctl            *dfc, 
                              const struct _TFC_geometry *geo)
  \brief     Provides default initialization of control structure
  \param dfc Pointer to the filtering process control parameters
  \param geo Pointer to the track geometry. Only the pointer is saved
             so the backing memory must remain valid.
  \return    Status
                                                                          */
/* ---------------------------------------------------------------------- */
extern int DFC_ctlInit (struct _DFC_ctl      *dfc,
		const 	struct _TFC_geometry *geo)
{
   DFC_ctlAfcInit (&dfc->afc);
   DFC_ctlAtfInit (&dfc->atf);
   DFC_ctlCfcInit (&dfc->cfc);
   DFC_ctlTfcInit (&dfc->tfc, geo);

   return 0;
}
/* ---------------------------------------------------------------------- */

  



