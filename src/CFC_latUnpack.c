/* ---------------------------------------------------------------------- *//*!
   
   \file  CFC_latUnpack.c
   \brief Unpacks the CAL data for the whole LAT
   \author JJRussell - russell@slac.stanford.edu

\verbatim

  CVS $Id
\endverbatim  
   
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/EBF_directory.h"
#include "DFC/EBF_cid.h"

#include "DFC/CFC_latUnpack.h"
#include "DFC/CFC_latRecord.h"
#include "DFC/CFC_constants.h"
#include "DFC/CFC_towerUnpack.h"



/* ---------------------------------------------------------------------- *//*!

  \fn int *CFC_latUnpack (struct _CFC_latRecord                    *clr,
                          const struct _EBF_directory              *dir,
                          const struct _CFC_logConstants *cal_constants)
                          
 \brief Unpacks the CAL data for the whole LAT
  \param            clr The CAL LAT Record data structure to fill
  \param            dir The directory structure for this event; used to
                        locate the CAL data.
  \param  cal_constants The CAL calibration constants (gains/pedestals)
                        for this tower
  \return               The next CAL energy location to be stored

   The Cal LAT Record contains the control structure to describe
   a fully unpacked and calibrated CAL data for the entire LAT
   tower. 
                                                                          */
/* ---------------------------------------------------------------------- */
int *CFC_latUnpack (struct       _CFC_latRecord              *clr,
                    const struct _EBF_directory              *dir,
                    const struct _CFC_logConstants *cal_constants)
{
   int                 energy; /* Accumulates the total energy in this event*/
   int                     e0; /* Accumulates the energy in layer 0         */
   int                  tower; /* Current tower number                      */
   CFC_towerRecord       *ctr; /* Pointer to the current CAL tower record   */
   int                 twrMap; /* Accumulates towers that have data         */
   int                   *dst; /* Output location for log energies          */
   const EBF_contributor *src; /* Data contributor source                   */
   
    
   energy      = 0;
   tower       = 0;
   e0          = 0;
   ctr         = clr->twr;
   dst         = clr->logEnds;
   src         = dir->contributors + EBF_K_CID_TWR;
   twrMap      = EBF_DIRECTORY_TWRS_CAL (dir->ctids);
   clr->twrMap = twrMap;
   
       
   do
   {
      /*
       | Fill the CAL tower record iff this tower has some CAL data
       | The CAL count includes the CAL count field in the event builder
       | data. This is always there, so must subtract 1 to see if there
       | is any data.
       */
      if (twrMap < 0)
      {
         int             layerMap;
         const unsigned int *data;
            
         data     = EBF__calLocate (src->ptr);
         dst      = CFC_towerUnpack (ctr, dst, data, cal_constants);
         energy  += ctr->energy;
         layerMap = ctr->layerMap;
            
         /* Accumulate the energy seen in layer 0 */
         if (layerMap < 0) e0 += ctr->layerEnergy[0];
      }
   }
   while (ctr++, src++, tower++, twrMap <<= 1);


   /* Fill in the global numbers for this event */
   clr->layerEnergy[0] = e0;
   clr->energy         = energy;
   clr->logCnt         = (dst - clr->logEnds) / 2;
    
   // printf ("L0 energy /EOT = %d/%d\n", clr->layerEnergy[0], clr->energy);

    
   return dst;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn     int CFC_latUnpackSizeof (void)
  \brief  Returns the size, in bytes, of a CAL LAT record.
  \return The size, in bytes, of a CAL LAT record.

   This function should be called to determine the size of a \a clr. This
   allows the calling program to avoid including the structure definition
   of a \a clr.

   After allocating a \a tlr, the structure should be initialized using
   \a TFC_latInit().
                                                                          */
/* ---------------------------------------------------------------------- */
int CFC_latUnpackSizeof (void)
{
   return sizeof (struct _CFC_latRecord);
}
/* ---------------------------------------------------------------------- */


