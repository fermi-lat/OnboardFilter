/* ---------------------------------------------------------------------- *//*!
   
   \file   CFC_latThreshold.c
   \brief  Determines whether the energy in the CAL exceeds the specified
           threshold.
   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $Id
\endverbatim 
                                                                         */
/* --------------------------------------------------------------------- */


#include "DFC/EBF_cid.h"
#include "DFC/EBF_directory.h"
#include "DFC/CFC_logData.h"
#include "DFC/CFC_constants.h"
#include "DFC/CFC_latThreshold.h"
#include "ffs.h"

#ifdef __cplusplus
extern "C" {
#endif    

    
static __inline int getEnergy (unsigned int data, const CFC_logConstants *gp);

    
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
static __inline int getEnergy (unsigned int data, const CFC_logConstants *gp)
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

    /*
     | The convention is that each log end measures the total energy
     | (that is up to the sharing correction), so must divide this number
     | by 2. Logs that have only one end operational must be treated
     | specially.
    */
    energy >>= 1;

    return energy;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!
 
  \fn    int CFC_latThreshold (const struct _EBF_directory   *dir,
                               const struct _CFC_logConstants *gp,
                               int                      threshold)
  \brief            Sums the energy in each struck CAL log in the LAT
                    until the specified threshold is exceede
  \param       dir  The event directory
  \param        gp  The array of gains and pedestals for the CAL 
  \param threshold  The threshold, in LEU's
  \return           The amount the total energy exceeds the threshold.
                    If this number is negative, the threshold was
                    not exceeded, if 0, the threshold was exactly
                    met, it positive, then the threshold was exceeded,
                    Although in this case, one cannot say by how
                    much since the summing is stopped as soon as the
                    threshold is exceeded or met.
                                                                          */
/* ---------------------------------------------------------------------- */
int CFC_latThreshold (const struct _EBF_directory    *dir,
                      const struct _CFC_logConstants  *gp,
                      int                       threshold)
{
   int                            cids;
   int                           tower;
   int                          energy;
   const EBF_contributor *contributors;

   /*
    | Initialize the energy sum at -threshold, this allows the threshold
    | check to be a simple check of crossing from negative to positive.
   */
   energy       = -threshold;
   
   /* Get the map of which CAL towers have data, return if none */
   cids = EBF_DIRECTORY_TWRS_CAL(dir->ctids);
   if (cids == 0) return energy;

   
   contributors = dir->contributors;
   tower        = 0;


   
   /* Loop over all towers that have data */
   do
   {
       unsigned int                     counts;
       const unsigned int                *data;
       const EBF_contributor      *contributor;
       const struct _CFC_logConstants *gpLayer;
       
       
       tower       = FFS (cids);
       contributor = contributors + tower;
       gpLayer     = gp + CFC_K_LOGS_PER_TOWER * tower;
       cids        = FFS_eliminate (cids, tower);
       data        = EBF__calLocate (contributor->ptr);
       counts      = *data++;

       /* Loop over the layers */
       do
       {
           int count;
           
           /* Extract the number of logs with energy on this layer */
           count = counts & 0xf;
           if (count == 0) continue;

           /*
            | The sum will be done in pairs, so get rid of any odd piece,
            | then compute the number of log pairs and sum 2 at a time
           */
           if (count & 1) { count++;  goto ODD; }
           
           do
           {
               int e0, e1;
               
               energy += e0 = getEnergy (*data++, gpLayer);
               
              ODD:
               energy += e1 = getEnergy (*data++, gpLayer);
               
               if (energy >= 0) return energy;
           }
           while ((count -= 2) > 0);


       }
       while (gpLayer += CFC_K_LOGS_PER_LAYER, counts >>= 4);
       
   }
   while (cids);
   

   return energy;
}
/* ---------------------------------------------------------------------- */


        


/* ---------------------------------------------------------------------- *//*!
 
  \fn    int CFC_latThresholdEvt (const unsigned int            *cur,
                                  int                           left,
                                  const struct _CFC_logConstants *gp,
                                  int                      threshold)
  \brief            Sums the energy in each struck CAL log in the LAT
                    until the specified threshold is exceede
  \param       cur  Pointer to the current contributor's trailer word
  \param      left  Number of words left in the event.
  \param        gp  The array of gains and pedestals for the CAL 
  \param threshold  The threshold, in LEU's
  \return           The amount the total energy exceeds the threshold.
                    If this number is negative, the threshold was
                    not exceeded, if 0, the threshold was exactly
                    met, it positive, then the threshold was exceeded,
                    Although in this case, one cannot say by how
                    much since the summing is stopped as soon as the
                    threshold is exceeded or met.
                                                                          */
/* ---------------------------------------------------------------------- */
int CFC_latThresholdEvt (const unsigned int             *cur,
                         int                            left,
                         const struct _CFC_logConstants  *gp,
                         int                       threshold)
{

   int energy;

   /*
    | Initialize the energy sum at -threshold, this allows the threshold
    | check to be a simple check of crossing from negative to positive.
   */
   energy = -threshold;
   
   /* Loop over the contributors */ 
   do
   {
       int            len;
       unsigned int tower;

       /* Extract the length of this contributor in 32-bit words */
       len   = *cur + 1;
       left -= len;
       cur  -= len;
       tower =  ((cur[1] >> 17) &0x1f) - EBF_K_CID_TWR;

       /* Is this a tower contributor */
       if (tower < 16)
       {
           const unsigned int                *data;
           unsigned int                     counts;
           const struct _CFC_logConstants *gpLayer;           

           /* SKip to, then over 2 header words and point at the CAL counts */
           data    = cur + 1 + 2;
           counts  = *data++;
           gpLayer = gp + CFC_K_LOGS_PER_TOWER * tower;

           
           /* Loop over the layers */
           do
           {
               int                               count;

               
           
               /* Extract the number of logs with energy on this layer */
               count = counts & 0xf;
               if (count == 0) continue;

               /*
                | The sum will be done in pairs, so get rid of any odd piece,
                | then compute the number of log pairs and sum 2 at a time
               */
               if (count & 1) { count++;  goto ODD; }
           
               do
               {
                      energy += getEnergy (*data++, gpLayer);
                 ODD: energy += getEnergy (*data++, gpLayer);

                      if (energy >= 0) return energy;
               }
               while ((count -= 2) > 0);
           }
           while (gpLayer += CFC_K_LOGS_PER_LAYER, counts >>= 4);
       }
   }
   while (left > 0);
   

   return energy;
}
/* ---------------------------------------------------------------------- */



