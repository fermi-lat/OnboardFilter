/* ---------------------------------------------------------------------- *//*!
   
   \file   CFC_towerRawUnpack.c
   \brief  Unpacks the calorimeter record for one tower. No energy
           conversion is performed.
   \author JJRussell - russell@slac.stanford.edu

   \verbatim

CVS $Id
   \endverbatim   

                                                                         */
/* --------------------------------------------------------------------- */




#include "DFC/CFC_logData.h"
#include "DFC/CFC_towerRawRecord.h"
#include "DFC/CFC_towerRawUnpack.h"


/* ---------------------------------------------------------------------- *//*!

  ]fn unsigned short int *CFC_towerRawUnpack (struct _CFC_towerRawRecord *ctr,
	                                      unigned short int          *phas,
					      const unsigned int         *data)

  \brief Unpacks the CAL data from 1 tower into a Cal Tower Raw Record (ctr)
  \param  ctr The CTR data structure to fill
  \param phas The destination array for the CAL ADC/PHA values
  \param data The CAL data
  \return     The next CAL pha location to be stored

   The Cal Tower Record contains the control structure to describe
   a fully unpacked, but not energy calibrated CAL tower.
                                                                          */
/* ---------------------------------------------------------------------- */
unsigned short int *CFC_towerRawUnpack (struct _CFC_towerRawRecord  *ctr,
					unsigned short int         *phas,
					const unsigned int         *data)
{
   unsigned int         counts;
   int                layerNum;
   int                layerMap;
   unsigned short int *colMapP;
   unsigned short int   colMap;
   unsigned short int *begPhas;
   unsigned char        *ophas;
   

   /*
    |  Get the number of struck logs per layer. These are arranged
    |  1 count per nibble, with layer 0 in the least significant nibble.
   */
   counts    = *data++;
   ophas     = ctr->ophas;
   begPhas   = phas;
   ctr->phas = begPhas;
   

   
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
       ctr->nphas     = 0;
       return phas;
   }


   /* There are struck logs in this layer that need to be unpacked           */
   layerMap    = 0;                /* Accumulates bit map of struck layers   */
   layerNum    = 0;                /* Current layer number                   */


   colMapP        = ctr->colMap;      /* Pointer to current layer column map */
   ctr->layerCnts = counts;           /* Fill in the number of hits per layer*/
   
   
   /* Loop over all the layers */
   do
   {
       int                          nlogs;

       
       
       /* Extract the number of struck logs on this layer */
       colMap = 0;
       nlogs  = (counts & 0xf0000000) >> 28;

       /* If none, next layer */
       if (nlogs == 0) continue;

       layerMap       |= 0x8000 >> layerNum;
       ophas[layerNum] = phas   -  begPhas;
       
       do
       {
           CFC_logData log;           


           /* Transfer so can easily extract the bit fields                 */ 
           log.ui  = *data++; 
          *phas++  = log.cf.phaA;
          *phas++  = log.cf.phaB;
           colMap |= 0x8000 >> log.bf.col;
       }
       while (--nlogs > 0);

       
       /* End processing layer, store the information about this layer     */
   }
   while (colMapP[layerNum] = colMap,
          layerNum++,
          counts <<= 4);

   
   ctr->layerMap = layerMap;
   ctr->nphas    = phas - begPhas;
   

   return phas;
   
}
/* ---------------------------------------------------------------------- */

