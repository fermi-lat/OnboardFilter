#ifndef TFC_TRIGGERFORM_H
#define TFC_TRIGGERFORM_H



/* ---------------------------------------------------------------------- *//*!
   
   \file   TFC_triggerForm.h
   \brief  Defines the interface and inline routines needed to form a
           tracking trigger from  a bit list of hit tracker layers.
   \author JJRussell - russell@slac.stanford.edu

\verbatim

 CVXS $Id
\endverbatim 

    A layer hit bit list is presented in the event builder data format.
    Each 18-bit accept word, one of the X layers and one for the Y layers,
    contains two sets of bits, one representing the odd layers one the even
    layers. The returned word contains summary information on the highest
    coincidence achieved, e.g. none, 4/4, 6/6 or 7/8, plus information on
    the start layer and the length of the coincidence closest to the top
    of the LAT. See \e TFC_trigger.h for the exact definition of the
    returned word.   
                                                                          */
/* ---------------------------------------------------------------------- */



#include "../src/ffs.h"
#include "../DFC/EBF_tkr.h"
#include "../src/TFC_trigger.h"


/* ---------------------------------------------------------------------- *//*!

  \def   TFC_TRIGGER_PRINT
  \brief If defined, then the internal debugging macro TFC_TRIGGER_PRINTF
         is activated.
                                                                          */
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \def   TFC_TRIGGER_PRINTF
  \brief Internal debugging macro, activated by defining TFC_TRIGGER_PRINT
                                                                          */
/* ---------------------------------------------------------------------- */
#define TFC_TRIGGER_PRINT
#undef  TFC_TRIGGER_PRINT




/* ---------------------------------------------------------------------- */

#ifdef TFC_TRIGGER_PRINT

#include <stdio.h>
#define TFC_TRIGGER_PRINTF(_args) printf _args

#else

#define TFC_TRIGGER_PRINTF(_args)

#endif
/* ---------------------------------------------------------------------- */

                   
                   

#ifdef __cplusplus
extern "C" {
#endif    

/* ---------------------------------------------------------------------- *//*!

  \def    OFFSET_EVEN
  \brief  An additional offset (from the right) in a 32-bit word from
          where the set of even layers normally begin.

          In order to properly form the coincidences, a buffer area
          between the bits representing the even and odd layers must
          be maintained. This defines the beginning of the set of bits
          representing the even layers. There is also an offset defined
          for the set of bits representing the odd bits. Only one of
          these is non-zero, but the code is written in such a way that
          the non-zero one can be either.
                                                                          *//*!
  \def    OFFSET_ODD
  \brief  An additional offset (from the right) in a 32-bit word from
          where the set of odd layers begin.

          In order to properly form the coincidences, a buffer area
          between the bits representing the even and odd layers must
          be maintained. This defines the beginning of the set of bits
          representing the idd layers. There is also an offset defined
          for the set of bits representing the even bits. Only one of
          these is non-zero, but the code is written in such a way that
          the non-zero one can be either.
                                                                          */
/* ---------------------------------------------------------------------- */
#define OFFSET_EVEN 0
#define OFFSET_ODD  7    
/* ---------------------------------------------------------------------- */

    


/* ---------------------------------------------------------------------- *//*!

  \def TFC_TRG_REMAP_INIT(_trigger, _even, _odd, _e, _o)
  \brief Remaps the specified even and odd layers from the trigger word
         to the canonical (LSB=Layer 0) order.
  \param _trigger The trigger word to remap
  \param _even    Variable to receive the remapped even layers
  \param _odd     Variable to receive the remapped odd  layers
  \param _e       The designator of the even layer to remap (0,2,4,6,8,A,C,E,G)
  \param _o       The designator of the odd  layer to remap (1,3,5,7,9,B,D,F,H)

   This differs from TFC_TRG_REMAP in that it initializes \a _even and
   _odd. This macro, together with TFC_TRG_REMAP allow TFC__triggerRemap
   to be implemented in a generic form.
                                                                          */
/* ---------------------------------------------------------------------- */



    
/* ---------------------------------------------------------------------- *//*!

  \def TFC_TRG_REMAP(_trigger, _even, _odd, _e, _o)
  \brief Remaps the specified even and odd layers from the trigger word
         to the canonical (LSB=Layer 0) order.
  \param _trigger The trigger word to remap
  \param _even    Variable to receive the remapped even layers
  \param _odd     Variable to receive the remapped odd  layers
  \param _e       The designator of the even layer to remap (0,2,4,6,8,A,C,E,G)
  \param _o       The designator of the odd  layer to remap (1,3,5,7,9,B,D,F,H)

  
   This differs from TFC_TRG_REMAP_INIT in that it believes that \a _even
   and _odd are already initialized. This macro, together with
   TFC_TRG_REMAP_INIT allow TFC__triggerRemap to be implemented in a
   generic form.

                                                                          */
/* ---------------------------------------------------------------------- */

#if  defined(VXWORKS) && 0

/*
 | Jan 23, 2004, removed %32 on (EBF_TKR_K_LEFT_BIT_L ## _e & _o)
 | The new assembler apparently does not like this. They are 
 | unnecessary and, in retrospect, probably not the right thing to
 | do. They must have no effect and if they did, the code is wrong
*/
#define TFC_TRG_REMAP_INIT(_trigger, _even, _odd, _e, _o)                 \
   asm volatile                                                           \
       (" rlwinm %0,%2,(EBF_TKR_K_LEFT_BIT_L ## _e+OFFSET_EVEN+1),        \
                       (31-EBF_TKR_K_L ## _e), (31-EBF_TKR_K_L ## _e))\n  \
          rlwinm %1,%2,(EBF_TKR_K_LEFT_BIT_L ## _o+OFFSET_ODD +1),        \
                       (31-EBF_TKR_K_L ## _o), (31-EBF_TKR_K_L ## _o))"   \
        : "=r"(_even), "=r"(_odd) : "r"(_trigger));

#define TFC_TRG_REMAP(_trigger, _even, _odd, _e, _o)                   \
   asm volatile                                                        \
       (" rlwimi %0,%2,(EBF_TKR_K_LEFT_BIT_L ## _e+OFFSET_EVEN+1),     \
                       (31-EBF_TKR_K_L ## _e), (31-EBF_TKR_K_L ## _e))    \
          rlwimi %1,%2,(EBF_TKR_K_LEFT_BIT_L ## _o+OFFSET_ODD +1),     \
                       (31-EBF_TKR_K_L ## _o), (31-EBF_TKR_K_L ## _o))"   \
        :               "=r"(_even), "=r"(_odd)                        \
        : "r"(_trigger), "0"(_even), "1"(_odd));

#else
        
#define TFC_TRG_REMAP_INIT(_trigger, _even, _odd, _e, _o)                  \
_even  =  ((((_trigger)>>(EBF_TKR_K_RIGHT_BIT_L ## _e + OFFSET_EVEN)) & 1) \
      << (EBF_TKR_K_L ## _e));                                             \
_odd   =  ((((_trigger)>>(EBF_TKR_K_RIGHT_BIT_L ## _o + OFFSET_ODD )) & 1) \
      << (EBF_TKR_K_L ## _o))

    
#define TFC_TRG_REMAP(_trigger, _even, _odd, _e, _o)                       \
_even |=  ((((_trigger)>>(EBF_TKR_K_RIGHT_BIT_L ## _e + OFFSET_EVEN)) & 1) \
      << (EBF_TKR_K_L ## _e));                                             \
_odd  |=  ((((_trigger)>>(EBF_TKR_K_RIGHT_BIT_L ## _o + OFFSET_ODD )) & 1) \
      << (EBF_TKR_K_L ## _o))
    
#endif
    
/* ---------------------------------------------------------------------- */
static __inline int TFC__triggerRemap     (int trigger);

static __inline int TFC__trigger7of8Form (unsigned int x,
                                        unsigned int y,
                                        unsigned int xy012,
                                        unsigned int xy013,
                                        unsigned int xy023,
                                        unsigned int xy123);

static __inline int TFC__triggerForm     (unsigned int x,
                                        unsigned int y);
/* ---------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

   \fn int TFC__triggerRemap (int trigger)
   \brief  Remaps the trigger pattern word into the more conventional
           order.
   
   \param trigger The trigger pattern word to remap. This is a bit mask
                  of the struck layers as produced by the hardware
   \return        The trigger pattern word in layer order
   

   \a TFC_triggerRemap remaps the trigger pattern word into the more
   conventional order. The PPC version is written in assembler, achieving
   about twice the performance of the normal C version.
 
   The accept word was first split into the odd and even portions. This
   provides some 'dead' area between the two, preventing accidental
   coincidences from being formed during the shifting operations. This
   means the remap operation found in EBF_tkr does not work, necessitating
   this special purpose remap.

   \verbatim

        0123456789abcdef0123456789abcdef
        fedcba9876543210fedcba9876543210
        --------------------------------
               13579bdfh       02468aceg

   \endverbatim
  
   The following remaps this to

   \verbatim
                      hgfdebca9876543210
   \endverbatim
  
   Another slight difference occurs because layers 0 and 1 can never
   start 3/3 and 4/4 type of coincidences, so these layers do not need
   to be remapped; a slight optimization.

   This routine has a platform dependent implementation. The PPC version
   takes advantage of the PPC CPUs that have 2 execution units, using
   the rlwimi instructions in pairs, one remapping the even layers, the
   other the odd layers.
                                                                         */
/* --------------------------------------------------------------------- */
static int TFC__triggerRemap (int trigger)
{
   /*
    | Two variables, 'even' and 'odd' are used during the remapping
    | even though, strictly speaking, only one is needed. Using two
    | variables allows both integer units on those processors that
    | have it to be used. Using an extra variable costs one additional
    | OR instruction, but allows 18 other instructions to be executed
    | in pairs.
    |
    | This routine should take on the order of 10 clock cycles, or
    | for 133 MHz RAD750, about 75nsecs.
   */
   unsigned int even;
   unsigned int  odd;

   /*
    | Note that layers 0 and 1 are not remapped. They can never begin
    | a coincidence, hence no need to remap them.
   */
   TFC_TRG_REMAP_INIT (trigger, even, odd, 2, 3);   
   TFC_TRG_REMAP      (trigger, even, odd, 4, 5);
   TFC_TRG_REMAP      (trigger, even, odd, 6, 7);
   TFC_TRG_REMAP      (trigger, even, odd, 8, 9);
   TFC_TRG_REMAP      (trigger, even, odd, A, B);
   TFC_TRG_REMAP      (trigger, even, odd, C, D);
   TFC_TRG_REMAP      (trigger, even, odd, E, F);
   TFC_TRG_REMAP      (trigger, even, odd, G, H);

   TFC_TRIGGER_PRINTF (("Original = %8.8x\n"
                        "Remap    = %8.8x\n",
                        trigger,
                        even | odd));
   

   return even | odd;
}
/* ---------------------------------------------------------------------- */


  

/* ---------------------------------------------------------------------- *//*!

  \fn     int TFC__trigger7of8Form (unsigned int x,
                                    unsigned int y,
                                    unsigned int xy012,
                                    unsigned int xy013,
                                    unsigned int xy023,
                                    unsigned int xy123)
  \brief       Forms the 7 / 8 coincidence pattern
  \param     x The bit mask of X layers with hits with Xodd << 16 | Xeven
  \param     y The bit mask of Y layers with hits with Yodd << 16 | Xeven
  \param xy012 The bit-wise and of layers 0,1 and 2
  \param xy013 The bit-wise and of layers 0,1 and 3
  \param xy023 The bit-wise and of layers 0,2 and 3
  \param xy123 The bit-wise and of layers 1,2 and 3
  \return      The coincidence pattern, aligned such that a coincidence
               pattern beginning in layer 17 has bit 15 set.

  This forms the 7 of 8 pattern for the input masks. The calling convention
  is a little weird, but it is assumed that that the triple coincidences
  have been formed.

  If the coincidence pattern is over only 4 layers, the minimum, then only
  one bit will be set. If the pattern is over 5 layers, then 2 consecutive
  bits will be set.

  The 7 of 8 coincidence is formed by and'd all the possible 3-fold
  coincidence between the x and y with the OR of the 'left-out' layer.

  \verbatim
    = xy012 & (x_or_y_3)
    | xy013 & (x_or_y_2)
    | xy023 & (x_or_y_1)
    | xy123 & (x_or_y_0)
  \endverbatim  

  The notation xy012, for example, means the 6/6 coincidence of the X
  and Y of layers 0,1,2. 

 \warning
  This is intended to be for internal usage only.
                                                                          */
/* ---------------------------------------------------------------------- */
static int TFC__trigger7of8Form (unsigned int x,
                                 unsigned int y, 
                                 unsigned int xy012,
                                 unsigned int xy013,
                                 unsigned int xy023,
                                 unsigned int xy123)
{
   unsigned int      xy;
   unsigned int x_or_y0;
   unsigned int x_or_y1;

   /*

    x_or_y0 =   (       13579bdfh       02468aceg)
    x_or_y1 =   (       02468aceg       _13579bdf)

                 fedcba9876543210fedcba9876543210    
      xy012 = xy(       13579bdfh       02468aceg)    xy00
            & xy(       02468aceg       _13579bdf)    xy11
            & xy(       _13579bdfh      _02468ace)    xy22
            &   (       _02468aceg      __13579bd)    x_or_y1 >> 1

      xy013 = xy(       13579bdfh       02468aceg)    xy00
            & xy(       02468aceg       _13579bdf)    xy11
            &   (       _13579bdfh      _02468ace)    x_or_y0 >> 1            
            & xy(       _02468aceg      __13579bdf)   xy33


      xy023 = xy(       13579bdfh       02468aceg)    xy00
            &   (       02468aceg       _13579bdfh)   x_or_y1      
            & xy(       _13579bdfh      _02468ace)    xy22
            & xy(       _02468aceg      __13579bdf)   xy33


      xy123 =   (       13579bdfh       02468aceg)    x_or_y0
            & xy(       02468aceg       _13579bdf)    xy11
            & xy(       _13579bdfh      _02468ace)    xy22
            & xy(       _02468aceg      __13579bdf)   xy33
            
   */
   x_or_y0 = (x | y);
   x_or_y1 = (x_or_y0 << 16) | (x_or_y0 >> 17);



   /* Now form all the triple coincidence AND'd with the missing X or Y   */
   xy   = ((xy012 & ((x_or_y1)>>1)) |          /* (xy321 & x|y(0)) */
           (xy013 & ((x_or_y0)>>1)) |          /* (xy320 & x|y(1)) */
           (xy023 & ((x_or_y1)>>0)) |          /* (xy310 & x|y(2)) */
           (xy123 & ((x_or_y0)>>0)));          /* (xy210 & x|y(3)) */

   
   return (int) xy;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn             int TFC__triggerForm (unsigned int x,
                                        unsigned int y)
  \brief          Looks for a 7/8 or 6/6 coincidence. If one is found
                  returns the starting layer and number of layers in
                  the coincidence.
  \param x        A bit mask of the X layers that have a hit strip.
  \param y        A bit mask of the Y layers that have a hit strip.
  retval          -1, No possibiity of a trigger
  retval          -2, A possibility of a trigger, but no 7/8 or 6/6
  retval          >0, The starting layer and length of the coincidence
                      packed together as two 16 bit words, with the
                      starting layer number in the MSB 16 bits.

  This routine attempts to look first for a coincidence of 4/4 layers,
  2 X and 2 Y. If this is not found, it is deemed that there is no
  possibility of finding a trigger in this tower. If a coincidence of
  4/4 layers is found, then one looks first for a string of 7/8. If
  that fails, 6/6. If both fail, a -2 is returned, indicating only
  a possibility of a trigger.

  If a 7/8 or 6/6 is found, the starting layer number (with 0 being
  closest to the set ACD top tiles) is returned. The length of the
  coincidence beyond a 6/6 (ie 0 indicates at least 3 X,Y pairs of
  layers have been crossed) is computed and returned. (Each X,Y
  pair counts as 2 in the coincidence length.)

  The bit masks for x and y must have the bits for the even layers
  in the low 16 bits and the bits for the odd layers in the high
  16 bits.
                                                                          */
/* ---------------------------------------------------------------------- */  
static int TFC__triggerForm (unsigned int x, unsigned int y)
{
   unsigned int      xt;
   unsigned int      yt;
   unsigned int    xy00;
   unsigned int    xy11;
   unsigned int    xy22;
   unsigned int    xy01;
   unsigned int    xy02;
   unsigned int    xy12;
   unsigned int    xy13;
   unsigned int    xy23;
   unsigned int   xy012;
   unsigned int   xy013;
   unsigned int   xy023;
   unsigned int   xy123;
   unsigned int trigger;
   int            start;
   int           length;
   TFC_trigger        t;
   

  /*
            876543210fedcba9876543210
       x =         13579bdfh02468aceg
       y =         13579bdfh02468aceg
       
      xt   = (x & 0x1ff) | ((x & (0x1ff << 9)) << 7));
      yt   = (y & 0x1ff) | ((y & (0x1ff << 9)) << 7));      
      
      xt  = 13579bdfh.......02468aceg
      xt  = 13579bdfh.......02468aceg


      xy00 = xt & yt;
      xy11 = (xy00 << 16) | (xy00 >> 17)
      xy22 = (xy00 >> 1)
      xy33 =  xy11 >> 1)

                fedcba9876543210fedcba9876543210 
      xy01 = xy(       13579bdfh       02468aceg)    xy00
           & xy(       02468aceg       _13579bdf)    xy11
           = xy00 &  xy11;

      xy02 = xy(       13579bdfh       02468aceg)    xy00
           = xy(       _13579bdfh      _02468ace)    xy22
           = xy00 & xy22;

      xy12 = xy(       02468aceg       _13579bdf)    xy11
           = xy(       _13579bdfh      _02468ace)    xy22
           =  xy11 & xy22;
             
      xy13 = xy(       02468aceg       _13579bdf)    xy11
           & xy(       _02468aceg       _13579bd)    xy33
           = xy11 & (xy11 >> 1);

      xy23 = xy(       _13579bdfh      _02468ace)    xy22
           & xy(       _02468aceg       _13579bd)    xy33
           = xy01 >> 1;
   */

   
   /*
    | To get a 7/8 or 6/6, must have at least a 4/4 somewhere. This is
    | a quick check to eliminate towers with no possibility of a trigger.
    | Initialize the return value to be the 2/2 coincidence.
   */
   xt   =  (x & 0x1ff) | ((x & (0x1ff << 9)) << 7);
   yt   =  (y & 0x1ff) | ((y & (0x1ff << 9)) << 7);
   xy00 = (xt & yt);
   xy11 = (xy00 << 16) | (xy00 >> 17);
   xy01 =  xy00 & xy11;
   TFC_TRIGGER_PRINTF (("X   : %8.8x\n"
                        "Y   : %8.8x\n"
                        "XT  : %8.8x\n"
                        "YT  : %8.8x\n"
                        "XY00: %8.8x\n"
                        "XY11: %8.8x\n"
                        "XY01: %8.8x\n",
                        x, y, xt, yt, xy00, xy11, xy01));
   
   if (xy01 == 0)
   {
       /*
        | No need to fill in remaining trigger fields, they are defaulted.
        | In this case and in this case only, the layer bits are still in
        | the accept order. For the most part no one ever looks at the
        | layer map when there is less than a 4/4 coincidence. Basically,
        | doing the remapping isn't worth the time. (Remember, this case
        | is the most common fate, so saving time here is worth this
        | complication.)
       */
       return x & y;
   }
   
   
   /*
    | Try going for 7/8 layer coincidence. In order to get the length
    | the coincidence (ie the number of struck planes) correct, the
    | length is initialized at 1. That is because the minimum coincidence
    | length for 7/8 is 4 planes and the minimum coincidence length for
    | 3/3 is 3 planes. Arbitrarily, the length of coincidence is baselined
    | at 3 planes.
   */
   xy22      = xy00 >> 1;
   xy02      = xy00 & xy22;
   xy12      = xy11 & xy22;
   xy13      = xy11 & (xy11 >> 1);
   xy23      = xy01 >> 1;
   xy012     = xy01 & xy22;
   xy013     = xy01 & xy13;
   xy023     = xy02 & xy23;
   xy123     = xy12 & xy23;
   trigger   = TFC__trigger7of8Form (xt, yt, xy012, xy013, xy023, xy123);
   
   TFC_TRIGGER_PRINTF (("7/8\n"
                        "XY22 :%8.8x\n"
                        "XY12 :%8.8x\n"
                        "XY13 :%8.8x\n"
                        "XY23 :%8.8x\n"
                        "XY012:%8.8x\n"
                        "XY013:%8.8x\n"
                        "XY023:%8.8x\n"
                        "XY123:%8.8x\n"
                        "T7/8: %8.8x\n",
                        xy22,  xy12,  xy13,  xy23,                        
                        xy012, xy013, xy023, xy123, trigger));

   
   length    = 3;
   t.si      = 0;
   t.bf.type = TFC_K_TRIGGER_TYPE_7_OF_8;

   
   /*
    | If no 7/8, try 6/6 coincidence. In order to get the length of
    | the coincidence (ie the number of struck planes) correct, the
    | length is initialized at 0.
   */
   if (trigger == 0)
   {
       /*
                       fedcba9876543210fedcba9876543210
            xy012 = xy(       13579bdfh       02468aceg)    xy00
                  & xy(       02468aceg       _13579bdf)    xy11
                  & xy(       _13579bdfh      _02468ace)    xy22
       */
      trigger = xy01 & xy02;
      
      TFC_TRIGGER_PRINTF (("6/6 : Attempt\n"
                           "XY01: %8.8x\n"
                           "XY02: %8.8x\n"
                           "T012: %8.8x\n",
                           xy01,
                           xy02,
                           trigger));

      /* No 6/6 of coincidence */  
      if (trigger == 0)
      {
          /* Only made it to 4/4 */
          t.si        = 0;
          t.bf.type   = TFC_K_TRIGGER_TYPE_4_OF_4;
          t.bf.layers = ((xy01 >> OFFSET_ODD) & (0x1ff << 9)) | (xy01 & 0x1ff);
          return t.si;
      }

      /* No 7/8, but did have a 6/6 */
      t.bf.type = TFC_K_TRIGGER_TYPE_6_OF_6;
      length    = 2;
   }
   else
   {
       TFC_TRIGGER_PRINTF (("7/8\n"
                            "XY02: %8.8x\n"
                            "XY12: %8.8x\n"
                            "XY13: %8.8x\n"
                            "XY23: %8.8x\n"
                            "XY012:%8.8x\n"
                            "XY013:%8.8x\n"
                            "XY023:%8.8x\n"
                            "XY123:%8.8x\n"
                            "T7/8: %8.8x\n",
                            xy02,  xy12,  xy13,  xy23,
                            xy012, xy013, xy023, xy123, trigger));
   }
   

   trigger = TFC__triggerRemap (trigger);

   
   /*
    | Must have a trigger now. Find the starting layer number and the
    | length of the coincidence. Note that the layer closest to the ACD
    | top plane is labelled as 0. 
   */
   t.bf.layers = trigger;   
   trigger   <<= 32 - 18;        
   start       = FFS (trigger);
   trigger   <<= start;
   trigger     = ~trigger;
   length     += FFS (trigger);

   t.bf.start  = start;
   t.bf.length = length;
   
   return t.si;
}
/* ---------------------------------------------------------------------- */


#ifdef __cplusplus
}
#endif


#endif

















