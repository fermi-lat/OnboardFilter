#ifndef ATF_SHADOWEDTOWERSCTL_H
#define ATF_SHADOWEDTOWERSCTL_H

/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   ATF_shadowedTowersCtl.h
   \brief  Defines the data structure needed to translate a list of
           ACD tiles into a list of shadowed towers.
   \author JJRussell - russell@slac.stanford.edu

   \verbatim

CVS $Id$
   \endverbatim 
                                                                          */
/* ---------------------------------------------------------------------- */





#ifdef __cplusplus
extern "C" {
#endif

    
/* ---------------------------------------------------------------------- *//*!

  \struct _ATF_shadowedTowersCtl
  \brief   The control structure containing the arrays which translate
           a list of struct tiles to a list of shadowed towers.
                                                                          *//*!
  \typedef ATF_shadowedTowersCtl
  \brief   Typedef for struct _ATF_shadowedTowersCtl
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _ATF_shadowedTowersCtl
{
   unsigned short int top[25]; /*!< Xlat of top tile # to shadowed towers */
   unsigned short int   x[15]; /*!< Xlat of X   tile # to shadowed towers */
   unsigned short int   y[15]; /*!< Xlat of Y   tile # to shadowed towers */
}
ATF_shadowedTowersCtl;
/* ---------------------------------------------------------------------- */
    

#ifdef __cplusplus
}
#endif

#endif
