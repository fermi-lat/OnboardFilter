#ifndef FFS_H
#define FFS_H

#ifdef   VXWORKS
#include <types/vxCpu.h>
#endif

#include "windowsCompat.h"

/*
//  ROUTINE
//  -------
//      FFS  - find first set bit in a longword
//
//  DESCRIPTION
//  -----------
//  This routine scans a 32 bit longword from left (MSB) to right (LSB)
//  looking for the first bit set. If it finds a bit it returns a value
//  in the range 0-31. The routine is not protected against 0 as an
//  input and the result is undefined.
//
//  This is a direct map to the PPC instruction 'cntlz', count leading zeros,
//  which is, in fact, the PPC implementation.
//
//  The Intel implementation is an 'bsr' instruction with the bit numbers
//  reversed to match a big endian machine.
//
//  The non PPC604 implementation is a straighforward piece of C code.
//
//  RETURNS
//  -------
//  If there is a set bit, it will return a number from 0-31
//  If there is not a set bit, it will return 32.
*/

#ifdef __cplusplus
extern "C" {
#endif    

static int FFS (unsigned lw);

#if defined(CPU_FAMILY) && (CPU_FAMILY == PPC)

    static inline int FFS (unsigned lw)
    {
       int n;
       asm("cntlzw %0,%1": "=r"(n): "r"(lw));
       return n;
    }


#elif defined(__i386) && !defined(_WIN32)// && defined (__GNUC__)

    static inline int FFS (unsigned lw)
    {
        int n;
        asm volatile ("bsr %1,%0" : "=r"((int)n) : "r"((int)lw));
        return n ^ 31;
    }
        

#else

  static inline int FFS (unsigned lw)    
  {
     int n;
     int cpy = lw;
     if (cpy == 0) n = 32;
     else  { n = 0; while(cpy >= 0) { cpy <<= 1; n++; } }
     return n;
  }

#endif


#ifdef __cplusplus
}
#endif    

    
#endif








