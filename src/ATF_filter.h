#ifndef ATF_FILTER_H
#define ATF_FILTER_H


/* ---------------------------------------------------------------------- *//*!
   
   \file   ATF_filter.h
   \brief  Defines the interface to implement the Acd/Tkr Fast filter
   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $Id$
\endverbatim 

                                                                          */
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

                                                                          *//*!
  \def   ATF_FILTER_PRINTF(_arg)
  \brief Debugging macro -- activated by defining ATF_TKRVETO_PRINT
                                                                          */
/* ---------------------------------------------------------------------- */  
#ifdef  ATF_FILTER_PRINT

#define ATF_FILTER_PRINTF(_args) printf _args

#else

#define ATF_FILTER_PRINTF(_args)

#endif
/* ---------------------------------------------------------------------- */  



  
#ifdef __cpluscplus
extern "C" {
#endif

    

/* ---------------------------------------------------------------------- */
static __inline int ATF__filterSideMasksGet (int start,
                                             int length);
    
static __inline int ATF__filterTop          (int   tower,
                                             int   start,
                                             int acd_top);
    
static __inline int ATF__filterSides        (int tower,
                                             int tower_mask,
                                             int start,
                                             int length,
                                             int xm_candidates, int xm,
                                             int xp_candidates, int xp,
                                             int ym_candidates, int ym,
                                             int yp_candidates, int yp);
/* ---------------------------------------------------------------------- */

    


/* ---------------------------------------------------------------------- *//*!

  \fn        int ATF__filterTop (int tower,int start, int acd_top)
  \brief     Vetos an event based on whether the track, as determined by
             the layer bit masks, is associated with an ACD tile.
  \return    The subset of the bits in \a acd_top that are matched for
             this tower. If 0, none match.
  
  \param    tower  The tower number being examined
  \param    start  Starting layer of the track
  \param  acd_top  Bit mask of which ACD tiles are hit in the top plane
 
  The decision to keep or pitch the event is based on whether the X/Y
  layer OR trigger masks align with an appropriate ACD tile. The tracker
  layer masks have been examined, producing a candidate tower. 
  
  If the coincidence began in the top layer, the top plane of ACD tiles
  are examined for a coincidence. Since the resolution is crude, the
  coincidence can be any one of 4 tiles.

 \warning 
  The list of ACD tiles associated with a tower is determined by
  examining the diagram below.

 \code
 
  TOP COINCIDENCE PATTERN    Tower   ACD Tiles    Tower     ACD Tiles
  -----------------------    -----   ----------   -----   -----------   
  ACD  20  21  22  23  24        0   0, 1, 5, 6       8   10,11,15,16
  twr    12  13  14  15          1   1, 2, 6, 7       9   11,12,16,17
  ACD  15  16  17  18  19        2   2, 3, 7, 8      10   12,13,17,18
  twr     8   9  10  11          3   3, 4, 8, 9      11   13,14,18,19
  ACD  10  11  12  13  14        4   5, 6,10,11      12   15,16,20,21
  twr     4  <5>  6   7      ->  5   6, 7,11,12 <-   13   16,17,21,22
  ACD  05  06  07  08  09        6   7, 8,12,13      14   17,18,22,23
  twr     0   1   2   3          7   8, 9,13,14      15   18,19,23,24    
  ACD  00  01  02  03  04

 \endcode

  For example, concentrate on \e twr \e 5 in the left most diagram. The
  shadowing ACD tiles are <em>6,7,11,12</em>. Now locate tower 5 in the
  middle cluster of numbers and note that tower 5 is indeed associated
  with tiles <em>6,7,11,12</em>

  A little numerology shows that the proper bit mask is formed if one
  takes the following bit pattern

 \verbatim 
        6543210
  P = 0b1100011
 \endverbatim

  and shifts it by P << (tower + (tower>>2)). Effectively the 'tower'
  term marches one along in the X dimension, and the 'tower>>2' term
  advances the Y dimension. One could have used a lookup table, but
  that is a memory reference which may or may not be cached. This
  calculation is a couple of simple register-to-register instructions.
                                                                          */
/* ---------------------------------------------------------------------- */
static __inline int ATF__filterTop (int tower, int start, int acd_top)
{
   int          mt;
   int coincidence;
       
   mt          = tower + ((tower >> 2) & 0x3);  
   coincidence = (acd_top) & ((0x3 | (0x3<<5)) << mt);


   ATF_FILTER_PRINTF((" * %2d %8.8x %8.8x", mt, acd_top, coincidence));
   if (coincidence) ATF_FILTER_PRINTF (("!"));

   return coincidence;
}
/* ---------------------------------------------------------------------- */

    



/* ---------------------------------------------------------------------- *//*!

  \fn     int ATF__filterSideMasksGet (int start, int length)
  \brief  Returns the list of ACD tiles associated with a side exiting
          track.
  \return A bit mask representing the list of ACD tiles associated
          with a track of the specified length and starting point
          exiting the side of the specified tower.

  \param  start The starting layer of the track (0 is closest to the ACD 
                top tiles).
  \param length The length of the track, in layers

  While the list of ACD tiles associated a track exiting the top of tower
  can be determined algorithmically, unfortunately this is not the case
  for a side exiting track. The only recourse is to build a lookup table.

  However, building a straight lookup table based on the tower, the
  track's starting point and its length, would be prohibitively large,
  ensuring that it will never be cached. Therefore, a combination of
  a lookup table and algorithm is used to form the mask.

  The association is only in the vertical direction for the lowest
  numbered corner tower. The routine assumes the caller will adjust the
  mask based on the tower number and orientation. This last clause is
  why this routine cannot produce the correct tower mask based solely on
  the tower number. The corner towers have two different masks, for
  the two faces that are exposed.
                                                                          */
/* ---------------------------------------------------------------------- */  
static __inline int ATF__filterSideMasksGet (int start, int length)
{
   int mask;
 

   static const unsigned int AcdSideMasks[32] =
   {
      0x55555555, 0x00000000, /* Layer  0 */
      0x15555555, 0x00000000, /* Layer  1 */
      0x05555555, 0x00000000, /* Layer  2 */
      0x01555555, 0x00000000, /* Layer  3 */
      0x00555555, 0x00000000, /* Layer  4 */
      0x003fffff, 0x00000000, /* Layer  5 */
      0x000fffff, 0x00000000, /* Layer  6 */
      0x0003ffff, 0x00000000, /* Layer  7 */
      0x0000ffff, 0x00000000, /* Layer  8 */
      0x00003fff, 0x00000000, /* Layer  9 */
      0x00000fff, 0x00000000, /* Layer 10 */
      0x000003ff, 0x00000000, /* Layer 11 */
      0x000000ff, 0x00000055, /* Layer 12 */
      0x0000003f, 0x00000015, /* Layer 13 */
      0x0000000f, 0x00000005, /* Layer 14 */
      0x00000003, 0x00000001  /* Layer 15 */
   };
   
   /*
    | Extract the 4 bits that indicate which Z layers to check
    | The 4 bits are simply a mask of the 4 ACD side tile rows.
    | The length arguement justifies the mask. It is assumed
    | that there are at least 3 layers in a row hit. This means
    | that there is no justification when there are 3-in-a-row.
    | The doubling (<<1) for both the start and length variables
    | is because the element in the mask array is 2 bits wide.
   */
   length  = (length - 3) << 1;
   start <<=  1;
   mask    =  (AcdSideMasks[start]   >> length) & 0x3;
   mask   |= ((AcdSideMasks[start+1] >> length) & 0x3) << 2;

      
   /*
    | Remap these bits to be 5 bits apart, then duplicate adjacent
    | bits, because every tower is shadowed by 2 adjacent ACD tiles
   */
   mask  =  (mask & 1)
         | ((mask & 2) << ( 5-1))
         | ((mask & 4) << (10-2))
         | ((mask & 8) << (15-3));
   mask |=  (mask << 1);

   return mask;
}
/* ---------------------------------------------------------------------- */





    


/* ---------------------------------------------------------------------- *//*!

  \fn int ATF__filterSides (int tower,
                            int tower_mask,
                            int start,
                            int length,
                            int xm_candidates, int xm,
                            int xp_candidates, int xp,
                            int ym_candidates, int ym,
                            int yp_candidates, int yp)
  \brief  Looks for a ACD coincidence with a side exiting track.
  \param  tower         The tower to be examined.
  \param  tower_mask    The towers to consider, expressed as a bit mask.
  \param  start         The starting layer (0 is closest to the ACD top tiles)
  \param  length        The length of the coincidence. This must
                        be a minimum of 3 layers, in keeping with
                        the 3-in-a-row philosophy.
  \param  xm_candidates The towers to consider along the XM face.
  \param  xm            The struck ACD XM face tiles.
  \param  xp_candidates The towers to consider along the XP face.
  \param  xp            The struck ACD XP face tiles.
  \param  ym_candidates The towers to consider along the YM face.
  \param  ym            The struck ACD YM face tiles.  
  \param  yp_candidates The towers to consider along the YP face
  \param  yp            The struck ACD YP face tiles.  
  \retval       == 0, if no coincidence
  \retval       != 0, if have a coincidence
  
  This routine looks for a coincidence between a side exiting track and
  associated ACD tiles.  Only the appropriate ACD plane is consulted.
  In particular, inner towers are not exposed, so no ACD tile can be in
  coincidence. Corner towers consult both 1 X and 1 Y plane, while 'middle'
  towers only consult the appropriate X or Y plane. At most 2 planes may
  be consulted for the corner towers.
                                                                          */
/* ---------------------------------------------------------------------- */
static int ATF__filterSides (int tower,
                             int tower_mask,
                             int start,
                             int length,
                             int xm_candidates, int xm,
                             int xp_candidates, int xp,
                             int ym_candidates, int ym,
                             int yp_candidates, int yp)
{
   int  mask;
   int match;

   mask  = ATF__filterSideMasksGet (start, length);
   match = 0;
   
   
   if (xm_candidates & tower_mask)
   {
       int col = (tower >> 2) & 0x3;
       match  |= xm & (mask << col);
   }

               
   if (xp_candidates & tower_mask)
   {
       int col = (tower >> 2) & 0x3;
       match  |= xp & (mask << col);
   }
   
               
   if (ym_candidates & tower_mask)
   {
       int row  = (tower & 0x3);
       match   |= ym & (mask << row);
   }

               
   if (yp_candidates & tower_mask)
   {
       int row  = (tower & 0x3);                     
       match   |= yp & (mask << row);
   }
               

   return match;
}
/* ---------------------------------------------------------------------- */



#ifdef __cplusplus
}
#endif


#endif
