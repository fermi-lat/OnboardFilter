/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  AFC_splash.c
   \brief Evaluates the splash veto
   \author JJRussell - russell@slac.stanford.edu
                                                                         */
/* --------------------------------------------------------------------- */



#include "AFC_splashMap.h"
#include "AFC_splash.h"
#include "ffs.h"



/* --------------------------------------------------------------------- *//*!

   \def          MOVE_BITS(_w, _from, _width, _to)
   \brief        Moves a bit field from one spot to another
   \param _w     The word to extract the bit field from
   \param _from  The right most bit position (LSB = bit 0)
   \param _width The width of the bit field
   \param _to    The new right most bit position
   \return       The extracted, justified bit field

    This macro is not guaranteed to move bit fields round the boundaries,
    ie, perform wrap-around moves.
                                                                         */
/* --------------------------------------------------------------------- */
#define MOVE_BITS(_w, _from, _width, _to)  \
     ((((_w) >> (_from)) & ((1<<(_width)) - 1)) << (_to))
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

   \def       MSK(_n)
   \brief     Bit mask to bit _n set
   \param _n  The bit to set (MSB = bit 0)
   \return    The word with bit n set
                                                                         */
/* --------------------------------------------------------------------- */
#define MSK(_n) (0x80000000 >> (_n))



/* --------------------------------------------------------------------- *//*!

   \fn int evaluate1 (int acd, const unsigned int masks[32][2], int which)
   \brief  Evaluates whether the veto is satified for an individual word.
   \param    acd  The word to consider
   \param  which  Which mask to use.
   \param  masks  The bit mask of tiles considered as 'far' neighbors
   \retval        0
   \retval        1, if splash veto satisfied.
                                                                         */
/* --------------------------------------------------------------------- */
static inline int evaluate1 (int                         acd,
                             const unsigned int masks[32][2],
                             int                      which);


static int evaluate1 (int                         acd,
                      const unsigned int masks[32][2],
                      int                       which)
{
    int   n;
    int msk;
    int farVar;
    

    /*
     | This first section of code checks whether the first set bit
     | results in a far neighbor match.
     |
     | CASE 0: A far neighbor match was found.
     | In this case, one has acquired two pieces of knowledge
     |    1. There is a far neighbor match
     |    2. There are at least two struck tiles
     |
     | To satisfy the splash veto, all that is needed is to find
     | one remaining struck tile.. This is done by checking to see
     | if there are any remaining bits in the ACD tile mask after
     | eliminating the neighbor bits from the struck ACD mask.
     |
     | CASE 1: A far neighbor match was not found
     | In this case, one has acquired one piece of knowledge
     |    1. There is at least one struck tile.
     |
     | To satisfy the splash veto, all that is needed is to find
     | a far neighbor match, which then will satisfy the splash
     | requirement of a far neighbor match and 3 struck tiles. (One
     | of the struck tiles is in hand, the other two are from the
     | match.
     | 
    */
    n    = FFS (acd);              /* Find the set bit                    */
    acd &= ~MSK(n);                /* Eliminate it                        */
    msk  = masks[n][which];        /* Pick up the mask                    */
    farVar  = acd & msk;              /* Coincidence with 'far' tile?        */
    if (farVar)                       /* If have a 'far' tile hit            */
    {                              /* Just need one more tile hit         */
        n = FFS (farVar);             /* Locate which far tile hit           */
        acd &= ~MSK(n);            /* Eliminate first far tile hit        */
        if (acd) return 1;
                                   /* If anything left, have >2 tiles hit */
    }

    
    /* Have one, hit, all that is needed is a matched hit */
    while (acd)
    {
        n    = FFS (acd);    
        msk  = masks[n][which];
        acd &= ~MSK(n);
        if (acd & msk) return 1;
    }

    return 0;
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

   \fn int AFC_splash (const struct _AFC_splashMap *map,
                       int                      acd_top,
                       int                        acd_x,
                       int                        acd_y)
   \brief         Determines whether the ACD pattern of hits satisfies the
                  ACD splash veto condition.
   \param map     This map determines whether 2 tiles are considered
                  'near' or 'far' neighbors.
   \param acd_top The ACD TOP face hits, expressed as a bit mask.
   \param acd_x   The ACD X   face hits, expressed as a bit mask.
   \param acd_y   The ACD Y   face hits, expressed as a bit mask.
   \retval        0, if the condition is not satisfied
   \retval        1, if the condition is satisfied
   

   The splash condition is statisfied if there is at least 1 pair of
   'far' neighbors and at least 3 hits in the ACD tiles. The algorithm
   only considers the ACD TOP tiles and the upper two rows of the side
   tiles.
                                                                         */
/* --------------------------------------------------------------------- */
int AFC_splash (const struct _AFC_splashMap *map,
                int                      acd_top,
                int                        acd_x,
                int                        acd_y)
{
   int acd_0;
   int acd_1;
   
    /*
     | Build the veto masks
     |
     | Mask 0: All the top tiles (25) + tiles 2-8 of 7 YP
     | Mask 1: The first 10 XM, XP, YM  and tiles 0,1 of YP
     |
     | The end of this file contains an ASCII art rendition of these 2 words.
    */
   acd_0 = acd_top | MOVE_BITS (acd_y, 18, 7, 25);
   acd_1 = MOVE_BITS (acd_x,  0, 10,  0) | MOVE_BITS (acd_x, 16, 10, 10)
         | MOVE_BITS (acd_y,  0, 10, 20) | MOVE_BITS (acd_y, 16,  2, 30);

#if 0
   printf ("Top = %8.8x X = %8.8x Y = %8.8x\n"
           "ACD_0[00:24] = %8.8x\n"
           "ACD_0[25:31] = %8.8x\n"
           "ACD_1[00:09] = %8.8x\n"
           "ACD_1[10:19] = %8.8x\n"
           "ACD_1[20:29] = %8.8x\n"
           "ACD_1[30:31] = %8.8x\n"
           "Bit 64       = %8.8x\n",
           acd_top, acd_x, acd_y,
           acd_top,
           MOVE_BITS(acd_y, 18,  7, 25),
           MOVE_BITS(acd_x,  0, 10,  0),
           MOVE_BITS(acd_x, 16, 10, 10),
           MOVE_BITS(acd_y,  0, 10, 20),
           MOVE_BITS(acd_y, 16,  2, 30),
           acd_y & (1 << (16+9)));
#endif
   


   
   /* Check if bit 64 is set */
   if (acd_y & (1 << (16 + 9)))
   {
       int   n;
       int farVar;

       farVar = acd_0 & map->msk_64[0];
       if (farVar)
       {
           n = FFS (farVar);
           if (acd_1 | (acd_0 & ~MSK(n))) return 1;
       }
       
           
       farVar = acd_1 & map->msk_64[1];
       if (farVar)
       {
           n = FFS (farVar);
           if (acd_0 | (acd_1 & ~MSK(n))) return 1;
       }
   }
       
           
               

   /*
    |  Parse into 4 cases
    |      ACD_0 & ACD_1  both non-zero
    |      ACD_0          non zero (only)
    |      ACD_1          non zero (only)
    |                     both zero
   */
   if (acd_0)
   {
       if (acd_1)
       {
           /* Both are non-zero */
           int    n;
           int  msk;
           int  farVar;

           
           /* Have hits in both acd_0 and acd_1                             */
           n      = FFS (acd_0);            /* Find the set bit             */
           acd_0 &= ~MSK(n);                /* Eliminate it                 */
           msk    = map->msk_0[n][0];       /* Pick up the mask             */
           

           /*
            | If have a 'far' tile match, just need one more tile hit, but
            | this tile is already in hand, since we know that ACD_1 is
            | non-zero.
           */
           if (acd_0 & msk) return 1;

           
           /*
            | Check the other mask. If have a match then need only one
            | more struck tile to satify the veto. In this case one of
            | neighbors is in ACD_0 and one in in ACD_1. The neighbor
            | in ACD_0 has already been eliminated, so if it is non-zero
            | then, this is the third tile. The ACD_1 can only be checked
            | after eliminating its neighbor. If there is something
            | left, then the veto is satisfied, else, there is no way
            | it can be satisfied.
           */
           msk = map->msk_0[n][1];
           farVar = acd_1 & msk;
           if (farVar) 
           {
               /* Other mask hit, need a third tile */
               if (acd_0) return 1;
               n = FFS (farVar);
               return (acd_1 & ~MSK(n)) ? 1 : 0;
           }
           else
           {
               /*
                | Have one hit, Need one more hit + match
               */
               while (acd_0)                
               {
                   /* Check if have a far neighbor in ACD_0 */
                   n    = FFS (acd_0);    
                   msk  = map->msk_0[n][0];

                   if (acd_0 & msk) return 1;

                   /* No match in ACD_0, try ACD_1 */
                   acd_0 &= ~MSK(n);
                   msk    = map->msk_0[n][1];
                   if (acd_1 & msk) return 1;

               }

               /* No match ACD_0 with 0 or 1, the only hope is 1 and 1 */
               while (acd_1)
               {
                   n      = FFS (acd_1);
                   msk    = map->msk_1[n][1];
                   acd_1 &= ~MSK(n);
                   
                   if (acd_1 & msk) return 1;
               }

               /* No matches... */
           }
       }
       else
       {
           /* Only have hits in acd_0 */
           if (evaluate1 (acd_0, map->msk_0, 0)) return 1;
       }
       
   }
   else if (acd_1)
   {
       /* Only have hits in acd_1 */
       if (evaluate1 (acd_1, map->msk_1, 1)) return 1;
   }
   
   return 0;
}
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------------
   
   This is pictorially what the splash map tiles of the ACD look like
   
               +---+---+---+---+---+
               |   |   |   |   |   |
               | 35| 36| 37| 38| 39|
               |   |   |   |   |   |
         YP    +---+---+---+---+---+   YP
               |   |   |   |   |   |
               | 30| 31| 32| 33| 34|
               |   |   |   |   |   |
               +---+---+---+---+---+


   +---+---+   +---+---+---+---+---+   +---+---+
   |   |   |   |   |   |   |   |   |   |   |   |
   |  9|  4|   | 20| 21| 22| 23| 24|   | 14| 19|
   |   |   |   |   |   |   |   |   |   |   |   |
   +---+---+   +---+---+---+---+---+   +---+---+
   |   |   |   |   |   |   |   |   |   |   |   |
   |  8|  3|   | 15| 16| 17| 18| 19|   | 13| 18|
   |   |   |   |   |   |   |   |   |   |   |   |
XM +---+---+   +---+---+---+---+---+   +---+---+   XP
   |   |   |   |   |   |   |   |   |   |   |   |
   |  7|  2|   | 10| 11| 12| 13| 14|   | 12| 17|
   |   |   |   |   |   |   |   |   |   |   |   |
   +---+---+   +---+---+---+---+---+   +---+---+
   |   |   |   |   |   |   |   |   |   |   |   |
   |  6|  1|   |  5|  6|  7|  8|  9|   | 11| 16|
   |   |   |   |   |   |   |   |   |   |   |   |
   +---+---+   +---+---+---+---+---+   +---+---+
   |   |   |   |   |   |   |   |   |   |   |   |
   |  5|  0|   |  0|  1|  2|  3|  4|   | 10| 15|
   |   |   |   |   |   |   |   |   |   |   |   |
   +---+---+   +---+---+---+---+---+   +---+---+
                                     
               +---+---+---+---+---+
               |   |   |   |   |   |
               |  0|  1|  2|  3|  4|
               |   |   |   |   |   |
      YM       +---+---+---+---+---+    YP
               |   |   |   |   |   |
               |  5|  6|  7|  8|  9|
               |   |   |   |   |   |
               +---+---+---+---+---+


   This is the layout of the two ACD words containing only the splash tiles.
   
   1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
   f e d c b a 9 8 7 6 5 4 3 2 1 0 f e d c b a 9 8 7 6 5 4 3 2 1 0
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
  |8|7|6|5|4|3|2|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |             |                                                 |
  |<---- YP --->|<--------------------- TOP --------------------->|
  |             |                                                 |
                

   3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2
   f e d c b a 9 8 7 6 5 4 3 2 1 0 f e d c b a 9 8 7 6 5 4 3 2 1 0
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
  |1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |   |                   |                   |                   |
  |YP |<------- YM ------>|<------- XP ------>|<------- XM ------>|
  |   |                   |                   |                   |

      
*/


  
