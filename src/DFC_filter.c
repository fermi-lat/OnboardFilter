/*------------------------------------------------------------------------
| CVS $Id: DFC_filter.c,v 1.10 2003/10/02 19:11:18 golpa Exp $
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  DFC_filter.c
   \brief Determines the fate of one event
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */



/* Facility public includes */
#include "DFC/DFC_filter.h"
#include "DFC/DFC_status.h"
#include "DFC/DFC_results.h"
#include "DFC/DFC_latRecord.h"



/* Pick up the Event Builder Data Format definitions and services */
#include "DFC/EBF_cid.h"
#include "DFC/EBF_hdr.h"
#include "DFC/EBF_acd.h"
#include "DFC/EBF_glt.h"
#include "DFC/EBF_tkr.h"
#include "DFC/EBF_locate.h"
#include "DFC/EBF_directory.h"
#include "DFC/EBF_directoryPrint.h"


/* Subsystem specific, public */
#include "DFC/CFC_towerSum.h"
#include "DFC/CFC_latEnergySum.h"
#include "DFC/CFC_latRecord.h"


#include "DFC/CFC_latUnpack.h"
#include "DFC/TFC_latRecord.h"
#include "DFC/TFC_latUnpack.h"
#include "DFC/TFC_triggerFill.h"
#include "DFC/TFC_projectionTowerFind.h"
#include "DFC/TFC_projectionPrint.h"


/* Configuration control includes */
#include "DFC_ctlDef.h"
#include "DFC_resultsDef.h"


/* Facility private includes */
#include "AFC_splash.h"
#include "ATF_filter.h"
#include "ATF_shadowedTowers.h"
#include "ATF_shadowedTowersCtl.h"
#include "CFC_ratioCheck.h"
#include "TFC_triggerForm.h"
#include "TFC_triggerSortedForm.h"
#include "TFC_geometryDef.h"
#include "TFC_projectionDef.h"
#include "TFC_acd.h"
#include "TFC_skirt.h"

/* ---------------------------------------------------------------------- *//*!

  \def    _DBG(statement)
  \brief  Simple macro to drop the specified statement into the code. This
          macro will only drop the statement if DEBUG is defined, otherwise
          this macro is a NOOP.
                                                                          */
/* ---------------------------------------------------------------------- */
#ifdef DEBUG
# undef DEBUG
#endif

#ifdef DEBUG
#include <stdio.h>
#define _DBG(_statement) _statement
#else
#define _DBG(_statement)
#endif

/* ---------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

extern struct _TFC_geometry TFC_GeometryStd;
    

/* ---------------------------------------------------------------------- *//*!

  \def   AFC_M_SIDE_FILTER_TILES
  \brief Defines the ACD side filtering tiles to be the upper two rows.
         The lower two rows are not used in throttling once the energy
         is too high. This avoids self-vetoing due to back-splash.
                                                                         */
 #define AFC_M_SIDE_FILTER_TILES (((0x3ff) << 16) | (0x3ff))
/* ---------------------------------------------------------------------- */


    

/* ---------------------------------------------------------------------- */

static inline int isVetoed      (int                           status);

static inline int cntBits       (unsigned int                    word);
    
    
static inline int evaluateAcd   (int                             status,
                                 int                             energy,
                                 int                         threshold0,
                                 int                         threshold1,
                                 const struct _AFC_splashMap *splashMap,
                                 int                             acd_xy,
                                 int                             acd_xz,
                                 int                             acd_yz);


static inline int evaluateAtf   (const ATF_shadowedTowersCtl *acd_ctl,
                                 int                           energy,
                                 const EBF_contributor   *contributor,
                                 int                             tmsk,
                                 int                           acd_xy,
                                 int                           acd_xz,
                                 int                           acd_yz,
                                 int                      tkr_trg[16]);

static inline int evaluateCal1  (const EBF_directory               *dir,
                                 const CFC_latRecord               *clr);

static inline int classifyAcd   (int acd_xy, int acd_xz, int acd_yz);
    

static inline int nominateTowers(const EBF_directory              *dir);
    
    
static unsigned int latFilter  (TFC_latRecord  *tlr,
                                int          energy,
                                int          acd_xy,
                                int          acd_xz,
                                int          acd_yz);
    

    
/* ---------------------------------------------------------------------- */


    
#ifdef __cplusplus
}
#endif
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn            int isVetoed (int status)
  \brief         Checks whether any of the veto bits are up
                                                                          */
/* ---------------------------------------------------------------------- */
static int isVetoed (int status)
{
  return status & DFC_M_STATUS_VETOES;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn            int cntBits (unsigned int word)
  \brief         Counts the bits in the specified word
  \param  word   The word to count the bits in
  \return        The number of bits in the specified word.
                                                                          */
/* ---------------------------------------------------------------------- */
static int cntBits (unsigned int word)
{
   int count = 0;

   while (word)
   {
       int bit = FFS (word);

       count += 1;
       word  &= ~(0x80000000 >> bit);
   }

   return count;
}
/* ---------------------------------------------------------------------- */


   



/* ---------------------------------------------------------------------- *//*!

  \fn         int evaluateAtf (const ATF_shadowedTowersCtl *acd_ctl,
                               int                           energy,
                               const EBF_contributor   *contributor,
                               int                             tmsk,
                               int                           acd_xy,
                               int                           acd_xz,
                               int                           acd_yz,
                               int                      tkr_trg[16])
  \brief  Evaluates whether any tower with a track trigger is in coincidence
          with a shadowing ACD tile.
  \param acd_ctl Structure defining which towers are shadowed by a tile.
  \param contributor The array of tower contributors
  \param        tmsk Bit mask giving the towers with TKR hits (TWR 0 = MSB) 
  \param      acd_xy The bit mask of struck tiles in the     top face.
  \param      acd_xz The bit mask of struck tiles in the X+/X- side face.
  \param      acd_yz The bit mask of struck tiles in the Y+/Y- side face.
  \param     tkr_trg Filled with the trigger patterns for each tower.
                                                                          */  
/* ---------------------------------------------------------------------- */  
static int evaluateAtf (const ATF_shadowedTowersCtl *acd_ctl,
                        int                           energy,
                        const EBF_contributor   *contributor,
                        int                             tmsk,
                        int                           acd_xy,
                        int                           acd_xz,
                        int                           acd_yz,
                        int                      tkr_trg[16])
{
   int              status;
   int               tower;
   int              dtower;
   int              acd_xm;
   int              acd_xp;
   int              acd_ym;
   int              acd_yp;
   int          candidates;
   int      top_candidates;
   int     side_candidates;
   int       xm_candidates;
   int       xp_candidates;
   int       ym_candidates;
   int       yp_candidates;

   
   /*!!! EXPERIMENTAL !!! */
   //status = evaluateZbottom (&dir, energy);
   //if (isVetoed (status) return status | DFC_M_STATUS_ATF;

       
   if (energy > CFC_MEV_TO_LEU(5000)) return DFC_M_STATUS_ATF;
   
   /*
    | This section evaluates the ACD information. The 5 masks indicating
    | the set bits in each of the 5 faces are extracted. To avoid problems
    | with self-vetoing, the lower 2 rows of side facing tiles are ignored. 
   */
   acd_xm = ((unsigned int)acd_xz >>  0) & AFC_M_VETO_SIDE_TILES;
   acd_xp = ((unsigned int)acd_xz >> 16) & AFC_M_VETO_SIDE_TILES;
   acd_ym = ((unsigned int)acd_yz >>  0) & AFC_M_VETO_SIDE_TILES;
   acd_yp = ((unsigned int)acd_yz >> 16) & AFC_M_VETO_SIDE_TILES;
   
   
   /*
    | Evaluate which towers should be examined. This is done by
    | assuming that each ACD tile shadows some set of tiles. It is not
    | expected that this shadowing is all-inclusive, only that ~80-90%
    | of the tracks passing through one a given tile will also pass
    | through the shadowed tower. This  means that vetoes based on this
    | information will only be 80-90% effective. The 10-20% of events
    | that sneak by will have to be caught be a more refined filter.
    | The idea here is to do something that is computationally quick but
    | not necessarily completely accurate. When the algorithm fails, it
    | fails on the conservative side, ie the event is, in some sense,
    | incorrectly passed to the next stage, where it will be rejected
    | using more refined but computationally more expensive information.
   */
   top_candidates  = ATF__shadowedTowersByTop (acd_ctl, acd_xy);
   xm_candidates   = ATF__shadowedTowersByXM  (acd_ctl, acd_xm);
   xp_candidates   = ATF__shadowedTowersByXP  (acd_ctl, acd_xp);
   ym_candidates   = ATF__shadowedTowersByYM  (acd_ctl, acd_ym);
   yp_candidates   = ATF__shadowedTowersByYP  (acd_ctl, acd_yp);
   side_candidates = xm_candidates | xp_candidates
                   | ym_candidates | yp_candidates;
   
   
   /*
    | Loop over the towers. The search is limited to only those
    | candidate towers that have data. The loop is also terminated
    | when the first tower with a veto is encountered.
   */
   candidates   = top_candidates | side_candidates;
   candidates <<= 16;
   candidates  &= tmsk;
   status       = DFC_M_STATUS_ATF;

   
   /* Anything to do ? */
   if (candidates == 0) return status;

   
   /* Find the first candidate tower */
   tower         = FFS (candidates);
   contributor  += tower;
   dtower        = tower + 1;

   
   while (1)
   {
        int        xcapture;
        int        ycapture;
        const EBF_tkr  *tkr;           
        TFC_trigger trigger;
        int            type;

        
        /* Remove the current tower from the candidates, find TKR data */
        candidates <<= dtower;
        tkr          = EBF__tkrLocate (contributor->ptr,
                                       contributor->calcnt);
        
        /* Get the bit masks representing the struck layers, form trigger */
        EBF_TKR_ACCEPTS_RIGHT_ORED_UNPACK (tkr->accepts, xcapture, ycapture);
        trigger.si     = TFC__triggerForm (xcapture, ycapture);
        
//      tkr_trg[tower] = xcapture & ycapture;
        type           = trigger.bf.type;
           
        
        /* Check if have a legitimate trigger pattern */
        if (trigger.si < 0)
        {
            int  tower_mask = (0x8000 >> tower);
            int       start = trigger.bf.start;

            /* Indicate that we have a trigger for this event */
            status |= DFC_M_STATUS_TKR_TRIGGER;


            /* Evaluate possible veto from the top tiles */
            if ((top_candidates & tower_mask) && start <= 2)
            {
                if (ATF__filterTop (tower, start, acd_xy)) 
                {
                    status |= DFC_M_STATUS_TOP;
                    break;
                }
            }

               
            /* Evaluate possible veto from the side tile */
            if (side_candidates & tower_mask)
            {
                if (ATF__filterSides (tower,
                                      tower_mask,
                                      start,
                                      trigger.bf.length, 
                                      xm_candidates, acd_xm,
                                      xp_candidates, acd_xp,
                                      ym_candidates, acd_ym,
                                      yp_candidates, acd_yp))
                {
                    status |= DFC_M_STATUS_SIDE;
                    break;
                }
            }
        }
        else if (type == TFC_K_TRIGGER_TYPE_4_OF_4)
        {
            /* This says there is some (remote) possibitity of a track */
            status |= DFC_M_STATUS_TKR_POSSIBLE;
        }

        
        /* Anymore towers */
        if (candidates == 0) break;


        /* There are more candidate towers to examine, get the next one */
        dtower       = FFS (candidates) + 1;
        tower       += dtower;
        contributor += dtower;
   }

   
   return status;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!
  \fn          int  evaluateAcd (int                             status,
                                 int                             energy,
                                 int                         threshold0,
                                 int                         threshold1,
                                 const struct _AFC_splashMap *splashMap,
                                 int                             acd_xy,
                                 int                             acd_xz,
                                 int                             acd_yz)




  \brief            Rejects events based on CAL energy and ACD info
  \param status     The current status of this event.
  \param energy     The total energy deposited in the CAL.
  \param threshold0 The threshold, in LEUs, which determines whether
                    the CAL has significant energy. This is used for
                    the cut which considers all tiles
  \param threshold1 The threshold, in LEUs, which determines whether
                    one can cut on only the upper 2 rows of the side
                    faces and the top face.
  \param splashMap  The ACD splash map to use when evaluting the splash veto
  \param acd_xy     The ACD TOP face tiles that are struck.
  \param acd_xz     The ACD X   face tiles that are struck.
  \param acd_yz     The ACD Y  face tiles that are struck.  
  
  \returns          The status, possibly augmented with the status
                    indicating no energy in the CAL and a struck
                    tile veto.

   Events that have no significant energy deposited in the CAL must
   be completely quiet in the ACD.
                                                                          */
/* ---------------------------------------------------------------------- */
static int  evaluateAcd (int                             status,
                         int                             energy,
                         int                         threshold0,
                         int                         threshold1,
                         const struct _AFC_splashMap *splashMap,
                         int                             acd_xy,
                         int                             acd_xz,
                         int                             acd_yz)
{

 
   /* Check if have any hits in the ACD */
   if (status & (DFC_M_STATUS_ACD_TOP | DFC_M_STATUS_ACD_SIDE) )
   {
       /* If energy is below 0-tolerance level, reject if any hits */
       if (energy < threshold0) return status | DFC_M_STATUS_E0_TILE;//Veto 28

       
       /* If have any hits in the upper part of the ACD */
       if (status & (DFC_M_STATUS_ACD_TOP | DFC_M_STATUS_ACD_SIDE_FILTER))
       {
           /* If energy is below filter tile limit, reject if filter tiles */
           if (energy < threshold1)
           {
               return status | DFC_M_STATUS_E350_FILTER_TILE;
           }
       }
   }
   

   /*
    | !!!! KLUDGE !!!!
    | ----------------
    | If have less than 40Gev and more than 4 tiles reject
   */
   /*
   if (energy < CFC_MEV_TO_LEU(40000))
   {
       int count;

       count = cntBits (acd_xy) + cntBits (acd_xz) + cntBits (acd_yz);
       if (count >= 4)
       {
           //printf ("Reject Energy = %10d T:X:Y %8.8x+%8.8x+%8.8x = %4d\n",
           //        CFC_LEU_TO_MEV(energy),
           //        acd_xy,
           //        acd_xz,
           //        acd_yz,
           //        count);
           return status | DFC_M_STATUS_SPLASH_1;
       }
       
       status |= AFC_splash (splashMap, acd_xy, acd_xz, acd_yz)
                 << DFC_V_STATUS_SPLASH_1;
   }
   */
   
   return status;
}
/* ---------------------------------------------------------------------- */
   


/* ---------------------------------------------------------------------- *//*!

  \def    _MOVE_BIT(_w, _from, _to)
  \brief  Moves bit from one location to another
  \param  _w    The word to move the bit from
  \param  _from The bit position (LSB = 0) to extract the bit from
  \param  _to   The bit position (LSB = 0) to move the bit to
  \return       A new 32-bit word with the specified bit set to the
                value of the \a _from bit in the original word.

                                                                          */
/* ---------------------------------------------------------------------- */  
#define _MOVE_BIT(_w, _from, _to) (((_w >> _from) & 1) << _to)
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \def    MASK0_4_OF_6
  \brief  A bit mask to be ordered with the dynamic 4/6 coincidence mask.
          Only patterns with 4/6 layers set are selected by this mask.
                                                                          *//*!
  \def    MASK1_4_OF_6
  \brief  A bit mask to be ordered with the dynamic 4/6 coincidence mask.
          Only patterns with 4/6 layers set are selected by this mask.

  The bits representing the last 6 layers are extracted from the X/Y
  layer hit masks and placed in the low 6 bits or a new word. This word
  can take on the values 0x00 to 0x3f (0-63).

  If the value of this word is less than 32, then MASK0_4_OF_6 is used.
  That is, of the low 5 bits, 4 must be set.

  If the value of this word is greater than or equal to 32, then
  MASK1_4_OF_6 is used. That is, of the remaining low 5 bits, 3 must
  be set.
                                                                          */
/* ---------------------------------------------------------------------- */  
#define MASK0_4_OF_6                                      \
 ((0 << 0x00) | (0 << 0x01) | (0 << 0x02) | (0 << 0x03) | \
  (0 << 0x04) | (0 << 0x05) | (0 << 0x06) | (0 << 0x07) | \
  (0 << 0x08) | (0 << 0x09) | (0 << 0x0a) | (0 << 0x0b) | \
  (0 << 0x0c) | (0 << 0x0d) | (0 << 0x0e) | (1 << 0x0f) | \
  (0 << 0x10) | (0 << 0x11) | (0 << 0x12) | (0 << 0x13) | \
  (0 << 0x14) | (0 << 0x15) | (0 << 0x16) | (1 << 0x17) | \
  (0 << 0x18) | (0 << 0x19) | (0 << 0x1a) | (1 << 0x1b) | \
  (0 << 0x1c) | (1 << 0x1d) | (1 << 0x1e) | (1 << 0x1f))

#define MASK1_4_OF_6                                      \
 ((0 << 0x00) | (0 << 0x01) | (0 << 0x02) | (0 << 0x03) | \
  (0 << 0x04) | (0 << 0x05) | (0 << 0x06) | (1 << 0x07) | \
  (0 << 0x08) | (0 << 0x09) | (0 << 0x0a) | (1 << 0x0b) | \
  (0 << 0x0c) | (1 << 0x0d) | (1 << 0x0e) | (1 << 0x0f) | \
  (0 << 0x10) | (0 << 0x11) | (0 << 0x12) | (1 << 0x13) | \
  (0 << 0x14) | (1 << 0x15) | (1 << 0x16) | (1 << 0x17) | \
  (0 << 0x18) | (1 << 0x19) | (1 << 0x1a) | (1 << 0x1b) | \
  (1 << 0x1c) | (1 << 0x1d) | (1 << 0x1e) | (1 << 0x1f))
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn       int evaluateZbottom (const EBF_directory *dir, int energy)
  \brief        Makes a quick test to see if there is energy in the CAL
                 without the possibility of a track pointing into it.
  \param    dir The directory of event contributors
  \param energy The energy in the calorimeter
  \retval     0 If there is no energy in the calorimeter or there
                is a possibility of a track pointing at the energy
  \retval DFC_M_STATUS_ZBOTTOM If there is energy in the calorimeter
                               and there is no possibility of a track
                               pointing at it.

  The check is deliberately crude so that it is extremely cheap. The
  bit masks of the hit layers for each tower are examined to see if
  there is even the remotest possibility of extending a track into the
  calorimeter. Currently 4 or the last 6 planes must have hits in them.
                                                                          */
/* ---------------------------------------------------------------------- */  
static int evaluateZbottom (const EBF_directory *dir, int energy)
{
   const EBF_contributor *contributors;
   int                            cids;
   int                             cid;
   int                           tcids;
   int                              m0;
   int                              m1;
   
   //the next line eliminates this veto completely!
   //return 0;
   //done eliminating!
   if (energy > CFC_MEV_TO_LEU (100)) {//ALTERED Veto 21 - remove line to change back
   if (energy < CFC_MEV_TO_LEU (10)) return 0;   

   tcids        = 0;
   cids         = EBF_DIRECTORY_TWRS_TKR (dir->ctids);
   contributors = dir->contributors;
   m0           = MASK0_4_OF_6;
   m1           = MASK1_4_OF_6;

   
   /* Loop over all towers that may have tracker data */
   for (cid = EBF_K_CID_TWR; cid < EBF_K_CID_TWR + 16; cid++)
   {
       
       /* If this tower had data... */
       if (cids & (0x80000000>>cid))
       {
           const EBF_contributor *contributor;
           const EBF_tkr                 *tkr;
           unsigned int              xaccepts;
           unsigned int              yaccepts;
           int                           mask;           
           
           contributor = &contributors[cid];
           tkr         = EBF__tkrLocate (contributor->ptr,
                                         contributor->calcnt);

           EBF_TKR_ACCEPTS_RIGHT_ORED_UNPACK (tkr->accepts, xaccepts,yaccepts);

           /*
            | Want to check the coincidence level of the 6 planes nearest
            | the CAL, so will form a bit mask of these planes. The six
            | relevant bits are moved from the right justified accept mask
            | to the low 6 bits of a variable.
           */
           mask = _MOVE_BIT(yaccepts, EBF_TKR_K_RIGHT_BIT_L0, 0)
                | _MOVE_BIT(xaccepts, EBF_TKR_K_RIGHT_BIT_L0, 1)
                | _MOVE_BIT(yaccepts, EBF_TKR_K_RIGHT_BIT_L1, 2)
                | _MOVE_BIT(xaccepts, EBF_TKR_K_RIGHT_BIT_L1, 3)
                | _MOVE_BIT(yaccepts, EBF_TKR_K_RIGHT_BIT_L2, 4)
                | _MOVE_BIT(xaccepts, EBF_TKR_K_RIGHT_BIT_L2, 5);

           //printf ("Twr %1.1x /Y accepts = %8.8x:%8.8x => %8.8x %8.8x\n",
           //        cid,
           //        xaccepts,
           //        yaccepts,
           //        mask,
           //        (mask < 32) ? (m0 & (1 << mask)) : (m1 & (1<< (mask-32))));

           /*
            | The mo and m1 variables have bits set in the positions that
            | have allowed coincidences. For example, if mask has the value
            | of 31, then if bit position 31 of m0 is set, then there
            | are enough layers in the lower part of the TKR to be considered
            | adjacent to the CAL.
            |
            | Since there are only 32 bits in a 32-bit integer (dah..),
            | to variables are used, one when the value of the mask is
            | less than 32 and one when it is greater than or equal to 32.
            | In the latter case, the mask is adjusted downwards by
            | subtracting 32.
           */
           if (mask < 32)
           {
               if (m0 & (1 <<  mask   ))  { return 0; }
           }
           else
           {
               if (m1 & (1 << (mask-32))) { return 0; }
           }
           
       }
   }
   }             //ALTERED Veto 21 - remove line to change back
   else{         //ALTERED - remove line to change back
	   return 0; //ALTERED - remove line to change back
   }             //ALTERED - remove line to change back

   return DFC_M_STATUS_ZBOTTOM;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn            int evaluateCal1 (const EBF_directory               *dir,
                                   const               CFC_latRecord *clr)

  \brief           Does the second level of CAL filtering.
  \param dir       The directory of event contributors.
  \param clr       Pointer to a previously filled CAL LAT record.
  \return          A status bit mask to be added to the event status word.
                                                                          */
/* ---------------------------------------------------------------------- */
static int evaluateCal1 (const EBF_directory               *dir,
                         const CFC_latRecord               *clr)
{
   int energy;
   int status;
   

   energy = clr->energy;
   status = 0;

   
   
   
   /*
    | If have some energy,
    | Then subject the event to 2 cuts.
    |    1. The ratio of the energy in layer 0 / etotal is 1%-90%.
    |    2. At least 20% of the logs must each have 1% of the energy.
    |
   */
   if (energy > 0)
   {

       if (energy < CFC_MEV_TO_LEU (300))
       {
             
          /*
           | Check that .01 < E0/ETOT < .90
           | ------------------------------
           | Check the ratio of energy in layer 0 to total is 1% - 90%
           | Care needs to be taken to avoid overflow/underflow.
           |
           | The energy in layer 0 will be scaled by multiplying by 1024 bits.
           | For a signed number this means that there must be at least 11 bits
           | free.
           |
           | If the total energy will also be multiplied by a number close
           | to 1024 (90%), so it to must have 11 free bits. It is assumed
           | that if the total energy has 11 free bits, the energy in layer
           | 0 also has 11 free bits.
           |
          */
           //status |= CFC__ratioCheck (clr->layerEnergy[0],   energy,
           //                           10,  DFC_M_STATUS_EL0_ETOT_01,
           //                           900, DFC_M_STATUS_EL0_ETOT_90);//REMOVED 22 & 23
           if (isVetoed (status))
           {
               _DBG (printf ("REJECT on ratio\n"));
#ifdef GLEAM
               if(passThrough==0)
#endif
               return status;
           }
       }
       
#      if 0         
       {
           int         eneed;
           int        logCnt;
           const int *logLst;
           const int *logCur;
           
       
           /*
            | To avoid divisions, 20% is approximated by 200/1024
            | and                  1% is approximated by 1/128.
            |
            | These are somewhat arbitrary numbers, so this is not deemed
            | to be significant.
           */
       
           /* Calculate the number of logs needed to exceed the 20% number */
           logCnt = clr->logCnt;
           eneed  = logCnt * 200 / 1024;

       
           /*
            | If there are only a few struck logs, then the number needed can
            | be 0 (20% of numbers < 5) yield 0). One does not really need
            | to check this because at least 1 log must exceed the 1%
            | threshold, so that the number still needed will be checked.
            | This is purely an optimization issue; which is better, checking
            | all the time in hopes of avoiding 1 trip through the loop or
            | just taking the hit on those and avoiding the additional cost
            | for all the others.
            |
            | Test indicate that omitting the test is slightly better.
           */
           // if (eneed == 0) return status;
       
           logCur = clr->logEnds;       
           logLst = logCur + (logCnt << 1);
       

           /*
            | The loop is unrolled by a factor of 2, so need to handle case
            | where an odd number of logs are struck. This is done by
            |
            |       1. Checking for this case
            |       2. If have an odd number, adjusting the starting point
            |       3. Jumping into the middle of the loop (this ought to
            |          was the code checkers, a goto and jumping into the
            |          middle of a loop.
            |
            | Tried unrolling by 4, but did not see an better performance.
           */
           if (logCnt & 3)
           {
               logCur -= 2;
               goto ODD;
           }
       
           do
           {
              int e;
           
              e = logCur[0] + logCur[1];
              if ((e*128 + 64) > energy && (--eneed <= 0)) return status;
           
             ODD:
              e = logCur[2] + logCur[3];
              if ((e*128 + 64) > energy && (--eneed <= 0)) return status;
           
           }
           while ((logCur += 4) < logLst);

           status |= DFC_M_STATUS_CAL_XTAL_RATIO;
       }
#      endif       
       
   }

   return status;
   
}
/* ---------------------------------------------------------------------- */



  
  

/* ---------------------------------------------------------------------- *//*!

  \fn     int classifyAcd (int acd_xy, int acd_xz, int acd_yz)
  \brief  Sets status bits indicating which part of the ACD has hits.
  \param acd_xy The struck tiles in the ACD XY (top) face.
  \param acd_xz The struck tiles in the ACD XZ face.
  \param acd_yz The struck tiles in the ACD YZ face.
  return        A status mask indicating those parts of the ACD which have
                hits.

  The ACD is considered to be 3 logical pieces. The top 25 tiles are
  consider one piece; all the rows of the side faces are another, and only
  the lower 2 rows of the side faces are the third. A  bit is set for each
  of these three areas which have a hit. Note that these are not mutually
  exclusive.
                                                                          */
/* ---------------------------------------------------------------------- */
static int classifyAcd (int acd_xy, int acd_xz, int acd_yz)
{
   int status;
   int acd_xz_yz; 
 

   status    = 0;
   acd_xz_yz = acd_xz | acd_yz;

   
   /* Mark if any tile is struck in the top face */
   if (acd_xy) status |= DFC_M_STATUS_ACD_TOP;

   
   /* Any side hits ? */
   if (acd_xz_yz)
   {
       status |= DFC_M_STATUS_ACD_SIDE;

       /* Any of the side hits in the two rows ? */
       if ((acd_xz_yz) & AFC_M_SIDE_FILTER_TILES)
       {
           status |= DFC_M_STATUS_ACD_SIDE_FILTER;
       }
   }

   
   return status;
}
/* ---------------------------------------------------------------------- */











/* ---------------------------------------------------------------------- *//*!

  \fn int nominateTowers (const EBF_directory *dir)
  \brief  Creates a list of towers with the potential for finding tracks in.
  \param  dir     The event directory
  \return         The upper 16 bits contain a mask of the towers with at
                  least a 6 fold coincidence, the lower 16 bits contain
                  a mask of the towers with a 4 fold coincidence. They
                  are exclusive, that is if the tower has a 6 fold
                  coincidence, the 4 fold coincidence is not also set.
                                                                          */
/* ---------------------------------------------------------------------- */
static int nominateTowers (const EBF_directory *dir)
{
   const EBF_contributor *contributors;
   int                            cids;
   int                             cid;
   int                           tcids;
   

   tcids        = 0;
   cids         = EBF_DIRECTORY_TWRS_TKR (dir->ctids);
   contributors = dir->contributors;
   
   
   /* Loop over all towers that may have tracker data */
   for (cid = EBF_K_CID_TWR; cid < EBF_K_CID_TWR + 16; cid++)
   {
       const EBF_tkr         *tkr;
       unsigned int      xcapture;
       unsigned int      ycapture;

       
       /* If this tower had data... */
       if (cids & (0x80000000>>cid))
       {
           int               xy00, xy11, xy22;
           const EBF_contributor *contributor;
           
           
           contributor = &contributors[cid];
           tkr         = EBF__tkrLocate (contributor->ptr,
                                         contributor->calcnt);
           

           EBF_TKR_ACCEPTS_RIGHT_ORED_UNPACK (tkr->accepts, xcapture,ycapture);

           xy00 = xcapture & ycapture;
           if (xy00)
           {
               /*
                | Need to form the coincidence between the odd and even
                | portions. Want to form a variable which has layer n
                | and layer n+1 adjacent.
                |
                |             10fedcba9876543210
                |   xy      = 13579bdfh02468aceg
                |   xy >> 9 = .........13579bdfh
                |   xy << 1 = ........02468aceg_
                |
                | Note that when shifting down by 8, must eliminate the '1'
                | which appears in bit position 0.
                |
                | If one ORs the xy and xy << 1 (ignoring the bits
                | past bit 9, each bit position will contain the OR of
                | layer n-1 and layer n+1 relative to the xy >> 9
                | variable. Thus ANDing this variable| against this ORd
                | variable will produce a coincidence of layer N and
                | either layer N-1 or layer N+1, exactly what is needed.
                |
                |
               */
               xy11 = xy00 >> 9;
               xy22 = xy00 << 1;
               /* Fix recommended by David to catch all towers */
               if       (xy00 & xy11  & xy22) tcids |= 0x80000000 >> cid;
               else if ((xy00 | xy22) & xy11) tcids |= 0x00008000 >> cid;
           }
       }
   }

   return tcids;
}
/* ---------------------------------------------------------------------- */



       

/* ---------------------------------------------------------------------- *//*!

  \fn     int DFC_filter (struct _DFC_ctl          *ctl,
                          struct _DFC_results  *results,  
                          struct _DFC_latRecord    *dlr,
                          const unsigned int       *evt,
                          int                      size)
  \brief  Determines the fate of 1 event
  \param  ctl      Control parameter block
  \param  results  Pointer to the results vector to fill in  
  \param  dlr      Pointer to a structure to hold an unpacked event
  \param  evt      Pointer to the event to consider
  \param size      The number of words in the event.
  \return          The filtering fate
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_filter (struct _DFC_ctl         *dfc,
                struct _DFC_results *results,
                struct _DFC_latRecord   *dlr,
                const unsigned int      *evt,
                int                     size)
{
   int               status;
   const EBF_glt       *glt;
   int               acd_xy;
   int               acd_xz;
   int               acd_yz;
   int          tkr_trg[16];
   int               energy;
   int              calHiLo;
   
   
   /* Note the starting time, this is used to time the code */   
   results->beg = TMR_GET();
   status       = 0;
   
   
  /*----------------------------------------------------------------------*\
   |                                                                      |
   |  CAL HI Trigger Set?                                                 |
   |  -------------------                                                 |
   |  Currently all events with the CAL HI trigger bit set are kept       |
   |                                                                      |
  \*----------------------------------------------------------------------*/
   glt = EBF__gltLocate (evt, size);
   {
       calHiLo = glt->calHiLo;
       if (calHiLo     & 0x0000ffff) status |= DFC_M_STATUS_CAL_LO;
       if (calHiLo     & 0xffff0000) status |= DFC_M_STATUS_CAL_HI;
       if (glt->thrTkr & 0xffff0000) status |= DFC_M_STATUS_TKR_THROTTLE;
       /*
           !!! KLUDGE !!!
           --------------
           The CAL HI pass is currently commented out
        
       {
          results->acd           = TMR_GET ();       
          return results->status = DFC_M_STATUS_CAL_HI;
       }
       */
   }

   
   
  /*----------------------------------------------------------------------*\
   |                                                                      |
   | ACD INFORMATION                                                      |
   | ---------------                                                      |
   | Locate the GLT block so can extract the veto hit map. The            |
   | ACD information content is then classified as a series of            |
   | bits in the status word.                                             |
   |                                                                      |
  \*----------------------------------------------------------------------*/
   acd_xz  = glt->acd.vetoes[EBF_K_GLT_ACD_VETO_XZ];
   acd_yz  = glt->acd.vetoes[EBF_K_GLT_ACD_VETO_YZ];
   acd_xy  = glt->acd.vetoes[EBF_K_GLT_ACD_VETO_XY];   
#ifdef GLEAM
   TDS_variables.acd_xz=acd_xz;
   TDS_variables.acd_yz=acd_yz;
   TDS_variables.acd_xy=acd_xy;
#endif
   status |= classifyAcd (acd_xy, acd_xz, acd_yz);


   /* If no CAL LO discriminators, little chance of backsplash */
   if ((status & DFC_M_STATUS_CAL_LO) == 0)
   {
       /* Any upper region tiles struck with little energy? */
       if (status & (DFC_M_STATUS_ACD_TOP | DFC_M_STATUS_ACD_SIDE_FILTER))
       {
           /* Little energy and struck tiles... */
           status = status | DFC_M_STATUS_NOCALLO_FILTER_TILE;
           if (isVetoed (status))
           {
               results->energy = -1;
               results->cal0   = TMR_GET();
#ifdef GLEAM
               if(passThrough==0)
#endif
               return results->status = status | DFC_M_STATUS_VETOED;
           }
       }
   }

   
   /* If no CAL HI discriminators, try splash veto */
   if ((status & DFC_M_STATUS_CAL_HI) == 0)
   {
       if (cntBits (acd_xy) + cntBits (acd_xz) + cntBits (acd_yz) >= 4)
           status |= DFC_M_STATUS_SPLASH_0;
       else
           status |= AFC_splash (&dfc->afc.splashMap, acd_xy, acd_xz, acd_yz)
                   << DFC_V_STATUS_SPLASH_0;
       
       if (isVetoed (status))
       {
           results->energy = -1;
           results->cal0   = TMR_GET();
#ifdef GLEAM
           if(passThrough==0)
#endif
           return results->status = status | DFC_M_STATUS_VETOED;
       }
   }
   
   results->cal0   = TMR_GET();
   
  /*----------------------------------------------------------------------*\
   |                                                                      |
   | CAL/ACD PHASE 0                                                      |
   | ---------------                                                      |
   | This checks that the level of activity in the ACD is consistent with |
   | the energy deposition in the CAL. In particular, if there is no      |
   | energy to just a small amount, the entire ACD must be quiet. If      |
   | there is less than 350 Mev, the top portion (top face and first 2    |
   | rows of the ACD must be quiet.                                       |
  \*---------------------------------------------------------------------*/
   {
       energy = CFC_latEnergySumEvt (evt  + 1,
                                     size - sizeof (*evt),
                                     dfc->cfc.constants,
                                     dlr->cal.layerEnergy);
       
       results->energy = energy;

       status = evaluateAcd (status,
                             energy,
                             CFC_MEV_TO_LEU(10),
                             CFC_MEV_TO_LEU(350),
                             &dfc->afc.splashMap,
                             acd_xy,
                             acd_xz,
                             acd_yz) | DFC_M_STATUS_ACD;
       if (isVetoed (status))
       {
           results->status = status |= DFC_M_STATUS_VETOED;
           results->acd    = TMR_GET ();
#ifdef GLEAM
           if(passThrough==0)
#endif
           return status;
       }

       results->acd = TMR_GET();
   }
   

   

  /*----------------------------------------------------------------------*\
   |                                                                      |
   | DIRECTORY FORMATION                                                  |
   | -------------------                                                  |
   | To do further analysis, it is more efficient to form a directory of  |
   | the contributors to this event. This directory allows the subsequent |
   | routines to easily locate these contributors and, in the case of the |
   | towers, the CAl and TKR sub-components. In addition, summary         |
   | information is added, such as bit masks indicating which major       |
   | components are present and which sub-components are present. This    |
   | allows the routines to quickly traverse all the CAL towers with data |
   | or all the TKR towers with data.                                     |
   |                                                                      |
   | It is important to note that this step only composes the directory   |
   | data structure, no events are rejected.                              |
  \*----------------------------------------------------------------------*/
   {
       EBF_directoryCompose (&dlr->dir, evt);
       status      |= DFC_M_STATUS_DIR;
       results->dir = TMR_GET ();
   }
   

   

  /*----------------------------------------------------------------------*\
   |                                                                      |
   | ACD/TKR COINCIDENCE EVALUATION                                       |
   | ------------------------------                                       |
   | A match is performed between the struck ACD tiles and the TKR. The   |
   | match is down crudely, using only the struck layers in the the TKR   |
   | to determine whether to examine a subset of the ACD tiles. Contrast  |
   | this with the full tracking method, where tracks are found and pro-  |
   | jected to the ACD face in search of a coincidence.                   |
   |                                                                      |
  \*----------------------------------------------------------------------*/
   {
       status |= evaluateAtf (&dfc->atf.shadowed,
                              energy,
                              dlr->dir.contributors + EBF_K_CID_TWR,
                              EBF_DIRECTORY_TWRS_TKR (dlr->dir.ctids),
                              acd_xy,
                              acd_xz,
                              acd_yz,
                              tkr_trg);
       if (isVetoed (status) ||
	   isVetoed (status |= evaluateZbottom (&dlr->dir, energy)))
       {
           results->status = status |= DFC_M_STATUS_VETOED;
           results->atf    = TMR_GET();
#ifdef GLEAM
           if(passThrough==0)
#endif
           return status;
       }

       
       
       results->atf = TMR_GET();
   }
   


   
  /*----------------------------------------------------------------------*\
   |                                                                      |
   | CAL/ACD EVALUATION, PHASE 2                                          |
   | ---------------------------                                          |
   | A more time consuming analysis of the CAL is now performed. This     |
   | computes the total energy of the event and its distribution through  |
   | the layers. Before doing this the CAL information is unpacked into   |
   | a standard data structure.                                           |
   |                                                                      | 
  \*----------------------------------------------------------------------*/
   {
       dlr->cal.energy = energy;
       
       //CFC_latUnpack (&dlr->cal, dfc->cfc.constants, &dlr->dir);
       status |= evaluateCal1 (&dlr->dir, &dlr->cal) |  DFC_M_STATUS_CAL1;

       if (isVetoed (status))
       {
           results->status = status |= DFC_M_STATUS_VETOED;
           results->cal1   = TMR_GET ();
#ifdef GLEAM
           if(passThrough==0)
#endif
           return status;
       }

       results->cal1 = TMR_GET ();       
   }

   


  /*----------------------------------------------------------------------*\
   |                                                                      |
   | TKR, PATTERN RECOGNITION                                             |
   | ---------------------------                                          |
   | This looks for tracks using the strip addresses                      |
   |                                                                      |
  \*----------------------------------------------------------------------*/
   {
       int tcids;

       tcids = nominateTowers (&dlr->dir);
       

       if (tcids == 0)
       {
           if (energy > CFC_MEV_TO_LEU (250)) //ALTERED 17
		   status |= DFC_M_STATUS_TKR_EQ_0;
       }
       else    
       {
           /* Classify event either as a solid trigger or only a possibility */
           if (tcids & 0xffff0000) status |= DFC_M_STATUS_TKR_TRIGGER;
           else                    status |= DFC_M_STATUS_TKR_POSSIBLE;

           /* Only unpack those towers with any hope of a trigger */
           {
               unsigned int  cids;

               
               //printf ("< --------- New Event (%8d:%8d Mev) ---------- >\n",
               //        EBF_HDR_SEQ (glt->hdr),
               //        CFC_LEU_TO_MEV(calLatRec.energy));


               cids = tcids | (tcids << 16);
               TFC_latUnpackReset (&dlr->tkr);
               TFC_latUnpack      (&dlr->tkr, &dlr->dir, cids);
               
               
               //_DBG (TFC_latRecordPrint(&dlr->tkr, -1));
               
               status       |= latFilter (&dlr->tkr,
                                          energy,
                                          acd_xy,
                                          acd_xz,
                                          acd_yz) | DFC_M_STATUS_TKR;
               results->tkr  = TMR_GET ();
               
           }
           
           ///CFC_latRecordPrint (&calLatRec, -1);
           _DBG (
           {
               //int tkr_trg[16];
               unsigned int hdr1 = dlr->dir.contributors[0].ptr[1];
               unsigned int seq  =  ((hdr1 >> 29) & 3)
                                 | (((hdr1 >> 1) & 0x3fff) << 2);
               
               printf ("Sequence = %8.8x %8.8x\n", hdr1, seq);

               //TFC_triggerFill (&dlr->dir, tkr_trg);
               //DFC_displayAcdTkr (0, tkr_trg, acd_xy, acd_xz, acd_yz);
           })
           
       }

   }
   
   if(isVetoed (status)){
     status |= DFC_M_STATUS_VETOED;
   } 
   return results->status = status;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn int DFC_filterComplete (struct _DFC_ctl         *ctl, 
                              struct _DFC_results *results,
                              struct _DFC_latRecord   *dlr,
                              const unsigned int      *evt,
                              int                     size)
  \brief  Complete the results vector.
  \param  results  The results vector for the event
  \param  evt      The event
  \param  size     The size of the event in 32-bit words

  The DFC_filter routine completes only that portion of the results vector
  needed to reject the event. It does not calculate quantities that it
  does need. This routine completes those quantities so that the statistics
  derived from the results vector are consistent.
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_filterComplete (struct _DFC_ctl         *ctl,
                        struct _DFC_results *results,
                        struct _DFC_latRecord   *dlr,
                        const unsigned int      *evt,
                        int                     size)
{
    int status;
    int    twr;
    EBF_directory     *dir = &dlr->dir;
    const EBF_contributor *contributor;

    
    /* Add the event number */
    results->evtNum = EBF_HDR_SEQ(evt[1]);



    /* If the energy of this event is was not computed */
    if (results->energy == -1)
    {
        results->energy = CFC_latEnergySumEvt (evt  + 1,
                                               size - sizeof (*evt),
                                               ctl->cfc.constants,
                                               dlr->cal.layerEnergy);
    }

    
    EBF_directoryCompose (dir, evt);

    
    /* Complete the size information */
    results->sizes.evt = size;    
    results->sizes.glt = dir->contributors[EBF_K_CID_GLT].len;
    results->sizes.acd = dir->contributors[EBF_K_CID_ACD].len;
    contributor        = dir->contributors + EBF_K_CID_TWR;
    
    
    for (twr = 0; twr < 16; twr++, contributor++)
    {
        int    size =  contributor->len;
        int calSize = (contributor->calcnt + 1) * 4;        
        
        results->sizes.tem[twr].tem = size;
        results->sizes.tem[twr].cal = calSize;
        results->sizes.tem[twr].tkr = size - calSize;        
    }
        
    
    
    
    /* If the tracker not done, do it */
    status  = results->status;    
    if ((status & DFC_M_STATUS_TKR) == 0)
    {
        int      trgs[16];
        int         tcids;
        
        
        tcids = TFC_triggerFill  (dir, trgs);
#ifdef GLEAM
	TDS_variables.tcids=tcids;
#endif
        //dir.contributors[EBF_K_CID_GLT].ptr);
        
        
        /* Classify event either as a solid trigger or only a possibility */
        if      (tcids & 0xffff0000) status |= DFC_M_STATUS_TKR_TRIGGER;
        else if (tcids & 0x0000ffff) status |= DFC_M_STATUS_TKR_POSSIBLE;
    }
    
    results->status = status;
    
    return status;
}
/* ---------------------------------------------------------------------- */



#if ACD_GLT_COMPARE
static void checkAcdInfo (const unsigned int *evt,
                          int                size,
                          unsigned int     acd_xy,
                          unsigned int     acd_xm,
                          unsigned int     acd_xp,
                          unsigned int     acd_ym,
                          unsigned int     acd_yp)
{
   EBF_directory dir;
   EBF_acd      *acd;
   unsigned int   xy;
   unsigned int   xm;
   unsigned int   xp;
   unsigned int   ym;
   unsigned int   yp;

   
   EBF_directoryCompose (&dir, evt, size);
   acd = (EBF_acd *)dir.contributors[EBF_K_CID_ACD].ptr;
   EBF_ACD_VETOES_ORED_UNPACK (acd->vetoes, top, xm, xp, ym, yp);

   
   /* Check if any of the faces do not match */    
   if (xy != acd_xy  ||
       xm != acd_xm  ||
       xp != acd_xp  ||
       ym != acd_ym  ||
       yp != acd_yp)
   {
       printf(
            "     Top       X-       X+       Y-       Y+        X        Y\n"
           " -------- -------- -------- -------- -------- -------- --------\n"
           " %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x\n"
           " %8.8x %8.8x %8.8x %8.8x %8.8x\n",
            acd_xy, acd_xm, acd_xp, acd_ym, acd_yp, acd_xz, acd_yz,
                xy,     xm,     xp,     ym,     yp);

       printf ("BRD4:5 %8.8x %8.8x : %8.8x %8.8x %8.8x %8.8x\n",
               EBF_ACD_VETO_BRD4(acd->vetoes[2]),
               EBF_ACD_VETO_BRD5(acd->vetoes[2], acd->vetoes[3]),
               EBF_ACD_VETO_BRDA(acd->vetoes[5], acd->vetoes[6]),
               EBF_ACD_VETO_BRDB(acd->vetoes[6]),
               acd->vetoes[5], acd->vetoes[6]);
       
   }

   return;
   
}


#endif


/* ---------------------------------------------------------------------- *//*!

  \fn unsigned int latFilter  (TFC_latRecord *tlr,
                               int         energy,
                               int         acd_xy,
                               int         acd_xz,
                               int         acd_yz)
  \brief Performs the most computationally intensive portions of the
         filtering. This involves TKR pattern recognition and matching
         to the ACD, skirt region and CAL

  \param tlr    The unpacked tracker data for the entire LAT
  \param energy The total energy in the CAL
  \param acd_xy The bit pattern of struck tiles in the XY (top) ACD plane
  \param acd_xz The bit pattern of struct tiles in the side XZ+/XZ- planes
  \param acd_xz The bit pattern of struct tiles in the side YZ+/YZ- planes  
  \return       A summary status bit mask
                                                                          */
/* ---------------------------------------------------------------------- */
static unsigned int latFilter  (TFC_latRecord *tlr,
                                int         energy,
                                int         acd_xy,
                                int         acd_xz,
                                int         acd_yz)
{
   TFC_towerRecord *ttrs;
   TFC_projections  prjs;
   int              tmsk;
   int            status;
   int             xyCnt;
   int          dispatch;
   
   /* If nothing to do... */
   tmsk = tlr->twrMap;
   //if (tmsk == 0) return DFC_M_STATUS_TKR_EQ_0;//ALTERED 17
   if (tmsk == 0) {
      if (energy > CFC_MEV_TO_LEU (250)) return DFC_M_STATUS_TKR_EQ_0;
	  else 
      return status;
   }

   /*
    |  !!! KLUDGE !!!
    |  --------------
    |  This is too sloppy, need better way to initialize and keep the
    |  maximum number of projections under control
   */
   prjs.curCnt = 0;
   prjs.maxCnt = 1000;
   prjs.xy[0]  = 0;
   prjs.xy[1]  = 0;
   xyCnt       = 0;
   dispatch    = TFC_acdProjectTemplate (acd_xy, acd_xz, acd_yz);
   
   
   ttrs   = tlr->twr;
   status = 0;
   do
   {
       int            tower;
       TFC_towerRecord *ttr;
       int        tkrStatus;
       
       tower = FFS (tmsk);
       ttr   = ttrs + tower;
       tmsk &= ~(0x80000000 >> tower);

       
       /* Find the projections */
       tkrStatus = TFC_projectionTowerFind (&prjs,
                                         &TFC_GeometryStd.tkr,
                                         ttr,
                                         ttr->layerMaps[0],
                                         ttr->layerMaps[1]);
       //_DBG (TFC_projectionsPrint (&prjs, tower));
#ifdef GLEAM
       memcpy(&(TDS_variables.prjs[tower]),&prjs,sizeof(prjs));
#endif
       
       /* If have any projections, try matching with ACD tile */
       if (tkrStatus)
       {
           int   acdStatus;
           int skirtStatus;
           
           
           xyCnt    += (tkrStatus & 0xffff) + ((tkrStatus >> 16) & 0xffff);
           acdStatus = TFC_acdProject (prjs.prjs,
                                       prjs.xy[0],
                                       prjs.xy[1],
                                       &TFC_GeometryStd,
                                       tower,
                                       dispatch,
                                       acd_xy,
                                       acd_xz,
                                       acd_yz);
#ifdef GLEAM
           TDS_variables.acdStatus[tower]=acdStatus;
#endif
           
           
           /* Check if have any matches */
            if (acdStatus)
           {
               int which;

               /* Check whether have TOP or SIDE face match */
               which = (acdStatus & 0xf0000000) >> 28;
			   if (which == 4) {
				   if (energy < CFC_MEV_TO_LEU (30000))
					   status |= DFC_M_STATUS_TKR_TOP;//ALTERED 20 - 30 GeV Cut
			   }
               else
               {
                   /* Side face match, check if have ROW01 or ROW23 match */
				   if (acdStatus & 0x3ff) {
					   if (energy < CFC_MEV_TO_LEU (10000))
						   status |= DFC_M_STATUS_TKR_ROW01;//ALTERED 19 - 10 GeV Cut
				   }
				   else {                   
					   if ( (energy < CFC_MEV_TO_LEU (30000)) )
					       status |= DFC_M_STATUS_TKR_ROW2;//ALTERED 18 - 30 GeV Cut
				   }
               }

               return status;
           }

           
           if (energy == 0)
           {
                 
               skirtStatus = TFC_skirtProject (prjs.prjs,
                                               prjs.xy[0],
                                               prjs.xy[1],
                                               &TFC_GeometryStd,
                                               tower);

               /*
                | !!! KLUDGE !!!
                | --------------
                | Need a decision on this cut. Does one need to cancel
                | this cut if have 2 or more tracks in the event. Currently
                | it is not cancelled, so the logic says, as soon as
                | one fines any track in the skirt region, that's it.
               */
               if (skirtStatus)  return status | DFC_M_STATUS_TKR_SKIRT;
           }
           
       }
       
   }
   while (tmsk);


   if (xyCnt < 3)
   {

	   if (xyCnt < 2) {
		   if (energy > CFC_MEV_TO_LEU (250))
			   return status | DFC_M_STATUS_TKR_EQ_0;//ALTERED 17
	   }
       else            status |= DFC_M_STATUS_TKR_EQ_1;

       /* If energy low enough, must have some evidence of two tracks */
       //if (energy < CFC_MEV_TO_LEU (350)) status |= DFC_M_STATUS_TKR_LT_2_ELO;//REMOVED 15

   }
   else
   {
       /* Note that have ge 2 tracks, cancel skirt veto in this case */
       status |= DFC_M_STATUS_TKR_GE_2;
       //status &= ~DFC_M_STATUS_TKR_SKIRT;
   }

   

   /// printf ("LAT FILTER STATUS = %8.8x (xyCnt = %d)\n", status, xyCnt);
   

   return status;
}
/* ---------------------------------------------------------------------- */
       


