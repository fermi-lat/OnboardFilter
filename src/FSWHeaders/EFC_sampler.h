#ifndef EFC_SAMPLER_H
#define EFC_SAMPLER_H


// Define the following because the fsw header file uses C++ keywords
/* ---------------------------------------------------------------------- */
// Local struct definition for veto mask (must be better way...)
typedef struct _EFC_samplerPrescaleCounter
{
  unsigned int countdown;   /*!< Current countdown value                  */
  unsigned int   refresh;   /*!< Refresh value                            */
} EFC_samplerPrescaleCounter;

typedef struct _EFC_samplerPrescaleCtx
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
} EFC_samplerPrescaleCtx;

typedef struct _EFC_samplerClasses
{
    EFC_classes   defined; /*!< The set of defined class of bits          */
    EFC_classes   enabled; /*!< The set so enabled class of bits          */
} EFC_samplerClasses;

typedef struct _EFC_sampler
{
    EFC_samplerPrescaleCtx  prescale; /*!< The prescale context           */
    EFC_samplerClasses       classes; /*!< The status word classes        */
} EFC_sampler;

/* ---------------------------------------------------------------------- */



#endif
