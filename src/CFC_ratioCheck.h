#ifndef CFC_RATIOCHECK_H
#define CFC_RATIOCHECK_H


/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  CFC_ratioCheck.h    
   \brief Checks if the ratio of two numbers is between the specified
          limits.
   \author JJRussell - russell@slac.stanford.edu

    This function checks whether the ratio of two integer arguments is
    between the specified limits. The limits are given in units of
    1024, ie 100% = 1024, 1% = .01 * 1024. The arthimetic is done without
    any divisions and is guarded for overflow and underflow.
                                                                          */
/* ---------------------------------------------------------------------- */


#include "ffs.h"

#ifdef __cplusplus
extern "C" {
#endif


/* --------------------------------------------------------------------- *//*!
    
  \fn  unsigned int CFC__ratioCheck (int num,      int den,
                                     int lo_limit, int lo_status,
                                     int hi_limit, int hi_status)

  \brief Checks that the ratio of num/den is between the specified limits
  \param num       The numerator
  \param den       The denominator
  \param lo_limit  The lo limit of the ratio, in units of 1024
  \param lo_status The status value to return if <= the low limit.
  \param hi_limit  The hi limit of the ratio, in units of 1024
  \param hi_status The status value to return if >= the hi limit
  \return          One of lo_status, hi_status or 0 if inside limits.

   It is assumed that num is no bigger that den and that lo_limit and
   hi_limit are in the range 0-1024, inclusive. It is safe to use -1
   and/or 1025 to effectively disable the checks.
                                                                         */
/* --------------------------------------------------------------------- */    
static __inline int CFC__ratioCheck (int num,      int den,
                                   int lo_limit, int lo_status,
                                   int hi_limit, int hi_status);
/* --------------------------------------------------------------------- */    

    

/* --------------------------------------------------------------------- */
static int CFC__ratioCheck (int num,      int den,
                            int lo_limit, int lo_status,
                            int hi_limit, int hi_status)
{
   int  shf;

   shf = FFS (den) - 0xb;
   
   /*
    | Check to see if have a small enough number to scale by 10 bits.
    | This means at least 11 bits free at the top to stay out of the
    | sign bit. 
   */
   if (shf < 0)
   {
       /* Scale so that there is no overflow */
       num >>= shf;
       den >>= shf;
   }
       

   /* Scale by 10 bits, this gives 1/1024 accuracy */
   num <<= 10;

   if      (num <= lo_limit * den) return lo_status;
   else if (num >= hi_limit * den) return hi_status;


   return 0;
}
/* ---------------------------------------------------------------------- */

    

#ifdef __cplusplus
}
#endif


#endif
