#ifndef AFC_REMAPDEF_H
#define AFC_REMAPDEF_H


/* ---------------------------------------------------------------------- *//*!
   
  @file   AFC_remapDef.h
  @brief  Defines the remapping structures to take one from electronics
          space to geographical space.
@verbatim

    CVS $Id$
@endverbatim

  @author JJRussell - russell@slac.stanford.edu   
                                                                         */
/* --------------------------------------------------------------------- */

#include "DFC/EBF_acdConstants.h"


#ifdef __cplusplus
extern "C" {
#endif



/* ---------------------------------------------------------------------- *//*!

  @struct _AFC_remapBrd
  @brief   Structure defining the remapping the ACD data on a single board
           from electronics space into the standard 4 geographically
           oriented lists. 
                                                                          *//*!
  @typedef AFC_remapBrd
  @brief   Typedef for struct \e _AFC_remapBrd

   This is a fairly complicated structure whose layout is driven by the
   code. It consists of 3 arrays.

   The first array is a series of masks, one for each of the geometrically
   oriented lists. For a given list in electronics space, each mask
   projects out only those members of the corresponding geometrical list.

   The next array is indexed by electronics channel and yields the
   corresponding geometrical channel. This array may contain the same
   geometrical channel more than once, but, by virtue of using the first
   array to project out only those channels belonging to a particular
   geometrical list, the duplicate channels will never direct more than
   one electronics channel to a single geometrical channel. In fact the
   first mask is just a bit list of the channels in this array belonging
   to a single geometrical list.

   The final array remaps a PHA value in electronics space into a randomly
   accessible array indexed by geometrical index. This array is differs
   from the previous array only by having the geometrical list index as
   part of the output index. In fact, the other two arrays are merely
   rearrangements of this array.
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _AFC_remapBrd
{
   unsigned int msks[EBF_K_ACD_LIST_CNT];
   /*!< Masks channels belonging to a particular bit list                 */
    
   unsigned char rma[EBF_K_ACD_NCHNS_PER_BRD];
   /*!< Gives the remapped bit for a particular channel on a board        */
    
   unsigned char rmc[EBF_K_ACD_NCHNS_PER_BRD];
   /*!< Gives the remapped PHA number for a particular channel on a board */
}
AFC_remapBrd;
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  @struct _AFC_remap
  @brief   Defines the information necessary to remap all ACD information
           laid out in electronics space to a geographical space
                                                                         *//*!
  @typedef AFC_remap
  @brief   Typedef for struct _AFC_remap
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _AFC_remap
{
    AFC_remapBrd brds[EBF_K_ACD_NBRDS];
    /*!< Remapping structure for each of front end ACD boards             */
}
AFC_remap;
/* ---------------------------------------------------------------------- */


#ifdef __cplusplus
}
#endif


#endif
