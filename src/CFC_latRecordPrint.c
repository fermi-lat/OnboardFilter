/* ---------------------------------------------------------------------- *//*!
   
   \file   CFC_latRecordPrint.c
   \brief  Defines the implementaion of the CAL LAT record print routine.
   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $Id
\endverbatim
                                                                         */
/* --------------------------------------------------------------------- */


#include <stdio.h>
#include "ffs.h"
#include "DFC/CFC_latRecord.h"
#include "DFC/CFC_latRecordPrint.h"
#include "DFC/CFC_constants.h"
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




/* ---------------------------------------------------------------------- *//*!

  \fn    void CFC_latRecordPrint (const struct _CFC_latRecord *clr,
                                  int                        which)
  \brief        Produces an ASCII display of the CAL data.
  \param clr    A previously unpacked CAL LAT Record.
  \param which  A left justified mask of which towers to print
                (MSB = TOWER 0)
                                                                          */
/* ---------------------------------------------------------------------- */
void CFC_latRecordPrint (const struct _CFC_latRecord *clr, int which)
{
   int map;


   /*
    | Get the map of which towers to print. This is limited to the union
    | of the towers that have data and those that the user wishes to
    | display.
   */
   map = clr->twrMap;
   

   if (map == 0)
   {
       printf ("CAL:   Nothing to display\n"
               "Towers with  data: %8.8x\n"
               "Towers to display: %8.8x\n",
               clr->twrMap,
               which);
       
       return;
   }

   
   do
   {
       int                  tower;
       const CFC_towerRecord *ctr;

       tower = FFS (map);
       ctr   = clr->twr + tower;
       map   = FFS_eliminate (map, tower);

       CFC_towerRecordPrint (ctr, tower);
   }
   while (map);

   
   {
       int energy = clr->energy;
       int    el0 = clr->layerEnergy[0];
       
       printf ("Total Energy = %8d.%2.2d (%10d)\n"
               "        E[0] = %8d.%2.2d (%10d)\n",
               MEV(energy), FRACTION(energy),  energy,
               MEV(el0), FRACTION(el0), el0);
   }
   

   return;
}
/* ---------------------------------------------------------------------- */
       
       
