/* ---------------------------------------------------------------------- *//*!
   
   \file  AFC_splashMap1x1.c
   \brief Builds an AFC Splash Map which considers a tile to be a far
          neighbor if it is outside of a 1 tile boundary of the target
          tile.
   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $Id
\endverbatim

   This file contains no executable code, only the initialization of an
   \e AFC_splashMap. A splash map is a data structure defining, for each
   ACD tile, a set of \e near and \e far neighbors. This particular 
   instantiation of the \e AFC_splashMap defines the \e near neighbors
   as any tile that touches the target tile. All other tiles are deemed
   \e far neighbors.

   The data structure for each tile is represented by a symbolic #define
   using a macro to do the initialization. The macro is constructed such
   that the initialization not only produces the correct data but also
   a picture of the \e near and \e far tiles. For example, here is the
   initialization for tile XM 5.

  \code
    #define XM5 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  _,_,  x,x,x,x,x,  x,x,   \
                  X,_,  x,x,x,x,x,  x,x,   \
                                           \
                        _,x,x,x,x,         \
                        _,x,x,x,x)
  \endcode

   The \e x's represent the far neighbors, while the \e _'s repreesnt 
   the near tiles. For readability the \e X represents the target tile,
   but its contribution to the data structure is effectively ignored.

   The 5x5 pattern in the center represents the top ACD tiles. It is
   flanked by the 2x5 representations of the side ACD tiles. The splash
   veto only considers tiles in the upper two rows of the side ACD tiles.

   Thus, one can look at each tile and get a reasonably decent picture
   of the \e near and \e far tiles.

                                                                          */
/* ---------------------------------------------------------------------- */


#include "AFC_splashMap.h"
#include "AFC_splashMapBld.h"


#ifdef __cplusplus
extern "C" {
#endif    


/* ---------------------------------------------------------------------- *//*!

  \def   x
  \brief Short symbol to represent a tile considered to be a far neighbor
                                                                          *//*!
  \def   X
  \brief Short symbol to represent the target tile. The value of this
         symbol is the same as a 'near' neighbor. It is defined for
         visual presentation means only; i.e. it is easy to spot the
         target tile in the data definition.
                                                                          *//*!
  \def   _
  \brief Short symbol to represent a tile considered to be a near neighbot
                                                                          *//*!
  \def   MAP
  \brief Short symbol to represent the AFC_SPLASH_MAP_BLD macro
                                                                          */
/* ---------------------------------------------------------------------- */
#define x AFC_K_SPLASH_MAP_FAR
#define X AFC_K_SPLASH_MAP_NEAR
#define _ AFC_K_SPLASH_MAP_NEAR
#define MAP AFC_SPLASH_MAP_BLD
/* ---------------------------------------------------------------------- */




    

/* ---------------------------------------------------------------------- *//*!

  \def   XM0
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XM0
  \showinitializer
                                                                          *//*!
  \def   XM1
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XM1
  \showinitializer
                                                                          *//*!
  \def   XM2
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XM2
  \showinitializer
                                                                          *//*!
  \def   XM3
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XM3
  \showinitializer
                                                                          *//*!
  \def   XM4
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XM4
  \showinitializer
                                                                          *//*!
  \def   XM5
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XM5
  \showinitializer
                                                                          *//*!
  \def   XM6
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XM6
  \showinitializer
                                                                          *//*!
  \def   XM7
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XM7
  \showinitializer
                                                                          *//*!
  \def   XM8
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XM8
  \showinitializer
                                                                          *//*!
  \def   XM9
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XM9
  \showinitializer
                                                                          */
/* ---------------------------------------------------------------------- */
#define XM0 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  _,_,  _,x,x,x,x,  x,x,   \
                  _,X,  _,x,x,x,x,  x,x,   \
                                           \
                        _,x,x,x,x,         \
                        _,x,x,x,x          )


#define XM1 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  _,_,  _,x,x,x,x,  x,x,   \
                  _,X,  _,x,x,x,x,  x,x,   \
                  _,_,  _,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define XM2 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  _,_,  _,x,x,x,x,  x,x,   \
                  _,X,  _,x,x,x,x,  x,x,   \
                  _,_,  _,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define XM3 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  _,_,  _,x,x,x,x,  x,x,   \
                  _,X,  _,x,x,x,x,  x,x,   \
                  _,_,  _,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define XM4 MAP(                                       \
                        _,x,x,x,x,         \
                        _,x,x,x,x,         \
                                           \
                  _,X,  _,x,x,x,x,  x,x,   \
                  _,_,  _,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define XM5 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  _,_,  x,x,x,x,x,  x,x,   \
                  X,_,  x,x,x,x,x,  x,x,   \
                                           \
                        _,x,x,x,x,         \
                        _,x,x,x,x)

#define XM6 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  _,_,  x,x,x,x,x,  x,x,   \
                  X,_,  x,x,x,x,x,  x,x,   \
                  _,_,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define XM7 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  _,_,  x,x,x,x,x,  x,x,   \
                  X,_,  x,x,x,x,x,  x,x,   \
                  _,_,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define XM8 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  _,_,  x,x,x,x,x,  x,x,   \
                  X,_,  x,x,x,x,x,  x,x,   \
                  _,_,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define XM9 MAP(                                       \
                        _,x,x,x,x,         \
                        _,x,x,x,x,         \
                                           \
                  X,_,  x,x,x,x,x,  x,x,   \
                  _,_,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)
/* ---------------------------------------------------------------------- */


    

/* ---------------------------------------------------------------------- *//*!

  \def   XP0
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XP0
                                                                          *//*!
  \def   XP1
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XP1
  \showinitializer
                                                                          *//*!
  \def   XP2
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XP2
  \showinitializer
                                                                          *//*!
  \def   XP3
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XP3
  \showinitializer
                                                                          *//*!
  \def   XP4
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XP4
  \showinitializer
                                                                          *//*!
  \def   XP5
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XP5
  \showinitializer
                                                                          *//*!
  \def   XP6
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XP6
  \showinitializer
                                                                          *//*!
  \def   XP7
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XP7
  \showinitializer
                                                                          *//*!
  \def   XP8
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XP8
  \showinitializer
                                                                          *//*!
  \def   XP9
  \brief Internal only - 'near' and 'far' neighbors for ACD tile XP9
  \showinitializer
                                                                          */
/* ---------------------------------------------------------------------- */
#define XP0 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,_,  _,_,   \
                  x,x,  x,x,x,x,_,  X,_,   \
                                           \
                        x,x,x,x,_,         \
                        x,x,x,x,_)

#define XP1 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,_,  _,_,   \
                  x,x,  x,x,x,x,_,  X,_,   \
                  x,x,  x,x,x,x,_,  _,_,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define XP2 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,_,  _,_,   \
                  x,x,  x,x,x,x,_,  X,_,   \
                  x,x,  x,x,x,x,_,  _,_,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define XP3 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,_,  _,_,   \
                  x,x,  x,x,x,x,_,  X,_,   \
                  x,x,  x,x,x,x,_,  _,_,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define XP4 MAP(                                       \
                        x,x,x,x,_,         \
                        x,x,x,x,_,         \
                                           \
                  x,x,  x,x,x,x,_,  X,_,   \
                  x,x,  x,x,x,x,_,  _,_,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define XP5 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  _,_,   \
                  x,x,  x,x,x,x,x,  _,X,   \
                                           \
                        x,x,x,x,_,         \
                        x,x,x,x,_)

#define XP6 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  _,_,   \
                  x,x,  x,x,x,x,x,  _,X,   \
                  x,x,  x,x,x,x,x,  _,_,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define XP7 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  _,_,   \
                  x,x,  x,x,x,x,x,  _,X,   \
                  x,x,  x,x,x,x,x,  _,_,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define XP8 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  _,_,   \
                  x,x,  x,x,x,x,x,  _,X,   \
                  x,x,  x,x,x,x,x,  _,_,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)


#define XP9 MAP(                                       \
                        x,x,x,x,_,         \
                        x,x,x,x,_,         \
                                           \
                  x,x,  x,x,x,x,x,  _,X,   \
                  x,x,  x,x,x,x,x,  _,_,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)
/* ---------------------------------------------------------------------- */


    

/* ---------------------------------------------------------------------- *//*!

  \def   YM0
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YM0
  \showinitializer
                                                                          *//*!
  \def   YM1
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YM1
  \showinitializer
                                                                          *//*!
  \def   YM2
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YM2
  \showinitializer
                                                                          *//*!
  \def   YM3
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YM3
  \showinitializer
                                                                          *//*!
  \def   YM4
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YM4
  \showinitializer
                                                                          *//*!
  \def   YM5
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YM5
  \showinitializer
                                                                          *//*!
  \def   YM6
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YM6
  \showinitializer
                                                                          *//*!
  \def   YM7
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YM7
  \showinitializer
                                                                          *//*!
  \def   YM8
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YM8
  \showinitializer
                                                                          *//*!
  \def   YM9
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YM9
  \showinitializer
                                                                          */
/* ---------------------------------------------------------------------- */
#define YM0 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  _,_,  _,_,x,x,x,  x,x,   \
                                           \
                        X,_,x,x,x,         \
                        _,_,x,x,x)

#define YM1 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  _,_,_,x,x,  x,x,   \
                                           \
                        _,X,_,x,x,         \
                        _,_,_,x,x)

#define YM2 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,_,_,_,x,  x,x,   \
                                           \
                        x,_,X,_,x,         \
                        x,_,_,_,x)

#define YM3 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,_,_,_,  x,x,   \
                                           \
                        x,x,_,X,_,         \
                        x,x,_,_,_)

#define YM4 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,_,_,  _,_,   \
                                           \
                        x,x,x,_,X,         \
                        x,x,x,_,_)

#define YM5 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  _,_,  x,x,x,x,x,  x,x,   \
                                           \
                        _,_,x,x,x,         \
                        X,_,x,x,x)

#define YM6 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        _,_,_,x,x,         \
                        _,X,_,x,x)

#define YM7 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,_,_,_,x,         \
                        x,_,X,_,x)

#define YM8 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,_,_,_,         \
                        x,x,_,X,_)

#define YM9 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  _,_,   \
                                           \
                        x,x,x,_,_,         \
                        x,x,x,_,X)
/* ---------------------------------------------------------------------- */


    

/* ---------------------------------------------------------------------- *//*!

  \def   YP0
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YP0
  \showinitializer
                                                                          *//*!
  \def   YP1
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YP1
  \showinitializer
                                                                          *//*!
  \def   YP2
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YP2
  \showinitializer
                                                                          *//*!
  \def   YP3
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YP3
  \showinitializer
                                                                          *//*!
  \def   YP4
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YP4
  \showinitializer
                                                                          *//*!
  \def   YP5
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YP5
  \showinitializer
                                                                          *//*!
  \def   YP6
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YP6
  \showinitializer
                                                                          *//*!
  \def   YP7
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YP7
  \showinitializer
                                                                          *//*!
  \def   YP8
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YP8
  \showinitializer
                                                                          *//*!
  \def   YP9
  \brief Internal only - 'near' and 'far' neighbors for ACD tile YP9
  \showinitializer
                                                                          */
/* ---------------------------------------------------------------------- */
#define YP0 MAP(                                       \
                        _,_,x,x,x,         \
                        X,_,x,x,x,         \
                                           \
                  _,_,  _,_,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define YP1 MAP(                                       \
                        _,_,_,x,x,         \
                        _,X,_,x,x,         \
                                           \
                  x,x,  _,_,_,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define YP2 MAP(                                       \
                        x,_,_,_,x,         \
                        x,_,X,_,x,         \
                                           \
                  x,x,  x,_,_,_,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define YP3 MAP(                                       \
                        x,x,_,_,_,         \
                        x,x,_,X,_,         \
                                           \
                  x,x,  x,x,_,_,_,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define YP4 MAP(                                       \
                        x,x,x,_,_,         \
                        x,x,x,_,X,         \
                                           \
                  x,x,  x,x,x,_,_,  _,_,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define YP5 MAP(                                       \
                        X,_,x,x,x,         \
                        _,_,x,x,x,         \
                                           \
                  _,_,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define YP6 MAP(                                       \
                        _,X,_,x,x,         \
                        _,_,_,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define YP7 MAP(                                       \
                        x,_,X,_,x,         \
                        x,_,_,_,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define YP8 MAP(                                       \
                        x,x,_,X,_,         \
                        x,x,_,_,_,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define YP9 MAP(                                       \
                        x,x,x,_,X,         \
                        x,x,x,_,_,         \
                                           \
                  x,x,  x,x,x,x,x,  _,_,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)
/* ---------------------------------------------------------------------- */

    

    
/* ---------------------------------------------------------------------- *//*!

  \def   T00
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T0
  \showinitializer
                                                                          *//*!
  \def   T01
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T01
  \showinitializer
                                                                          *//*!
  \def   T02
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T0
  \showinitializer
                                                                          *//*!
  \def   T03
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T0
  \showinitializer
                                                                          *//*!
  \def   T04
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T04
  \showinitializer
                                                                          *//*!
  \def   T05
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T05
  \showinitializer
                                                                          *//*!
  \def   T06
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T0
  \showinitializer
                                                                          *//*!
  \def   T07
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T07
  \showinitializer
                                                                          *//*!
  \def   T08
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T08
  \showinitializer
                                                                          *//*!
  \def   T09
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T09
  \showinitializer
                                                                          *//*!
  \def   T10
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T10
  \showinitializer
                                                                          *//*!
  \def   T11
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T11
  \showinitializer
                                                                          *//*!
  \def   T12
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T12
  \showinitializer
                                                                          *//*!
  \def   T13
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T13
  \showinitializer
                                                                          *//*!
  \def   T14
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T14
  \showinitializer
                                                                          *//*!
  \def   T15
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T15
  \showinitializer
                                                                          *//*!
  \def   T16
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T16
  \showinitializer
                                                                          *//*!
  \def   T17
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T17
  \showinitializer
                                                                          *//*!
  \def   T18
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T18
  \showinitializer
                                                                          *//*!
  \def   T19
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T19
  \showinitializer
                                                                          *//*!
  \def   T20
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T20
  \showinitializer
                                                                          *//*!
  \def   T21
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T21
  \showinitializer
                                                                          *//*!
  \def   T22
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T22
  \showinitializer
                                                                          *//*!
  \def   T23
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T23
  \showinitializer
                                                                          *//*!
  \def   T24
  \brief Internal only - 'near' and 'far' neighbors for ACD tile T24
  \showinitializer
                                                                          */
/* ---------------------------------------------------------------------- */
#define T00 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,_,  _,_,x,x,x,  x,x,   \
                  x,_,  X,_,x,x,x,  x,x,   \
                                           \
                        _,_,x,x,x,         \
                        x,x,x,x,x)

#define T01 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  _,_,_,x,x,  x,x,   \
                  x,x,  _,X,_,x,x,  x,x,   \
                                           \
                        _,_,_,x,x,         \
                        x,x,x,x,x)

#define T02 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,_,_,_,x,  x,x,   \
                  x,x,  x,_,X,_,x,  x,x,   \
                                           \
                        x,_,_,_,x,         \
                        x,x,x,x,x)

#define T03 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,_,_,_,  x,x,   \
                  x,x,  x,x,_,X,_,  x,x,   \
                                           \
                        x,x,_,_,_,         \
                        x,x,x,x,x)

#define T04 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,_,_,  _,x,   \
                  x,x,  x,x,x,_,X,  _,x,   \
                                           \
                        x,x,x,_,_,         \
                        x,x,x,x,x)

#define T05 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,_,  _,_,x,x,x,  x,x,   \
                  x,_,  X,_,x,x,x,  x,x,   \
                  x,_,  _,_,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T06 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  _,_,_,x,x,  x,x,   \
                  x,x,  _,X,_,x,x,  x,x,   \
                  x,x,  _,_,_,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T07 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,_,_,_,x,  x,x,   \
                  x,x,  x,_,X,_,x,  x,x,   \
                  x,x,  x,_,_,_,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T08 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,_,_,_,  x,x,   \
                  x,x,  x,x,_,X,_,  x,x,   \
                  x,x,  x,x,_,_,_,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T09 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,_,_,  _,x,   \
                  x,x,  x,x,x,_,X,  _,x,   \
                  x,x,  x,x,x,_,_,  _,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T10 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,_,  _,_,x,x,x,  x,x,   \
                  x,_,  X,_,x,x,x,  x,x,   \
                  x,_,  _,_,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T11 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  _,_,_,x,x,  x,x,   \
                  x,x,  _,X,_,x,x,  x,x,   \
                  x,x,  _,_,_,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T12 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,_,_,_,x,  x,x,   \
                  x,x,  x,_,X,_,x,  x,x,   \
                  x,x,  x,_,_,_,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T13 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,_,_,_,  x,x,   \
                  x,x,  x,x,_,X,_,  x,x,   \
                  x,x,  x,x,_,_,_,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T14 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,_,_,  _,x,   \
                  x,x,  x,x,x,_,X,  _,x,   \
                  x,x,  x,x,x,_,_,  _,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T15 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,_,  _,_,x,x,x,  x,x,   \
                  x,_,  X,_,x,x,x,  x,x,   \
                  x,_,  _,_,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T16 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  _,_,_,x,x,  x,x,   \
                  x,x,  _,X,_,x,x,  x,x,   \
                  x,x,  _,_,_,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T17 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,_,_,_,x,  x,x,   \
                  x,x,  x,_,X,_,x,  x,x,   \
                  x,x,  x,_,_,_,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T18 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,_,_,_,  x,x,   \
                  x,x,  x,x,_,X,_,  x,x,   \
                  x,x,  x,x,_,_,_,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T19 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,x,x,         \
                                           \
                  x,x,  x,x,x,_,_,  _,x,   \
                  x,x,  x,x,x,_,X,  _,x,   \
                  x,x,  x,x,x,_,_,  _,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T20 MAP(                                       \
                        x,x,x,x,x,         \
                        _,_,x,x,x,         \
                                           \
                  x,_,  X,_,x,x,x,  x,x,   \
                  x,_,  _,_,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T21 MAP(                                       \
                        x,x,x,x,x,         \
                        _,_,_,x,x,         \
                                           \
                  x,x,  _,X,_,x,x,  x,x,   \
                  x,x,  _,_,_,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T22 MAP(                                       \
                        x,x,x,x,x,         \
                        x,_,_,_,x,         \
                                           \
                  x,x,  x,_,X,_,x,  x,x,   \
                  x,x,  x,_,_,_,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T23 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,_,_,_,         \
                                           \
                  x,x,  x,x,_,X,_,  x,x,   \
                  x,x,  x,x,_,_,_,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)

#define T24 MAP(                                       \
                        x,x,x,x,x,         \
                        x,x,x,_,_,         \
                                           \
                  x,x,  x,x,x,_,X,  _,x,   \
                  x,x,  x,x,x,_,_,  _,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                  x,x,  x,x,x,x,x,  x,x,   \
                                           \
                        x,x,x,x,x,         \
                        x,x,x,x,x)
/* ---------------------------------------------------------------------- */

    

    
/* ---------------------------------------------------------------------- *//*!

  \var   const struct _AFC_splashMap AFC_SplashMap1x1
  \brief Data structure representing the SplashMap with only nearest
         neighbors considered to be 'near' tiles.
                                                                          */
/* ---------------------------------------------------------------------- */
const struct _AFC_splashMap AFC_SplashMap1x1 =
{
   {
           YP8, YP7, YP6, YP5, 
      YP4, YP3, YP2,

      T24, T23, T22, T21, T20,
      T19, T18, T17, T16, T15, 
      T14, T13, T12, T11, T10,
      T09, T08, T07, T06, T05,
      T04, T03, T02, T01, T00
   },
   
   {
                    YP1, YP0,

     YM9, YM8, YM7, YM6, YM5, 
     YM4, YM3, YM2, YM1, YM0,

     XP9, XP8, XP7, XP6, XP5,
     XP4, XP3, XP2, XP1, XP0,

     XM9, XM8, XM7, XM6, XM5, 
     XM4, XM3, XM2, XM1, XM0
   },
   
   YP9
   
};
/* ---------------------------------------------------------------------- */   


#ifdef __cplusplus
}
#endif    
    

