#ifndef CFC_XTALRATIOCHECK_H
#define CFC_XTALRATIOCHECK_H




/* ---------------------------------------------------------------------- *//*!
   
   \file  CFC_xtalRatioCheck.h    
   \brief Checks that no more than the specified percentage of tiles have
          greater than the specified percentage of energy.
          limits.
   \author JJRussell - russell@slac.stanford.edu

    This function checks a specified percentage of tiles (typically 20%)
    have more than a specified percentage of the total energy (typically
    1%).

\verbatim
    CVS $Id
\endverbatim
                                                                          */
/* ---------------------------------------------------------------------- */

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


/* --------------------------------------------------------------------- *//*!
    
  \fn  unsigned int CFC__xtalRatioCheck (int          energy,
                                         int          logCnt,
                                         const int  *logEnds,
					 int         pEnergy,
					 int            pCnt,
					 unsigned int status)

  \brief    Checks that no more than a specified percentage of logs 
            \a pCnt, have less than pEnergy (a percentage of the energy)
  \return   0, if the condition is no true, else \a status;

  \param energy    The total enery in this event
  \param logCnt    The total number of struck logs in this event
  \param logEnds   Pointer to the array of log end energies
  \param pEnergy   The energy percentage. This is actually 1/percentage
                   allowing this to be a multiplier. 
  \param pCnt      The percentage of logs, expressed in units of 1024.
  \param status    The status value to return if the condition is true


   Typical parameters for \a pEnergy are taken to be 128. This is 
   approximately 1% (1/128) and avoids a multiply and \a pCnt is
   taken to be 200, approximating 20% (200/1024).
									 */
/* --------------------------------------------------------------------- */    
static __inline int CFC__xtalRatioCheck (int          energy,
				       int          logCnt,
				       const int  *logEnds,
				       int         pEnergy,
				       int            pCnt,
				       unsigned int status);
/* --------------------------------------------------------------------- */    

    


/* --------------------------------------------------------------------- */
static __inline int CFC__xtalRatioCheck (int          energy,
				       int          logCnt,
				       const int  *logEnds,
				       int         pEnergy,
				       int            pCnt,
				       unsigned int status)
{
  int         eneed;
  const int *logLst;
  const int *logCur;
           
       
  /*
   | To avoid divisions, 20% is approximated by 200/1024
   | and                  1% is approximated by 1/128.
   |
   | These are somewhat arbitrary numbers, so this is not deemed
   | to be significant.
  */
       
  /* Calculate the number of logs needed to exceed the pCnt/1024 number */
  eneed  = logCnt * pCnt / 1024;

       
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
  logCur = logEnds;       
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
    if ((e*pEnergy + pEnergy/2) > energy && (--eneed <= 0)) return 0;
    
  ODD:
    e = logCur[2] + logCur[3];
    if ((e*pEnergy + pEnergy/2) > energy && (--eneed <= 0)) return 0;
           
  }
  while ((logCur += 4) < logLst);

  return status;
}
/* ---------------------------------------------------------------------- */
    

#ifdef __cplusplus
}
#endif


#endif
