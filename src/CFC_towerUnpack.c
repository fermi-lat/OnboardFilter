/* ---------------------------------------------------------------------- *//*!
   
   \file   CFC_towerUnpack.c
   \brief  Unpacks the calorimeter record for one tower.
   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $Id
\endverbatim
                                                                         */
/* --------------------------------------------------------------------- */




#include "DFC/CFC_logData.h"
#include "DFC/CFC_constants.h"
#include "DFC/CFC_towerRecord.h"
#include "DFC/CFC_towerUnpack.h"

static __inline int convert (int val, int ped, int gain);


/* ---------------------------------------------------------------------- *//*!

  \fn     int convert (int val, int ped, int gain)
  \brief  Converts ADC value into standard units (Log Energy Units). LEUs
          are the standard unit internally used by the filter code.
  \param val  The ADC value to convert
  \param ped  The ADC pedestal value
  \param gain The ADC gain conversion
  \return     The energy, in LEUs.

   A simple linear conversion is used and a scaling factor is applied to
   convert an ADC value to an energy in LEUs (at one time 1 LEU = 1/4 MEV).
                                                                          */
/* ---------------------------------------------------------------------- */
static __inline int convert (int val, int ped, int gain)
{
   return  ((val - ped) * gain + CFC_K_GAIN_PER_LEU/2) / CFC_K_GAIN_PER_LEU;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn int *CFC_towerUnpack (struct _CFC_towerRecord      *ctr,
                            int                           *dst,
                            const unsigned int           *data,
                            const struct _CFC_logConstants *gp)
  \brief Unpacks the CAL data from 1 tower into a Cal Tower Record (ctr)
  \param  ctr The CTR data structure to fill
  \param  dst The destination array for the CAL energies
  \param data The CAL data
  \param   gp The CAL calibration constants for this tower
  \return     The next CAL energy location to be stored

   The Cal Tower Record contains the control structure to describe
   a fully unpacked and calibrated CAL tower. The calibrated energies
   are stored in a separate array. This allows the control structure
   to be relatively compact and the CAL energies to be contigious.
                                                                          */
/* ---------------------------------------------------------------------- */
int *CFC_towerUnpack (struct _CFC_towerRecord      *ctr,
                     int                           *dst,
                     const unsigned int           *data,
                     const struct _CFC_logConstants *gp)
{
   unsigned int    counts;
   int        towerEnergy;
   int           layerNum;
   int           layerMap;
   CFC_columnMap *colMapP;
   int      *layerEnergyP;


   /*
    |  Get the number of struck logs per layer. These are arranged
    |  1 count per nibble, with layer 0 in the least significant nibble.
   */
   counts         = *data++;
   ctr->logEnergy =  dst;

   
   /*
    | If there are no struck logs, fill out a NULL record. Note that only
    | the scalar fields of the structure are initialized. The array fields
    | must use the appropriate bit map to locate the struck layers/logs.
    | Since the bit maps are empty, there are no valid fields. 
   */
   if (counts == 0)
   {
       ctr->layerMap  = 0;
       ctr->layerCnts = 0;
       ctr->energy    = 0;
       return dst;
   }


   /* There are struck logs in this layer that need to be unpacked           */
   layerMap    = 0;                /* Accumulates bit map of struck layers   */
   layerNum    = 0;                /* Current layer number                   */
   towerEnergy = 0;                /* Accumulates total energy in this tower */


   colMapP        = ctr->colMap;      /* Pointer to current layer column map */
   layerEnergyP   = ctr->layerEnergy; /* Pointer to current layer energy     */
   ctr->layerCnts = counts;           /* Fill in the number of hits per layer*/
   
   
   /* Loop over all the layers */
   do
   {
       int                          nlogs;
       int                    layerEnergy;
       CFC_columnMap               colMap;
       const struct _CFC_logConstants *gpl;
       
       
       /* Extract the number of struck logs on this layer */
       nlogs = (counts & 0xf0000000) >> 28;

       
       /* If none, next layer */
       if (nlogs == 0) continue;

       
       layerEnergy = 0;
       layerMap   |= 0x80000000 >> layerNum;
       colMap      = 0;

       
       do
       {
           CFC_logData           log;           
           const CFC_logConstant *rA;
           const CFC_logConstant *rB;
           int                    eA;
           int                    eB;


           /* Transfer so can easily extract the bit fields                 */ 
           log.ui = *data++; 


           /* Locate the calibration constants for the two log ends         */
           gpl=  gp  + log.bf.col;     /* Index to the proper constants set */
           rA = &gpl->rng[log.bf.rngA];/* Index to get A's gain/ped by range*/
           rB = &gpl->rng[log.bf.rngB];/* Index to get B's gain/ped by range*/

       
           /* Translate the raw ADC value into energy                       */
           eA = convert (log.bf.valA, rA->pedestalA, rA->gainA);
           eB = convert (log.bf.valB, rB->pedestalB, rB->gainB);

           
           /* Store the energy of each log                                  */
           *dst++ = eA; 
           *dst++ = eB;
           
           /*
            | Keep track of the total energy in this layer                  
            | Note that the energy is average of the two ends. The convention
            | is that each log end measures the total energy (ignoring the
            | light sharing correction). Logs with only 1 end of data must
            | be processed specially.
           */
           layerEnergy += (eA + eB) >> 1;
           colMap      |= 0x8000 >> log.bf.col;
       }
       while (--nlogs > 0);

       
       /* End processing layer, store the information about this layer     */
       towerEnergy   += layerEnergy;   /* Accumulate total energy          */
      *layerEnergyP++ = layerEnergy;   /* Store the energy in this layer   */
      *colMapP++      = colMap;        /* Store the col map for this layer */
   }
   while (gp += CFC_K_LOGS_PER_LAYER, layerNum++, counts <<= 4);
   

   ctr->layerMap = layerMap;
   ctr->energy   = towerEnergy;

   
   return dst;
   
}
/* ---------------------------------------------------------------------- */

