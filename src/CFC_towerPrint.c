/* ---------------------------------------------------------------------- *//*!
   
   \file   CFC_towerPrint.c
   \brief  Interface to the CAL tower printing routines
   \author JJRussell - russell@slac.stanford.edu

\verbatim

CVS $Id
\endverbatim

    These routines exist for diagnostic reasons only.
                                                                          */
/* ---------------------------------------------------------------------- */


#include <stdio.h>
#include "DFC/CFC_towerPrint.h"
#include "DFC/CFC_logData.h"
    

/* ---------------------------------------------------------------------- *//*!

  \fn    void CFC_towerPrint (const unsigned int *data,
                              int                tower)
  \brief        Produces an ASCII display of the CAL data for the specified
                tower
  \param data   The array of 32-bit CAL data words
  \param tower  The tower number of this data
                                                                          */
/* ---------------------------------------------------------------------- */
void CFC_towerPrint (const unsigned int *data, int tower)
{
   int          layer;
   unsigned int counts;
   
 
   printf ("\n"
           "CAL data for tower %1x\n"
           "  Col RngA ValueA RngB ValueB     Data\n"
           "  --- ---- ------ ---- ------ --------\n", tower);

   layer  = 0;
   counts = *data++;
   do
   {
       int idx;
       int count = (counts >> 28) & 0xf;
       
       for (idx = 0; idx < count; idx++)
       {
          CFC_logData log;

          log.ui    = *data;

          printf (" %1d.%2d %4d %6d %4d %6d %8.8x\n",
                  layer,
                  log.bf.col,
                  log.bf.rngA,
                  log.bf.valA,
                  log.bf.rngB,
                  log.bf.valB,
                  *data);
          data++;
          
       }
               
       layer++;
   }
   while (counts <<= 4);
   

   return;
   
}
/* ---------------------------------------------------------------------- */



