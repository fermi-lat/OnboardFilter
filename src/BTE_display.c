/* ---------------------------------------------------------------------- *//*!
   
   \file  BTE_display.c
   \brief Binary Tree Encoding Display routines, implemenation file
   \author JJRussell - russell@slac.stanford.edu

    Implementation of the display routines for the binary tree encoding
    facility.
                                                                          */
/* ---------------------------------------------------------------------- */


#include <stdio.h>


   
/*! \def     D   \brief  Local symbol for a centered   display character  */
/*! \def     l   \brief  Local symbol for a left-hand  display character  */
/*! \def     r   \brief  Local symbol for a right-hand display character  */
/*! \def D_x_D   \brief  Local symbol for a Level 3 display branch        */
/*! \def DD_DD   \brief  Local symbol for a Level 4 display branch        */
/*! \def DISPLAY \brief  Local symbol for the display format string       */


#define  D     " %c "
#define  l     " %c-"
#define  r     "-%c "
#define  D_x_D " %c-+-%c "
#define  DD_DD " %2.2s %2.2s "


#define DISPLAY "\n"                                                      \
"                               "D"\n"                                    \
"                                |\n"                                     \
"                "l"-------------+---------------"r"\n"                   \
"                 |                               |\n"                    \
"        "l"------+------"r"             "l"------+------"r"\n"           \
"         |               |               |               |\n"            \
"    "l"--+--"r"     "l"--+--"r"     "l"--+--"r"     "l"--+--"r"\n"       \
"     |       |       |       |       |       |       |       |\n"        \
"  "D_x_D" "D_x_D" "D_x_D" "D_x_D" "D_x_D" "D_x_D" "D_x_D" "D_x_D"\n"     \
"  "DD_DD" "DD_DD" "DD_DD" "DD_DD" "DD_DD" "DD_DD" "DD_DD" "DD_DD"\n"


/* ---------------------------------------------------------------------- *//*!

  \def   C(_c, _w, _s)
  \brief Local macro to extract a single bit from w and display it

  \param _c  2-entry lookup array giving the display character for
             the bit (one-entry for the bit=0, one for the bit=1)
  \param _w  The 32 bit word to extract the bit from
  \param _s  The right shift needed to locate the bit to display
  \return    The character to display
                 
                                                                          *//*!
  \def   S(_c, _w, _s)
  \brief Local macro to extract a pair of bits from w and display them

  \param _c  4-entry lookup array giving the display characters for
             the pair of bits (one-entry for each of the 4 possible
             values of the pair of bits.
  \param _w  The 32 bit word to extract the bit from
  \param _s  The right shift needed to locate the lower bit of the
             pair of bits to display
  \return    Pointer to the array of the pair of characters to
             display.
                                                                          */
/* ---------------------------------------------------------------------- */
#define C(_c, _w, _s) _c[(_w >> _s) & 1]
#define S(_c, _w, _s) _c[(_w >> _s) & 3]
/* ---------------------------------------------------------------------- */
                               
                              

/* ---------------------------------------------------------------------- *//*!

   \fn    void BTE_wordDisplay (unsigned int w,
                                unsigned int p)
   \brief Draws a stick figure outline of the binary tree as represented
          by the two input parameters 
                                
   \param w     The original 32-bit word to encode
   \param p     The pattern word of the higher levels of the binary tree.
                This is the value returned by \e BTE_wordPrepare().
                                                                          */
/* ---------------------------------------------------------------------- */
void BTE_wordDisplay (unsigned int w,
                      unsigned int p)
{
  static  const char F[4][2] = { {' ',' '},
                                 {'.','1'},
                                 {'1','.'},
                                 {'1','1'}
                               };
 
  const char *f = F[1];

  if (w) p = (0x01 << 30) | (p >> 2);
  
 
  printf (DISPLAY,
          C(f, p, 30),
          C(f, p, 29), C(f, p, 28),
          
          C(f, p, 27), C(f, p, 26), C(f, p, 25), C(f, p, 24),
          
          C(f, p, 23), C(f, p, 22), C(f, p, 21), C(f, p, 20),
          C(f, p, 19), C(f, p, 18), C(f, p, 17), C(f, p, 16),

          C(f, p, 15), C(f, p, 14), C(f, p, 13), C(f, p, 12),
          C(f, p, 11), C(f, p, 10), C(f, p,  9), C(f, p,  8),
          C(f, p,  7), C(f, p,  6), C(f, p,  5), C(f, p,  4),
          C(f, p,  3), C(f, p,  2), C(f, p,  1), C(f, p,  0),

          S(F, w, 30), S(F, w, 28), S(F, w, 26), S(F, w, 24),
          S(F, w, 22), S(F, w, 20), S(F, w, 18), S(F, w, 16),
          S(F, w, 14), S(F, w, 12), S(F, w, 10), S(F, w,  8),
          S(F, w,  6), S(F, w,  4), S(F, w,  2), S(F, w,  0));
          
 return;
 
}
/* ---------------------------------------------------------------------- */

#undef D
#undef L
#undef R
#undef D_x_D
#undef DD_DD
         


