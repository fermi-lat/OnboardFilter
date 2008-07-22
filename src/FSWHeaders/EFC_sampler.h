#ifndef EFC_SAMPLERDEF_H
#define EFC_SAMPLERDEF_H


/* ---------------------------------------------------------------------- *//*!

  \file   EFC_samplerDef.h
  \brief  Defines the sampling structure
  \author JJRussell - russell@slac.stanford.edu

\verbatim

   CVS $Id: EFC_samplerDef.h,v 1.2 2007/06/24 22:35:29 russell Exp $
\endverbatim

                                                                          */
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *\
 *
 * HISTORY
 * -------
 *
 * DATE     WHO WHAT
 * -------- --- ---------------------------------------------------------
 * 10.06.05 jjr Created/Cloned from EFC _ gamma version
 *
\* ---------------------------------------------------------------------- */


#include "EFC/EFC.h"


/* ---------------------------------------------------------------------- *//*!

  \struct _EFC_samplerPrescaleCounter
  \brief   Controls the prescaling sampling criteria
                                                                          *//*!
  \typedef EFC_samplerPrescaleCounter
  \brief   Typedef for struct _EFC_ctlCfgSamplerPrescaleCounter
                                                                          */
/* ---------------------------------------------------------------------- */
struct _EFC_samplerPrescaleCounter
{
  unsigned int countdown;   /*!< Current countdown value                  */
  unsigned int   refresh;   /*!< Refresh value                            */
};
/* ---------------------------------------------------------------------- */
#ifndef EFC_SAMPLERPRESCALECOUNTER_TD
#define EFC_SAMPLERPRESCALECOUNTER_TD
typedef struct _EFC_samplerPrescaleCounter EFC_samplerPrescaleCounter;
#endif
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \struct _EFC_samplerPrescaleCtx
  \brief   The context block for a set of prescalers
                                                                          *//*!
  \typedef EFC_samplerPrescaleCtx
  \brief   Typedef for struct _EFC_samplerPrescaleCtx
                                                                          */
/* ---------------------------------------------------------------------- */
struct _EFC_samplerPrescaleCtx
{
    EFC_samplerPrescaleCounter
                         in;    /*!< Input prescaler                      */
    EFC_samplerPrescaleCounter
                        out;    /*!< Output prescaler                     */
    EFC_classes     enabled;    /*!< The classes of enabled prescalers    */
    EFC_samplerPrescaleCounter
              prescalers[32];   /*!< The prescales for each status bit
                                     an additional one for the output 
                                     prescaling                           */
};
/* ---------------------------------------------------------------------- */
#ifndef         EFC_SAMPLERPRESCALECTX_TD
#define         EFC_SAMPLERPRESCALECTX_TD
typedef struct _EFC_samplerPrescaleCtx    EFC_samplerPrescaleCtx;
#endif
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \struct _EFC_samplerClasses
  \brief   Controls the default and active class masks
                                                                          *//*!
  \typedef EFC_samplerClasses
  \brief   Typedef for struct _EFC_samplerClasses
                                                                          */
/* ---------------------------------------------------------------------- */
struct _EFC_samplerClasses
{
    EFC_classes   defined; /*!< The set of defined class of bits          */
    EFC_classes   enabled; /*!< The set so enabled class of bits          */
};
/* ---------------------------------------------------------------------- */
#ifndef         EFC_SAMPLERCLASSES_TD
#define         EFC_SAMPLERCLASSES_TD
typedef struct _EFC_samplerClasses EFC_samplerClasses;
#endif
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \struct _EFC_sampler
  \brief   Controls the sampling criteria
                                                                          *//*!
  \typedef EFC_sampler
  \brief   Typedef for struct _EFC_sampler
                                                                          */
/* ---------------------------------------------------------------------- */
struct _EFC_sampler
{
    EFC_samplerPrescaleCtx  prescale; /*!< The prescale context           */
    EFC_samplerClasses       classes; /*!< The status word classes        */
};
#ifndef         EFC_SAMPLER_TD
#define         EFC_SAMPLER_TD
typedef struct _EFC_sampler    EFC_sampler;
#endif
/* ---------------------------------------------------------------------- */


#endif
