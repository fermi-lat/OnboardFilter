/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   CFC_constantsPrint.c
   \brief  Prints the specified set of CAL calibration constants. This
           is purely a diagnostic routine.

   \author JJRussell - russell@slac.stanford.edu

    These routines exist only for debugging purposes. 
                                                                         */
/* --------------------------------------------------------------------- */


#include <stdio.h>
#include "DFC/CFC_constants.h"



/* ---------------------------------------------------------------------- *//*!
 
  \fn        void CFC_constantsPrint (const struct _CFC_logConstants *gp,
                                      int                          count)
  \brief     Prints the specified number of CAL log constants.
  \param  gp The  array of log constants to print.
  \param cnt The number of log constants to print.
                                                                          */
/* ---------------------------------------------------------------------- */
void CFC_constantsPrint (const struct _CFC_logConstants *gp, int count)
{
    int log;
 
    printf (" Log  GA0   PA0   GA1   PA1   GA2   PA2   GA3   PA3\n"
            " --- ----  ----  ----  ----  ----  ----  ----  ----\n");
    
    for (log = 0; log < count; log++)
    {
        printf (" %3d %5d %5d %5d %5d %5d %5d %5d %5d\n",
                log,
                gp[log].rng[0].gainA,
                gp[log].rng[0].pedestalA,
                gp[log].rng[1].gainA,
                gp[log].rng[1].pedestalA,
                gp[log].rng[2].gainA,
                gp[log].rng[2].pedestalA,
                gp[log].rng[3].gainA,
                gp[log].rng[3].pedestalA);
    }
    
    return;
}
/* ---------------------------------------------------------------------- */
