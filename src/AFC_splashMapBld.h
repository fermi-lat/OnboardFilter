#ifndef AFC_SPLASHMAPBLD_H
#define AFC_SPLASHMAPBLD_H



/* ---------------------------------------------------------------------- *//*!
   
   \file  AFC_splashMapBld.h
   \brief Macros to help build a Splash Veto Map 
   \author JJRussell - russell@slac.stanford.edu

\verbatim
   CVS $Id
\endverbatim

    This file should only be included when building a splash veto map.
                                                                          */
/* ---------------------------------------------------------------------- */


#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------- *//*!

  \def  AFC_SPLASH_MAP_BLD()
  \brief Builds the data structure which determines which tiles are
         considered near and far neighbors of a target file.

   In the end, this macro produces an initialization consisting of 2
   hex numbers. These numbers could have been typed in directly or
   produced with the aid of C program. The tact taken here was to provide
   a macro which directly initialized the data, but did it in a fashion
   that was pictorially obvious.

   For example, the following snippet is a possible initialization of
   the near and far neighbors of TOP tile 0.

  \verbatim

   #define TopTile00 MAP(        x,x,x,x,x,         \
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
  \endverbatim

  The \e x's represent the tiles considered far neighbors, the \e _'s
  represent the tiles considered near neighbors. The \e X is used to
  mark the target tile. Its value is the same as a near neighbor, but
  it serves to aid the eye. In order to keep the code readable, the 
  \e x's, \e X and \e _'s have been equated to the longer names
  AFC_K_SPLASH_MAP_NEAR and AFC_K_SPLASH_MAP_FAR.
                                                                          */
/* ---------------------------------------------------------------------- */
#define AFC_SPLASH_MAP_BLD(                                               \
                           _yp5, _yp6, _yp7, _yp8, _yp9,                  \
                           _yp0, _yp1, _yp2, _yp3, _yp4,                  \
                                                                          \
             _xm9, _xm4,   _t20, _t21, _t22, _t23, _t24,   _xp4, _xp9,    \
             _xm8, _xm3,   _t15, _t16, _t17, _t18, _t19,   _xp3, _xp8,    \
             _xm7, _xm2,   _t10, _t11, _t12, _t13, _t14,   _xp2, _xp7,    \
             _xm6, _xm1,   _t05, _t06, _t07, _t08, _t09,   _xp1, _xp6,    \
             _xm5, _xm0,   _t00, _t01, _t02, _t03, _t04,   _xp0, _xp5,    \
                                                                          \
                           _ym0, _ym1, _ym2, _ym3, _ym4,                  \
                           _ym5, _ym6, _ym7, _ym8, _ym9)                  \
                                                                          \
  { ( (_yp8 << 31) | (_yp7 << 30) | (_yp6 << 29) | (_yp5 << 28) |         \
      (_yp4 << 27) | (_yp3 << 26) | (_yp2 << 25) | (_t24 << 24) |         \
      (_t23 << 23) | (_t22 << 22) | (_t21 << 21) | (_t20 << 20) |         \
      (_t19 << 19) | (_t18 << 18) | (_t17 << 17) | (_t16 << 16) |         \
      (_t15 << 15) | (_t14 << 14) | (_t13 << 13) | (_t12 << 12) |         \
      (_t11 << 11) | (_t10 << 10) | (_t09 <<  9) | (_t08 <<  8) |         \
      (_t07 <<  7) | (_t06 <<  6) | (_t05 <<  5) | (_t04 <<  4) |         \
      (_t03 <<  3) | (_t02 <<  2) | (_t01 <<  1) | (_t00 <<  0) ),        \
                                                                          \
    ( (_yp1 << 31) | (_yp0 << 30) | (_ym6 << 29) | (_ym5 << 28) |         \
      (_ym4 << 27) | (_ym3 << 26) | (_ym2 << 25) | (_ym4 << 24) |         \
      (_ym3 << 23) | (_ym2 << 22) | (_ym1 << 21) | (_ym0 << 20) |         \
      (_xp9 << 19) | (_xp8 << 18) | (_xp7 << 17) | (_xp6 << 16) |         \
      (_xp5 << 15) | (_xp4 << 14) | (_xp3 << 13) | (_xp2 << 12) |         \
      (_xp1 << 11) | (_xp0 << 10) | (_xm9 <<  9) | (_xm8 <<  8) |         \
      (_xm7 <<  7) | (_xm6 <<  6) | (_xm5 <<  5) | (_xm4 <<  4) |         \
      (_xm3 <<  3) | (_xm2 <<  2) | (_xm1 <<  1) | (_xm0 <<  0) ) }
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \def   AFC_K_SPLASH_MAP_NEAR
  \brief The value to use for tiles considered to be near neighbors
                                                                          *//*!
  \def   AFC_K_SPLASH_MAP_FAR
  \brief The value to use for tiles considered to be far neighbors

   It is suggested that these values be used to define very short local
   symbols, as in the example where \e x, \e X and \e _, where used.
                                                                          */
/* ---------------------------------------------------------------------- */
#define AFC_K_SPLASH_MAP_NEAR 0
#define AFC_K_SPLASH_MAP_FAR  1
/* ---------------------------------------------------------------------- */


#ifdef __cplusplus
}
#endif


#endif

