/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  EBF_swap.c
   \brief Byte swapping routines
   \author JJRussell - russell@slac.stanford.edu

    This is a cheap version of byte swapping routines. They are meant to
    be used only locally within this package in an attempt to keep it
    more or less self-contained.
                                                                          */
/* ---------------------------------------------------------------------- */


#include "EBF_swap.h"
#include "DFC/DFC_endianness.h"



/* --------------------------------------------------------------------- *//*!

    \fn unsigned int EBF_swap32(unsigned int w)
    \brief   Byte swaps a 32 bit word
    \param w The 32 bit word to swap

                                                                         *//*!
    \fn unsigned int EBF_swap32_lclXbig(unsigned int w)
    \brief   Byte swaps a 32 bit word; local representation <=> big
    \param w The 32 bit word to swap
                                                                         *//*!
    \fn unsigned int EBF_swap32_lclXlittle(unsigned int w)
    \brief   Byte swaps a 32 bit word; local representation <=> little 
    \param w The 32 bit word to swap

                                                                         *//*!
    \fn unsigned int EBF_swap16(unsigned short w)
    \brief   Byte swaps  a 16 bit words
    \param w The 16 bit word to swap
                                                                         *//*!
    \fn unsigned short EBF_swap16_lclXbig(unsigned short w)
    \brief   Byte swap a 16 bit words; local representation <=> big
    \param w The 16 bit words to swap
                                                                         *//*!
    \fn void EBF_swap16_lclXlittle(unsigned short w) 
    \brief   Byte swaps a 16 bit words; local representation <=> little
    \param w The 16 bit words to swap
    
                                                                         *//*!
   
    \fn void EBF_swap32N(unsigned int *w, int n)
    \brief   Byte swaps    32 bit words
    \param w The array  of 32 bit words to swap
    \param n The number of 32 bit words to swap
                                                                         *//*!
    \fn void EBF_swap32_lclXbigN(unsigned int *w, int n)
    \brief   Byte swaps    32 bit words; local representation <=> big
    \param w The array  of 32 bit words to swap
    \param n The number of 32 bit words to swap
                                                                         *//*!
    \fn void EBF_swap32_lclXlittleN(unsigned int *w, int n)
    \brief   Byte swaps    32 bit words; local representation <=> little 
    \param w The array  of 32 bit words to swap
    \param n The number of 32 bit words to swap
                                                                         *//*!
    \fn void EBF_swap16N(unsigned short *w, int n)
    \brief   Byte swaps    16 bit words
    \param w The array  of 16 bit words to swap
    \param n The number of 16 bit words to swap
                                                                         *//*!
    \fn void EBF_swap16_lclXbigN(unsigned short *w, int n)
    \brief   Byte swaps    16 bit words; local representation <=> big
    \param w The array  of 16 bit words to swap
    \param n The number of 16 bit words to swap
                                                                         *//*!
    \fn void EBF_swap16_lclXlittleN(unsigned short *w, int n) 
    \brief   Byte swaps    16 bit words; local representation <=> little
    \param w The array  of 16 bit words to swap
    \param n The number of 16 bit words to swap
    


                                                                           */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- */
__inline unsigned int EBF_swap32 (unsigned int w)
{
   return   (((w >>  0) & 0xff) << 24)
          | (((w >>  8) & 0xff) << 16)
          | (((w >> 16) & 0xff) <<  8)
          |  ((w >> 24) & 0xff);
}
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- */
__inline unsigned int EBF_swap16 (unsigned short w)
{
   return   (((w >> 0) & 0xff) <<  8)
          |  ((w >> 8) & 0xff);
}
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- */
__inline void EBF_swap32N (unsigned int *w, int n)
{
    while (--n >= 0)
    {
        unsigned int tmp = *w;
        *w++ = (((tmp >>  0) & 0xff) << 24)
             | (((tmp >>  8) & 0xff) << 16)
             | (((tmp >> 16) & 0xff) <<  8)
             |  ((tmp >> 24) & 0xff);
    }

    return;
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- */
__inline void EBF_swap16N (unsigned short *w, int n)
{
    while (--n >= 0)
    {
        unsigned short int tmp = *w;
        *w++ = (((tmp >> 0) & 0xff) <<  8)
             |  ((tmp >> 8) & 0xff);
    }

    return;
}
/* --------------------------------------------------------------------- */



#if DFC_C_ENDIANNESS==DFC_K_ENDIANNESS_LITTLE


/* Single word swap routines */
unsigned int EBF_swap32_lclXlittle(unsigned int   w) { return w;              }
unsigned int EBF_swap16_lclXlittle(unsigned short w) { return w;              }
unsigned int EBF_swap32_lclXbig   (unsigned int   w) { return EBF_swap32 (w); }
unsigned int EBF_swap16_lclXbig   (unsigned short w) { return EBF_swap16 (w); }

/* Array swap routines */
void EBF_swap32_lclXlittleN (unsigned int   *w, int n) { return;              }
void EBF_swap16_lclXlittleN (unsigned short *w, int n) { return;              }
void EBF_swap32_lclXbigN    (unsigned int   *w, int n) { EBF_swap32N (w, n);  }
void EBF_swap16_lclXbigN    (unsigned short *w, int n) { EBF_swap16N (w, n);  }


#else if DFC_C_ENDIANNESS==DFC_K_ENDIANNESS_BIG


/* Single word swap routines */
unsigned int EBF_swap32_lclXlittle (unsigned int   w) { return EBF_swap32(w); }
unsigned int EBF_swap16_lclXlittle (unsigned short w) { return EBF_swap16(w); }
unsigned int EBF_swap32_lclXbig    (unsigned int   w) { return w;             }
unsigned int EBF_swap16_lclXbig    (unsigned short w) { return w;             }


/* Array swap routines */
void EBF_swap32_lclXlittleN (unsigned int   *w, int n) { EBF_swap32N (w, n);  }
void EBF_swap16_lclXlittleN (unsigned short *w, int n) { EBF_swap16N (w, n);  }
void EBF_swap32_lclXbigN    (unsigned int   *w, int n) { return;              }
void EBF_swap16_lclXbigN    (unsigned short *w, int n) { return;              }


#endif








