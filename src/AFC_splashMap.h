#ifndef AFC_SPLASHMAP_H
#define AFC_SPLASHMAP_H


/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  AFC_splashMap.h
   \brief Defines a Splash Veto Map data structure
   \author JJRussell - russell@slac.stanford.edu

                                                                          */
/* ---------------------------------------------------------------------- */


#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------- *//*!
 *
 *  \struct _AFC_splashMap
 *  \brief   The data structure defining a Splash Veto Map
 *   
 *   The splash map contains the definitions of what are considered the
 *   near and far neighbors of the 65 tiles (25 top, 4*10 from the sides)
 *   allowed to participate in the splash veto evaluation.
 *
 *   The are three primary pieces, one piece giving this information for
 *   the XM:XP,YM:YP[0:1] word (this is 32 bits), one piece giving this
 *   information for the YP[8-2]:TOP word, and one piece giving this
 *   information for YP9. The YP9 word is unfortunate, but there are 65
 *   bits...
 *
 *   For each struck bit in the two words, one indexes the proper array,
 *   which gives to masks to project out the near and far neighbors.
 *
 * ---------------------------------------------------------------------- */


/*!
 *    
 *  \typedef AFC_splashMap
 *  \brief   Typedef for struct \e _AFC_splashMap
 *
 */
    
/* ---------------------------------------------------------------------- */
typedef struct _AFC_splashMap
{
    unsigned int msk_0[32][2];   /*!< Masks to use when considering bits
                                      from the YP[8-2]:TOP word           */
    unsigned int msk_1[32][2];   /*!< Masks to use when considering bits 
                                      from the XM:XP YM:YP[0:1] word      */
    unsigned int msk_64[2];      /*!< Masks to use when considering YP9,
                                      this is the 65th bit problem        */
}
AFC_splashMap;

/* ---------------------------------------------------------------------- */ 
 

#ifdef __cplusplus
}
#endif

#endif

 
