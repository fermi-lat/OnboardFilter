#ifndef AFC_SPLASHMAP_H
#define AFC_SPLASHMAP_H


/* ---------------------------------------------------------------------- *//*!
   
   \file  AFC_splashMap.h
   \brief Defines a Splash Veto Map data structure
   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $Id
\endverbatim
                                                                          */
/* ---------------------------------------------------------------------- */


#ifdef __cplusplus
extern "C" {
#endif


/* ---------------------------------------------------------------------- *//*!
  
    \struct _AFC_splashMap
    \brief   The data structure defining a Splash Veto Map
     
     The splash map contains the definitions of what are considered the
     near and far neighbors of the 65 tiles (25 top, 4*10 from the sides)
     allowed to participate in the splash veto evaluation.
  
     They are three primary pieces of information,

         -# Information from YP[8-2]:TOP word, (this is also 32 bit, 
            and 7 from YP[8-2] and 25 from the TOP word.
         -# Information for the XM:XP,YM:YP[0:1] word (this is 32 bits, 
            10 from each of XM, XP and YM and 2 from YP[0:1]).
         -# Information for YP9. The YP9 word is unfortunate, but there
            are 65 bits...
  
     For each struck bit in the two words, one indexes the proper array,
     which gives to masks to project out the near and far neighbors.
									  */
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!
       
   \typedef AFC_splashMap
     \brief   Typedef for struct \e _AFC_splashMap
  
									  */
/* ---------------------------------------------------------------------- */


    
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

 
