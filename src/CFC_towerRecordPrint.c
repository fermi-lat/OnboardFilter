/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   CFC_towerRecordPrint.c
   \brief  Implementation of the CAL tower print routine
   \author JJRussell - russell@slac.stanford.edu

                                                                         */
/* --------------------------------------------------------------------- */


#include <stdio.h>
#include "DFC/CFC_constants.h"
#include "DFC/CFC_towerRecord.h"
#include "DFC/CFC_towerRecordPrint.h"



/* --------------------------------------------------------------------- *//*!

  \def    MEV(_energy_in_leus)
  \brief  Picks off the whole number of MEVS
                                                                         *//*!
  \def    FRACTION(_energy_in_leus)
  \brief  Picks off the fraction of MEVS (100rths)

   The macros MEV and FRACTION are used to print a pseudo floating point
   representation of a CAL energy, i.e. something like EEE.ee
                                                                         */
/* --------------------------------------------------------------------- */
#define MEV(_energy)       (_energy/CFC_K_LEU_PER_MEV)
#define FRACTION(_energy)  \
        ((_energy & (CFC_K_LEU_PER_MEV - 1)) * (100/CFC_K_LEU_PER_MEV))
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \fn  void CFC_towerRecordPrint (const struct _CFC_towerRecord *ctr,
                                  int                          tower)
  \brief Prints an ASCII formatted dump of a CTR
  \param   ctr The CTR to display
  \param tower The tower number of the CTR
                                                                         */
/* --------------------------------------------------------------------- */
void CFC_towerRecordPrint (const struct _CFC_towerRecord *ctr, int tower)
{
   int                    layer;
   int                 layerMap;
   const CFC_columnMap *colMapP;
   const int            *energy;
   const int      *layerEnergyP;
   
   
   printf (
     "\n"
   "%1x  Col     EnergyA     EnergyB       Total LayerEnergy TowerEnergy\n"
     "  -.-- ----------- ----------- ----------- ----------- -----------\n",
     tower);
   
   layer        = 0;
   layerMap     = ctr->layerMap;
   colMapP      = ctr->colMap;
   energy       = ctr->logEnergy;
   layerEnergyP = ctr->layerEnergy;
   

   /* Loop over the struck layers */
   while (1)
   {
       int    colMap;
       int    column;
       char layerNum;

       
       /* MSB (sign bit) indicates whether this tower has something in it */
       if (layerMap > 0)
       {
           layerMap <<= 1;
           layer++;
           continue;
       }
       

       /* Tower has something in it, pick up the column map */
       column   = 0;
       colMap   = *colMapP++ << 16;
       layerNum = 0x30 | layer;

       /* Loop over the struck logs */
       while (1)
       {
           /* Was this column struck? */
           if (colMap < 0)
           {
               int eA = energy[0];
               int eB = energy[1];
               int eT = (eA + eB) / 2;
           
               printf ("  %c.%2d %8d.%2.2d %8d.%2.2d %8d.%2.2d",
                       layerNum,
                       column,
                       MEV(eA), FRACTION(eA),
                       MEV(eB), FRACTION(eB),
                       MEV(eT), FRACTION(eT));

               layerNum = ' ';
               energy  += 2;


               if (colMap <<= 1)
               {
                   /* If have not exhausted the logs in this column... */
                   printf ("\n");
               }
               else
               {
                   /* If no logs left on this layer, print the layer energy */
                   int eL = *layerEnergyP++;
                   printf (" %8d.%2.2d", MEV(eL), FRACTION(eL));
                   

                   /* If out of layers, print total energy in this tower */
                   if ((layerMap <<= 1) == 0)
                   {
                       printf (" %8d.%2.2d\n",
                               MEV(ctr->energy), FRACTION(ctr->energy));
                       return;
                   }

                   /* Still more layer, go on to the next layer */
                   printf ("\n");
                   layer++;
                   break;

               }
           }
           else if ((colMap <<= 1) == 0) break;
           column++;
       }
           
   }


   
   return;
}

