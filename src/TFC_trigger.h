#ifndef TFC_TRIGGER_H
#define TFC_TRIGGER_H



/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/




/* ---------------------------------------------------------------------- *//*!
   
   \file   TFC_trigger.h
   \brief  Defines the contents of the 32 bit word returned by the
           TKR trigger formation routines. This word includes an
           18 bit coincidence pattern, the start, length and type
           of the coincidence. The word is arranged such that negative
           values indicate no 6/6 or 7/8 coincidence.

           The word is defined both in terms of the standard shifts
           and masks needed to extract the fields and in terms of a
           bit field structure.

   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/DFC_endianness.h"

#ifdef __cplusplus
extern "C" {
#endif    

/* ---------------------------------------------------------------------- *//*!

  \enum  _TFC_K_triggerType
  \brief  Enumerates the type of coincidence achieved
                                                                          *//*!
  \typedef TFC_K_triggerType
  \brief   Typedef for enum \e _TFC_K_triggerType

   This defines the values that the type field of the trigger coincidence
   can assume.
                                                                          */
/* ---------------------------------------------------------------------- */
typedef enum _TFC_K_triggerType
{
  TFC_K_TRIGGER_TYPE_NONE   =  0, /*!< No coincidence of 4/4 or greater   */ 
  TFC_K_TRIGGER_TYPE_4_OF_4 =  1, /*!< Achieved a 4/4 coincidence         */  
  TFC_K_TRIGGER_TYPE_6_OF_6 =  2, /*!< Achieved a 6/6 coincidence         */
  TFC_K_TRIGGER_TYPE_7_OF_8 =  3  /*!< Achieved a 7/8 coincidence         */
}
TFC_K_triggerType;
/* ---------------------------------------------------------------------- */

    

    
/* ---------------------------------------------------------------------- *//*!

  \enum  _TFC_S_trigger
  \brief  Maps out the size of the trigger coincidence word
                                                                          *//*!
  \typedef TFC_S_trigger
  \brief  Typedef for enum \e _TFC_S_trigger

   The trigger results summarizes the layer hit information from a single
   tower. The low 17 bits summarizes the X/Y coincidence. The level of
   the coincidence is always just before any failing step.

  \verbatim
      Level Type Layers
        4/4    3 Failure: Return the 2/2 coincidence bits
                 Success: Try 7/8
                
        7/8      Failure: Try 6/6
               1 Success: Return the 7/8 coincidence bits
                
        6/6    2 Failure: Return the 4/4 coincidence bits
               0 Success: Return The 6/6 coincidence bits

      The \e start and \e length fields are relevant only for types
      0 and 1.
  \endverbatim      
                                                                          */
/* ---------------------------------------------------------------------- */
typedef enum _TFC_S_trigger
{    
  TFC_S_TRIGGER_LAYERS = 18,  /*!< The layer coincidence bits             */
  TFC_S_TRIGGER_START  =  5,  /*!< Start of coincidence (nearest ACD      */
  TFC_S_TRIGGER_LENGTH =  5,  /*!< Number of layers in coincidence        */
  TFC_S_TRIGGER_RSVD   =  2,  /*!< Reserved for future expansion          */
  TFC_S_TRIGGER_TYPE   =  2   /*!< See TFC_K_triggerType for the
                                   definitions of the values of this field*/
} TFC_S_trigger;
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \enum  _TFC_V_trigger
  \brief  Defines the shift needed to right justify the field
                                                                          *//*!
  \typedef TFC_V_trigger
  \brief  Typedef for enum \e _TFC_V_trigger
                                                                          */
/* ---------------------------------------------------------------------- */
typedef enum _TFC_V_trigger
{
  TFC_V_TRIGGER_LAYERS = 0,
                         /* Bit offset to LSB of the trigger layers field */
  
  TFC_V_TRIGGER_START  = TFC_V_TRIGGER_LAYERS + TFC_S_TRIGGER_LAYERS,
                         /* Bit offset to LSB of the trigger start  field */
  
  TFC_V_TRIGGER_LENGTH = TFC_V_TRIGGER_START  + TFC_S_TRIGGER_START,
                         /* Bit offset to LSB of the trigger length  field */
  
  TFC_V_TRIGGER_RSVD   = TFC_V_TRIGGER_LENGTH + TFC_S_TRIGGER_LENGTH,
                         /* Bit offset to LSB of the trigger rsvd    field */
  
  TFC_V_TRIGGER_TYPE   = TFC_V_TRIGGER_RSVD   + TFC_S_TRIGGER_RSVD
                         /* Bit offset to LSB of the trigger type    field */  
}
TFC_V_trigger;
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \enum  _TFC_M_trigger
  \brief  Defines in place mask for the specified field
                                                                          *//*!
  \typedef TFC_M_trigger
  \brief  Typedef for enum \e _TFC_M_trigger
                                                                          */
/* ---------------------------------------------------------------------- */
typedef enum _TFC_M_trigger
{
  TFC_M_TRIGGER_LAYERS = ((1<<TFC_S_TRIGGER_LAYERS)-1)<<TFC_V_TRIGGER_LAYERS,
                         /* In place mask for the trigger layers field    */
  
  TFC_M_TRIGGER_START  = ((1<<TFC_S_TRIGGER_START )-1)<<TFC_V_TRIGGER_START,
                         /* In place mask for the trigger start  field    */
  
  TFC_M_TRIGGER_LENGTH = ((1<<TFC_S_TRIGGER_LENGTH)-1)<<TFC_V_TRIGGER_LENGTH,
                         /* In place mask for the trigger length  field   */
  
  TFC_M_TRIGGER_RSVD   = ((1<<TFC_S_TRIGGER_RSVD  )-1)<<TFC_V_TRIGGER_RSVD,
                         /* In place mask for the trigger rsvd  field     */
  
  TFC_M_TRIGGER_TYPE   = ((1<<TFC_S_TRIGGER_TYPE  )-1)<<TFC_V_TRIGGER_TYPE
                         /* In place mask for the trigger type  field     */
}
TFC_M_trigger;
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

 \struct _TFC_trigger_bf
 \brief  Structure to map out the bit fields of the trigger summary word

  The trigger summary word summarizes the trigger status. It gives an 18
  bit mask of the XY coincident layers and the start and length of the
  first acceptable coincidence layers (looking from the ACD TOP plane down)
  and the coincidence type, ie, 4/4, 6/6, 7/8.
                                                                          *//*!
  \typedef TFC_trigger_bf
  \brief   Typedef for struct \a _TFC_trigger_bf
                                                                          */
/* ---------------------------------------------------------------------- */

#if DFC_C_ENDIANNESS == DFC_K_ENDIANNESS_LITTLE

typedef struct _TFC_trigger_bf
{
  unsigned int layers:TFC_S_TRIGGER_LAYERS; /*!< Coincident layers        */
  unsigned int  start:TFC_S_TRIGGER_START;  /*!< Coincidence begin layer  */
  unsigned int length:TFC_S_TRIGGER_LENGTH; /*!< Coincidence length       */
  unsigned int   rsvd:TFC_S_TRIGGER_RSVD;   /*!< Reserved for future use  */
  unsigned int   type:TFC_S_TRIGGER_TYPE;   /*!< Coincidence type         */
}
TFC_trigger_bf;

#else


typedef struct _TFC_trigger_bf
{
  unsigned int   type:TFC_S_TRIGGER_TYPE;   /*!< Coincidence type         */
  unsigned int   rsvd:TFC_S_TRIGGER_RSVD;   /*!< Reserved for future use  */
  unsigned int length:TFC_S_TRIGGER_LENGTH; /*!< Coincidence length       */
  unsigned int  start:TFC_S_TRIGGER_START;  /*!< Coincidence begin layer  */
  unsigned int layers:TFC_S_TRIGGER_LAYERS; /*!< Coincident layers        */
}
TFC_trigger_bf;

#endif
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \union _TFC_trigger
  \brief  Union of the bit field definition of the trigger coincidence
          results and plain 32 bit integers.
                                                                          *//*!
  \typedef TFC_trigger
  \brief   Typedef for union \e _TFC_trigger 
                                                                          */
/* ---------------------------------------------------------------------- */
typedef union _TFC_trigger
{
    TFC_trigger_bf bf;  /*!< The trigger coincidence word as bit fields   */
    unsigned int   ui;  /*!< The trigger coincidence word as unsigned int */
      signed int   si;  /*!< The trigger coincidence word as   signed int */
}
TFC_trigger;
/* ---------------------------------------------------------------------- */



    
#ifdef __cplusplus
}
#endif    


#endif
