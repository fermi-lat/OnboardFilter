/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   CFC_towerSum.c
   \brief  Sums up the energy in a calorimeter tower on a layer by layer
           basis and, also returns the total energy in this tower.
   \author JJRussell - russell@slac.stanford.edu

                                                                         */
/* --------------------------------------------------------------------- */


#include "DFC/CFC_logData.h"
#include "DFC/CFC_constants.h"
#include "DFC/CFC_towerSum.h"

#include "windowsCompat.h"

#ifdef __cplusplus
extern "C" {
#endif    

    
static inline int getEnergy (unsigned int data, const CFC_logConstants *gp);

    
#ifdef __cplusplus
}
#endif    


    
/* ---------------------------------------------------------------------- *//*!

  \fn         int getEnergy (unsigned int data, const CFC_logConstants *gp)
  \brief      Converts the calorimeter data word in a energy.
  \param data The calorimeter data word for one log.
  \param   gp The calorimeter constants for this layer.
  \return     The energy of both ends.
                                                                          */
/* ---------------------------------------------------------------------- */  
static inline int getEnergy (unsigned int data, const CFC_logConstants *gp)
{
    CFC_logData            log;
    const CFC_logConstant  *rA;
    const CFC_logConstant  *rB;
    int                 energy;
    
    
    log.ui = data;                  /* Get so can extract bit fields      */
    gp     =  gp +  log.bf.col;     /* Index to the proper constants      */
    rA     = &gp->rng[log.bf.rngA]; /* Index to get A's gain/ped by range */
    rB     = &gp->rng[log.bf.rngB]; /* Index to get B's gain/ped by range */

    /*
     | There is a rounding problem to contend with. The answer one gets
     | is different depending on whether the / CFC_K_GAIN_PER_LEU is
     | done on each energy separately or one the sum. The more accurate
     | way is to do it on the sum, but this leaves the problem that
     | the two pieces do equal the sum. For the time-being am giving
     | up one bit of accuracy.
    */
    energy = (log.bf.valA - rA->pedestalA) * rA->gainA  / CFC_K_GAIN_PER_LEU
           + (log.bf.valB - rB->pedestalB) * rB->gainB  / CFC_K_GAIN_PER_LEU;

    return energy;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!
 
  \fn    int CFC_towerSum (int                        sums[8],
                           const unsigned int           *data,
                           const struct _CFC_logConstants *gp)
  \brief          Sums the energy in each struck CAL log in a tower and
                  returns both a layer-by-layer sum and a total tower sum.
  \brief sums[8]  An array of 8 sums in which to accumulate the
                  layer-by-layer energy sums
  \brief      gp  The array of gains and pedestals for this tower.
  \return         The total energy in this tower.
                                                                          */
/* ---------------------------------------------------------------------- */
int CFC_towerSum (int                         sums[8],
                  const unsigned int            *data,
                  const struct _CFC_logConstants  *gp)
{
   unsigned int counts;
   int             tot;

   /*
    | Extract the counts for each layer, these are stored 1 per nibble
    | with the least significant nibble representing column 0
   */
   tot    = 0;
   counts = *data++;
   do
   {
       int count;
       //int   sum;
       

       /* Extract the number of logs with energy on this layer */
       count = counts & 0xf;

       /* If none, next layer */
       if (count == 0) continue;

       /* Get the current amount of energy for this layer */
       //sum = *sums;

       /*
        | The sum will be done in pairs, so get rid of any odd piece,
        | then compute the number of log pairs and sum 2 at a time
       */
       if (count & 1) tot += getEnergy (*data++, gp);
       count >>= 1;
       while (--count >= 0)
       {
           int eA, eB;
           
           tot += eA = getEnergy (*data++, gp); 
           tot += eB = getEnergy (*data++, gp); 
       }


//       tot += sum;
//      *sums = sum;
       
       

#if 0       
       switch (count)
       {
           case 12: sum += getEnergy (*data++, gp);
           case 11: sum += getEnergy (*data++, gp);
           case 10: sum += getEnergy (*data++, gp);
           case  9: sum += getEnergy (*data++, gp);
           case  8: sum += getEnergy (*data++, gp);
           case  7: sum += getEnergy (*data++, gp);
           case  6: sum += getEnergy (*data++, gp);
           case  5: sum += getEnergy (*data++, gp);
           case  4: sum += getEnergy (*data++, gp);               
           case  3: sum += getEnergy (*data++, gp);               
           case  2: sum += getEnergy (*data++, gp);               
           case  1: sum += getEnergy (*data++, gp);
                   *sums = sum;
                   tot  += sum;
           case  0:
       }
#  endif       
       
   }
   while (gp += CFC_K_LOGS_PER_LAYER, sums++, counts >>= 4);

   return tot;
}
/* ---------------------------------------------------------------------- */


        
