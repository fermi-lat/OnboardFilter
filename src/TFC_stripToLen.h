#ifndef TFC_STRIP_TO_LEN_H
#define TFC_STRIP_TO_LEN_H


#ifdef __cplusplus
extern "C"
#endif


/* ------------------------------------------------------------------- */
/* ------------ Conversion by the register lookup method  ------------ */
/*                                                                     */

/* ------------------------------------------------------------------- *//*!

  \def   TFC_K_STRIP_TO_LEN_LADDER_GAP
  \brief Defines the inter-ladder gap in units of strips.
                                                                       *//*!
  \def   TFC_STRIP_TO_LEN_GEN(_n)
  \brief Defines the bit array entry which translates a segment number
         (which set of 128 wide strips) into the ladder it is in. This
         is an internal use only macro. The user should use
         TFC_STRIP_TO_LEN_DECLARE to drop an instance of the translation
         array. This technique allows the compiler to optimize this
         value to a register.
                                                                      *//*!
  \def   TFC_STRIP_TO_LEN_DECLARE(_sXl)
  \brief Declares an instance of the segment to ladder translation array.
         The variable declared by this macro should be passed to the
         the TFC__stripToLen translation routine.
                                                                       */
/* ------------------------------------------------------------------- */
#define TFC_K_STRIP_TO_LEN_LADDER_GAP 9
#define TFC_STRIP_TO_LEN_GEN(_n)  ( (_n / 3) << (2 * _n) )
#define TFC_STRIP_TO_LEN_DECLARE(_sXl)                        \
             unsigned int _sXl = TFC_STRIP_TO_LEN_GEN(0x0) |  \
                                 TFC_STRIP_TO_LEN_GEN(0x1) |  \
                                 TFC_STRIP_TO_LEN_GEN(0x2) |  \
                                 TFC_STRIP_TO_LEN_GEN(0x3) |  \
                                 TFC_STRIP_TO_LEN_GEN(0x4) |  \
                                 TFC_STRIP_TO_LEN_GEN(0x5) |  \
                                 TFC_STRIP_TO_LEN_GEN(0x6) |  \
                                 TFC_STRIP_TO_LEN_GEN(0x7) |  \
                                 TFC_STRIP_TO_LEN_GEN(0x8) |  \
                                 TFC_STRIP_TO_LEN_GEN(0x9) |  \
                                 TFC_STRIP_TO_LEN_GEN(0xA) |  \
                                 TFC_STRIP_TO_LEN_GEN(0xB);

static inline unsigned int TFC__stripToLen (unsigned int    s,
                                            unsigned int  sXl);
/* ------------------------------------------------------------------- */




/* ------------------------------------------------------------------- *//*!

  \fn  unsigned int TFC__stripToLen (unsigned int   s,
                                     unsigned int sXl)

  \brief     Converts an electronics strip number into a number which
             measures distance in units of strip numbers. This takes
             into account the gaps that occur between ladders.
  \param s   The strip number of convert
  \param sXl 32-bit lookup that converts a segment number to a ladder number
  \return    The converted strip number

  
  \b DESCRIPTION \n
  \b ----------- \n
  Convert an electronics strip number into a number which measures distance
  in units of strip numbers. This takes into account the gaps between the
  ladders.  This routine uses a bizarre lookup method, which is explained
  below. 


  \b ALGORITHM/MOTIVATION \n
  \b -------------------- \n
  The calculation one must do is to isolate the strip number to one of
  the ladders. If there are an equal number of strips in each piece, then
  the most straightforward thing to do is to divide the strip number by
  the number of strips per piece, multiple this by the gap and add this to
  the original strip number, i.e.

  \verbatim
     physical_strip_number = strip_number
                           + strip_number/NUMBER_OF_STRIPS_PER_SECTION * GAP;
  \endverbatim                           

  This method is straightforward. However, the divide is computational very
  expensive (>30 cycles on a PowerPC).
  
  There are other strategies. One is to locate the ladder by seeing
  if the strip number is greater than the number of strips in one ladder,
  then seeing if the strip number is greater than the number of strips in
  next ladder, etc. This is a good strategy if the number of ladders is small,
  which it is (4). This is  essentially division by successive subtraction.

  The other strategy is a lookup table. The size of the lookup table can be
  reduced because the sections are always constructed of some integral number
  of 64 strips. The maximum strip number is less than 1536. The current design
  was a gap every 384 strips, for a total of 4 ladders. Each ladder contains
  3 128 strip wide pieces. An effective division of a number (no bigger than
  1536/128 = 12) by 3 must be performed.

  While a lookup table of 12 entries giving the result could be easily
  constructed, a more efficient algorithm is to pack all the answers into a
  register and use a shift operation to extract them. Let me illustrate
  with a small example. Suppose one needs to divide the numbers 0-14 by 5.
  The answer is no bigger than 3, so one only needs 2 bits for each of the
  15 numbers in the range 0-14. One just initializes a 32 bit register with
  the value (arranged lsb to msb in the register):

              R = 0,0,0,0,0,1,1,1,1,1,2,2,2,2,2 = 0x22 25 54 00

  If one wishes to know what 13/5 is then

              13/5 = (R >> (13<<1)) & 0x3

  This takes 3 cycles (one to shift 13 by 1, one to shift R, and one for
  the and operations. With any luck, some other operation can be interleaved.
  This is faster than a lookup table, even if the lookup table is in the cache.

  The strip number is first shift by 7 places, giving a number from 0-11
  indicating which of the 12 128 strip wide pieces this strip number is in.
  Segments 0,1,2 are in ladder 0; 3,4,5 in ladder 1; 6,7,8 in ladder 2; and
  9,10,11 are in ladder 3. Each answer occupies 2 bits, so need a total of
  24 bits to contain the 12 numbers.
                                                                       */
/* ------------------------------------------------------------------- */
static unsigned int TFC__stripToLen (unsigned int strip,
                                     unsigned int   sXl)
{
   int segment = (sXl >> ((strip >> 6) & 0x1e)) & 0x3;
   return  strip + segment * TFC_K_STRIP_TO_LEN_LADDER_GAP;
}
/* ------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif
   
#endif
