#ifndef FFS_H
#define FFS_H



/* ----------------------------------------------------------------------- *//*!

  \file   ffs.h
  \brief  Provides the interface and implementation of the Find First Set
          routines.
  \author JJRussell - russell@slac.stanford.edu

\verbatim

CVS $Id
\endverbatim

  \b ABSTRACT \n
     -------- \n
     This facility provides the interface and implementation of the Find
     First Set routines. These routines provide a machine independent 
     interface to a machine dependent implementation to find the first
     set bit in a 32 bit word. On most machines, this consists of a single
     machine instruction
 
  \b USAGE \n
     ----- \n
     Two routines are provided, \e FFS() and FFS_eliminate(). These 
     routines find and then eliminate the first set bit in a 32 bit word.
     
  \b Example \n
     ------- \n
     As example, consider counting the number of set bits in a 32-bit word.

  \code

     int count_bits (unsigned int word)
     {
        int cnt = 0;
        while (word)
        {
	    int bit;
            bit  = FFS (word);
            cnt += 1;
            word = FFS_eliminate (word, bit);
        }
        return cnt;
     }

  \endcode
                                                                           */
/* ----------------------------------------------------------------------- */



/* ----------------------------------------------------------------------- *//*!

  \fn     int FFS (unsigned int word)
  \brief  Finds the first set bit (MSB = bit 0) in a 32-bit word
  \return Returns the bit number of the first set bit

   This routine scans a 32 bit longword from left (MSB) to right (LSB)
   looking for the first bit set. If it finds a bit it returns a value
   in the range 0-31. The routine is \b not protected against 0 as an 
   input and the result is undefined.

   On the PowerPCs, this is a direct map to the PPC instruction 'cntlz',
   count leading zeros.

   The Intel implementation is an 'bsr' instruction with the bit numbers
   reversed to match a big endian machine.

   The non PPC604 implementation is a straighforward piece of C code.
									   */
/* ----------------------------------------------------------------------- */




/* ----------------------------------------------------------------------- *//*!

  \fn     int FFS_eliminate (unsigned int word, int bit)
  \brief  Eliminates the specified bit, \a bit from \a word.
  \return The value of the input word with the specified bit eliminated.

  \param word  The word to eliminate the bit from
  \param  bit  The bit (MSB = 0) to eliminate

   This is merely a convenience routine. Nothing fancy is going on here, 
   just a straightforward elimination of the specified bit using a mask
   operation. It is provided to eliminate the inevitable mistake of 
   misspecifying the mask word (0x80000000 >> bit), for instance by 
   omitting one of the trailing 0's. That kind of mistake is almost 
   impossible to spot.
									   */
/* ----------------------------------------------------------------------- */


/* ----------------------------------------------------------------------- *//*!

  \fn     unsigned int FFS_mask (int bit)
  \brief  Converts the bit to a bit mask 
  \return A 32 bit value with the specified bit set (MSB = bit 0)

  \param  bit  The bit (MSB = 0) to set.

   Nothing fancy, this routine exists for consistency across the FFS 
   routines.
									   */
/* ----------------------------------------------------------------------- */



#ifdef   VXWORKS
#include <types/vxCpu.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif    

static __inline          int FFS           (unsigned int word);
static __inline unsigned int FFS_eliminate (unsigned int word, int bit);
static __inline unsigned int FFS_mask      (int bit);

/* ----------------------------------------------------------------------- */
static __inline unsigned int FFS_eliminate (unsigned int word, int bit)
{
   return word & ~(0x80000000 >> bit);
}
/* ----------------------------------------------------------------------- */



/* ----------------------------------------------------------------------- */
static __inline unsigned int FFS_mask (int bit)
{
  return 0x80000000 >> bit;
}
/* ----------------------------------------------------------------------- */


#if defined(CPU_FAMILY) && (CPU_FAMILY == PPC)

    static __inline int FFS (unsigned int word)
    {
       int n;
       asm("cntlzw %0,%1": "=r"(n): "r"(word));
       return n;
    }


#elif defined(__i386) && !defined(_WIN32) // && defined (__GNUC__)

    static __inline int FFS (unsigned int word)
    {
        int n;
        asm volatile ("bsr %1,%0" : "=r"((int)n) : "r"((int)word));
        return n ^ 31;
    }
        

#else

  static __inline int FFS (unsigned int word)    
  {
     int n;
     int cpy = word;
     if (cpy == 0) n = 32;
     else  { n = 0; while(cpy >= 0) { cpy <<= 1; n++; } }
     return n;
  }

#endif


#ifdef __cplusplus
}
#endif    

    
#endif








