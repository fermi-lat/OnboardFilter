#ifndef ATF_SHADOWEDTOWERS_H
#define ATF_SHADOWEDTOWERS_H

/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   ATF_shadowedTowers.h
   \brief  Defines the interface needed to translate a list of tiles
           into a list of shadowed towers.
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */



#include "ATF_shadowedTowersCtl.h"
#include "ffs.h"

#ifdef __cplusplus
extern "C" {
#endif



/* ---------------------------------------------------------------------- */
static inline int ATF__shadowedTowersGet   (const unsigned short *twrsByTile,
                                            int                        tiles,
                                            int                     validCnt);
static inline int ATF__shadowedTowersByTop (const ATF_shadowedTowersCtl *ctl,
                                            int                        tiles);
static inline int ATF__shadowedTowersByXM  (const ATF_shadowedTowersCtl *ctl,
                                            int                        tiles);
static inline int ATF__shadowedTowersByXP  (const ATF_shadowedTowersCtl *ctl,
                                            int                        tiles);
static inline int ATF__shadowedTowersByYM  (const ATF_shadowedTowersCtl *ctl,
                                            int                        tiles);
static inline int ATF__shadowedTowersByYP  (const ATF_shadowedTowersCtl *ctl,
                                            int                        tiles);
/* ---------------------------------------------------------------------- */
    

    
/* ---------------------------------------------------------------------- *//*!

  \def   ATF_K_FACE_SIDE_CNT
  \brief MSB number of an ACD side face -- internal use only
                                                                          *//*!
  \def   ATF_K_FACE_TOP_CNT
  \brief MSB number of an ACD top  face -- internal use only
                                                                          */
/* ---------------------------------------------------------------------- */
#define ATF_K_FACE_SIDE_CNT 15
#define ATF_K_FACE_TOP_CNT  25    

//static inline int get_side_acd_mask (int tower, int start, int length);


/* ---------------------------------------------------------------------- *//*!

  \fn           int ATF__shadowedTowersGet (const unsigned short *twrs,
                                            int                  tiles,
                                            int               validCnt)
  \brief            Constructs a bit list of towers to which are
                    shadowed by the specified list of tiles.
  \param twrsByTile Table containing arrays which will translate ATF
                    tile numbers into a into a bit list of shadowing towers.
  \param      tiles A bit mask of the struck tiles in the face being
                    considered.
  \param   validCnt Number of valid bits in the \a tiles bit array.
  \return           The bit list of towers shadowed by the struck tiles.

   This routine is not part of the public interface. The user should
   call either ATF__shawdowedTowersSideGet or ATF__shadowedTowersTopGet.
                                                                          */
/* ---------------------------------------------------------------------- */
static int ATF__shadowedTowersGet (const unsigned short *twrsByTile,
                                   int                        tiles,
                                   int                     validCnt)
{
   /*
    | The map is packed with COL 0 in the MSB, so will left
    | justify the map. That is the highest number tile maps
    | to entry 0.
   */
   int towers;
   int   tile;


   towers      = 0;

   
   /* Check if have any struck tiles */   
   if (tiles != 0)
   {
       /* Corrects the table addressing of the table to match FFS */
       
       twrsByTile -= (32 - validCnt);
       /*
        | Find the struck tiles and use this to index a table mapping
        | the struck tile into a list of candidate towers. The index
        | to the table is actually such that the tile number is 24-idx.
        | This is strictly because the 'ffs' routine finds the first
        | set bit starting at the most significant bit.
       */
       do
       {
           tile    = FFS (tiles);
           towers |= twrsByTile[tile];
       }
       while (tiles &= ~(0x80000000 >> (tile)));
   }
   
   return towers;
   
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn      int ATF__shadowedTowersByTop (const ATF_shadowedTowersCtl *ctl,
                                         int                        tiles)
  \brief       Constructs a bit list of towers to which are shadowed
               by the specified list of top ACD tiles.
  \param   ctl Control structure containing the array which will
               translate top ACD tile numbers into a into a bit list of
               shadowing towers.
  \param tiles A bit mask of the struck TOP tiles.
  \return      The bit list of towers shadowed by the struck tiles.
                        
                                                                          */
/* ---------------------------------------------------------------------- */
static int ATF__shadowedTowersByTop (const ATF_shadowedTowersCtl *ctl,
                                     int                        tiles)
{
   return ATF__shadowedTowersGet (ctl->top, tiles, ATF_K_FACE_TOP_CNT);
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn    int ATF__shadowedTowersByXM (const ATF_shadowedTowersCtl *ctl,
                                      int                        tiles)
  \brief       Constructs a bit list of towers to which are shadowed
               by the specified list of side X- ACD tiles.
  \param   ctl Control structure containing the array which will
               translate X- ACD tile numbers into a into a bit list of
               shadowing towers.

 \param tiles A bit mask of the X- struck tiles.
  \return      The bit list of towers shadowed by the struck tiles.
                                                                          */
/* ---------------------------------------------------------------------- */
static int ATF__shadowedTowersByXM (const ATF_shadowedTowersCtl *ctl,
                                    int                        tiles)
{
   return ATF__shadowedTowersGet (ctl->x, tiles, ATF_K_FACE_SIDE_CNT);
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn    int ATF__shadowedTowersByXP (const ATF_shadowedTowersCtl *ctl,
                                      int                        tiles)
  \brief       Constructs a bit list of towers to which are shadowed
               by the specified list of side X+ ACD tiles.
  \param   ctl Control structure containing the array which will
               translate X+ ACD tile numbers into a into a bit list of
               shadowing towers.               
  \param tiles A bit mask of the X+ struck tiles.
  \return      The bit list of towers shadowed by the struck tiles.
                                                                          */
/* ---------------------------------------------------------------------- */
static int ATF__shadowedTowersByXP (const ATF_shadowedTowersCtl *ctl,
                                    int                       tiles)
{
   /*
    | The tables are constructed so that they yield the right answer when
    | considering the X- face. The tower pattern is the same for the X+
    | face if one just slides the towers over by 3. This is a right shift
    | because the MSB corresponds to tile 0, ie the numbering is big
    | endian.
   */
   return ATF__shadowedTowersGet (ctl->x, tiles, ATF_K_FACE_SIDE_CNT) >> 3;
}
/* ---------------------------------------------------------------------- */








/* ---------------------------------------------------------------------- *//*!

  \fn    int ATF__shadowedTowersByYM (const ATF_shadowedTowersCtl *ctl,
                                      int                       tiles)
  \brief       Constructs a bit list of towers to which are shadowed
               by the specified list of side Y- ACD tiles.
  \param   ctl Control structure containing the array which will
               translate Y- ACD tile numbers into a into a bit list of
               shadowing towers.
  \param tiles A bit mask of the Y- struck tiles.
  \return      The bit list of towers shadowed by the struck tiles.
                                                                          */
/* ---------------------------------------------------------------------- */
static int ATF__shadowedTowersByYM (const ATF_shadowedTowersCtl *ctl,
                                    int                       tiles)
{
 return ATF__shadowedTowersGet (ctl->y, tiles, ATF_K_FACE_SIDE_CNT);
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn    int ATF__shadowedTowersByYP (const ATF_shadowedTowersCtl *ctl,
                                      int                        tiles)
  \brief       Constructs a bit list of towers to which are shadowed
               by the specified list of side Y+ ACD tiles.
  \param   ctl Control structure containing the array which will
               translate Y+ ACD tile numbers into a into a bit list of
               shadowing towers.               
  \param tiles A bit mask of the Y+ struck tiles.
  \return      The bit list of towers shadowed by the struck tiles.
                                                                          */
/* ---------------------------------------------------------------------- */
static int ATF__shadowedTowersByYP (const ATF_shadowedTowersCtl *ctl,
                                    int                       tiles)
{
   /*
    | The tables are constructed so that the yield the right answer when
    | considering the Y- face. The tower pattern is the same for the Y+
    | face if one just slides the towers over by 12. This is a right shift
    | because the MSB corresponds to tile 0, ie the numbering is big
    | endian.
   */
   return ATF__shadowedTowersGet (ctl->y, tiles, ATF_K_FACE_SIDE_CNT) >> 12;
}
/* ---------------------------------------------------------------------- */


#undef ATF_K_FACE_SIDE_CNT 
#undef ATF_K_FACE_TOP_CNT  



#ifdef __cplusplus
}
#endif



#endif
