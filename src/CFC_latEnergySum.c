/* ---------------------------------------------------------------------- *//*!
   
   \file   CFC_latEnergySum.c
   \brief  Does a simple sum of the total CAL energy in the CAL
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
#include "DFC/CFC_latEnergySum.h"
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
     | the two pieces do not equal the sum. For the time-being am giving
     | up one bit of accuracy.
     |
     | The convention is that each log end measures the total energy
     | (that is up to the sharing correction), so must divide number
     | by 2. Logs that have only one end operational must be treated
     | specially.
    */
    energy = ((log.bf.valA - rA->pedestalA) * rA->gainA
           +  (log.bf.valB - rB->pedestalB) * rB->gainB)
           / CFC_K_GAIN_PER_LEU / 2;


    return energy;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!
 
  \fn    int CFC_latEnergySum (const struct _EBF_directory   *dir,
                               const struct _CFC_logConstants *gp)
  \brief            Sums the energy in each struck CAL log in the LAT
  \param       dir  The event directory
  \param        gp  The array of gains and pedestals for the CAL 
  \return           The amount the total energy in the LAT in LEUs
                                                                          */
/* ---------------------------------------------------------------------- */
int CFC_latEnergySum (const struct _EBF_directory    *dir,
                      const struct _CFC_logConstants  *gp)
{
   int                            cids;
   int                           tower;
   int                          energy;
   const EBF_contributor *contributors;

   
   /* Get the map of which CAL towers have data, return if none */
   cids = EBF_DIRECTORY_TWRS_CAL(dir->ctids);
   if (cids == 0) return 0;

   
   contributors = dir->contributors;
   tower        = 0;
   energy       = 0;

   
   /* Loop over all towers that have data */
   do
   {
       unsigned int                      counts;
       const unsigned int                 *data;
       const EBF_contributor       *contributor;
       int                                  sum;
       const struct _CFC_logConstants  *gpLayer;
       
       
       tower       = FFS (cids);
       contributor = contributors + tower;
       gpLayer     = gp + tower * CFC_K_LOGS_PER_TOWER;
       cids        = FFS_eliminate (cids, tower);
       data        = EBF__calLocate (contributor->ptr);
       counts      = *data++;
       sum         =  0;
       

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
               
               sum += e0 = getEnergy (*data++, gpLayer);
               
              ODD:
               sum += e1 = getEnergy (*data++, gpLayer);
               
           }
           while ((count -= 2) > 0);
       }
       while (gpLayer += CFC_K_LOGS_PER_LAYER, counts >>= 4);

       /*
        | The convention is that each log end measures the total energy
        | (that is up to the sharing correction), so must divide this number
        | by 2. Logs that have only one end operational must be treated
        | specially. The conversion factor takes calibration units to LAT
        | energy units.
       */
       energy += sum;
   }
   while (cids);
   

   return energy;
}
/* ---------------------------------------------------------------------- */


        


/* ---------------------------------------------------------------------- *//*!
 
  \fn    int CFC_latEnergySumEvt (const unsigned int            *cur,
                                  int                           left,
                                  const struct _CFC_logConstants *gp,
                                  int                       *elayers)
  \brief            Sums the energy in each struck CAL log in the LAT
  \param       cur  Pointer to the current contributor's trailer word
  \param      left  Number of words left in the event.
  \param        gp  The array of gains and pedestals for the CAL
  \param   elayers  Array filled in with the layer-by-layer energy sum
  \return           The total energy in the calorimeter
                                                                          */
/* ---------------------------------------------------------------------- */
int CFC_latEnergySumEvt (const unsigned int             *cur,
                         int                            left,
                         const struct _CFC_logConstants  *gp,
                         int                        *elayers)
{

   int energy;

   energy = 0;
   elayers[0] = elayers[1] = elayers[2] = elayers[3] = 0;
   elayers[4] = elayers[5] = elayers[6] = elayers[7] = 0;   
   
   /* Loop over the contributors */ 
   do
   {
       int                              len;
       unsigned int                   tower;
       unsigned int                hdr_clen;
       const unsigned int             *data;

       
       /* Extract the length of this contributor in 32-bit words */
       hdr_clen = cur[0];
       len      = hdr_clen & 0xffff;
       left    -= len;

       /*
        | Skip to, then over 2 header words and point at the CAL counts
        | Advance the current contributor pointer to the next one
       */
       data  = cur + 2;
       cur   = (const unsigned int *)((unsigned char *)cur + len);
       tower = ((hdr_clen >> 17) & 0x1f) - EBF_K_CID_TWR;
       

       /* Is this a tower contributor */
       if (_EBF_CID_IS_TOWER (tower))
       {

           unsigned int                      counts;
           int                                  sum;
           int                                 esum;
           const struct _CFC_logConstants  *gpLayer;
           int                              *elayer;

           
           /* Get the word with the 1 struck log count/nibble */
           counts = *data++;

           
           /* If no struck logs, onward to the next contributor */
           if (counts == 0) continue;
           

           /* Beginning of gains/pedestals for layer 0, this tower */
           gpLayer = gp + tower * CFC_K_LOGS_PER_TOWER;
           sum     = 0;
           elayer  = elayers;

           
           /* Loop over the layers */
           do
           {
               int count;
           
               /* Extract the number of logs with energy on this layer */
               count = (counts & 0xf0000000) >> 28;
               if (count ==  0) continue;
               esum = 0;               
               /*
                | The sum will be done in pairs, so get rid of any odd piece,
                | then compute the number of log pairs and sum 2 at a time.
                | I've tried unrolling the loop completely with no noticable
                | improvement in the timing.
               */
               if (count & 1) { count++;  goto ODD; }
           
               do
               {
                      esum += getEnergy (*data++, gpLayer);
                 ODD: esum += getEnergy (*data++, gpLayer);
               }
               while ((count -= 2) > 0);

               sum     += esum;
               *elayer += esum;

           }
           while (elayer++, gpLayer += CFC_K_LOGS_PER_LAYER, counts <<= 4);

           energy += sum;
           
       }
   }
   while (left > 0);
   

   return energy;
}
/* ---------------------------------------------------------------------- */



