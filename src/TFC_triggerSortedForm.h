#ifndef TFC_TRIGGERSORTEDFORM_H
#define TFC_TRIGGERSORTEDFORM_H

/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   TFC_triggerSortedForm.h
   \brief  Defines the interface needed to form a tracking trigger from
           a bit list of sorted hit tracker layers.
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */



#include "../src/ffs.h"
#include "../src/TFC_trigger.h"


/* ---------------------------------------------------------------------- */



#ifdef __cplusplus
extern "C" {
#endif    


    
/* ---------------------------------------------------------------------- */
static __inline int TFC__triggerSorted7of8Form(int x, int y, int xy00, int xy01);
static __inline int TFC__triggerSortedForm    (int x, int y);
/* ---------------------------------------------------------------------- */



    
/* ---------------------------------------------------------------------- *//*!

  \fn         int TFC__triggerSorted7of8Form (int x, int y, int xy00, int xy01)
  \brief      Forms the 7 / 8 coincidence pattern
  \param x    The bit mask of X layers with hits
  \param y    The bit mask of Y layers with hits
  \param xy00 The bit-wise and of the above two masks
  \param xy01 The bit-wise and of the xy00 and itselt shifted down by 1
  \return     The coincidence pattern, aligned such that a coincidence
              pattern beginning in layer 17 has bit 15 set.

  This forms the 7 of 8 pattern for the input masks. The calling convention
  is a little weird, but it is assumed that these two quantities have
  already been formed. The mask comes back aligned so that a coincidence
  in layer 17 begins in bit 15. This makes it consistent with the
  forming the 6 of 6 coincidence.

  If the coincidence pattern is over only 4 layers, the minimum, then only
  one bit will be set. If the pattern is over 5 layers, then 2 consecutive
  bits will be set. The MSB bit of the coincidence indicates the layer
  the coincidence started in. For example, if the coincidence started in
  the layer closest to the the ACD, layer 17 would be returned. Because
  there must be 4 layers in the coincidence, it can never start below
  layer 3.

  The 7 of 8 coincidence is formed by and'd all the possible 3-fold
  coincidence between the x and y with the OR of the 'left-out' layer.

  \verbatim
    = xy012 & (x_or_y_3)
    | xy013 & (x_or_y_2)
    | xy023 & (x_or_y_1)
    | xy123 & (x_or_y_0)
  \endverbatim  

  The notation xy012, for example, means the 6/6 coincidence of the X
  and Y of layers 0,1,2. Simarily, x_or_y_3, means the OR of the X
  and Y layer 3.

 \warning
  This is intended to be for internal usage only.
                                                                          */
/* ---------------------------------------------------------------------- */
static int TFC__triggerSorted7of8Form (int x, int y, int xy00, int xy01)
{
   int xy02, xy12, xy13, xy23;
   int x_or_y = x | y;

   /* Form all the pair-wise coincidence */
   xy02 = xy00 & (xy00 << 2);
   xy12 = xy01 << 1;
   xy13 = xy02 << 1;
   xy23 = xy12 << 1;

   /* Now form all the triple coincidence AND'd with the missing X or Y   */
   return (((xy01 & xy02) & ((x_or_y)<<3)) |          /* (xy012 & x|y(3)) */
           ((xy01 & xy13) & ((x_or_y)<<2)) |          /* (xy013 & x|y(2)) */
           ((xy02 & xy23) & ((x_or_y)<<1)) |          /* (xy023 & x|y(1)) */
           ((xy12 & xy23) & ((x_or_y)<<0)));          /* (xy123 & x|y(0)) */
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn             int TFC__triggerSortedForm (int x,int y)
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
                                                                          */
/* ---------------------------------------------------------------------- */  
static int TFC__triggerSortedForm (int x, int y)
{
   int      xy00;
   int      xy01;
   int   trigger;
   int     start;
   int    length;
   TFC_trigger t;

      
   /*
    | To get a 7/8 or 6/6, must have at least a 4/4 somewhere. This is
    | a quick check to eliminate towers with no possibility of a trigger.
    | The return value is initialize to just 2/2/ coincidence level.
   */
   xy00 = (x & y);
   xy01 = xy00 & (xy00 << 1);
   if (xy01 == 0)
   {
       /*
        | In this case, all the other trigger fields are 0, so just
        | return the lowest coincidence mask.
       */
       return xy00;
   }
   
   
   /*
    | Try going for 7/8 layer coincidence. In order to get the length
    | the coincidence (ie the number of struck planes) correct, the
    | length is initialized at 1. That is because the minimum coincidence
    | length for 7/8 is 4 planes and the minimum coincidence length for
    | 3/3 is 3 planes. Arbitrarily, the length of coincidence is baselined
    | at 3 planes.
   */
   trigger   = TFC__triggerSorted7of8Form (x, y, xy00, xy01);
   t.si      = 0;
   t.bf.type = TFC_K_TRIGGER_TYPE_7_OF_8;
   length    = 3;

   /*
    | If no 7/8, try 6/6 coincidence. In order to get the length of
    | the coincidence (ie the number of struck planes) correct, the
    | length is initialized at 2.
   */
   if (trigger == 0)
   {
      trigger  = xy01 & (xy00 << 2);
      if (trigger == 0)
      {
          /* Only made it to a 4/4 level */
          t.bf.type   = TFC_K_TRIGGER_TYPE_4_OF_4;
          t.bf.layers = xy01;
          return t.si;
      }

      /* Made it to 6/6 */
      t.bf.type = TFC_K_TRIGGER_TYPE_6_OF_6;
      length    = 2;
   }

   

   /*
    | Must have a trigger now. Find the starting layer number and the
    | length of the coincidence.
   */
   t.bf.layers = trigger;
   trigger   <<= 32 - 18;        
   start       = FFS (trigger);
   trigger   <<= start ;
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
