/* ---------------------------------------------------------------------- *//*!
   
   \file  CFC_latRawUnpack.c
   \brief Unpacks the CAL data for the whole LAT, no energy calibration
          is performed
   \author JJRussell - russell@slac.stanford.edu

\verbatim

  CVS $Id
\endverbatim  
   
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/EBF_directory.h"
#include "DFC/EBF_cid.h"

#include "DFC/CFC_latRawUnpack.h"
#include "DFC/CFC_latRawRecord.h"
#include "DFC/CFC_towerRawUnpack.h"




/* ---------------------------------------------------------------------- *//*!

  \fn unsigned short int *CFC_latRawUnpack (struct _CFC_latRawRecord *clr,
                          const struct _EBF_directory                *dir)
                          
 \brief Unpacks the CAL data for the whole LAT
  \param            clr The CAL LAT Record data structure to fill
  \param            dir The directory structure for this event; used to
                        locate the CAL data.
  \return               The next CAL pha location to be stored

   The Cal LAT Record contains the control structure to describe
   a fully unpacked and but non-calibrated CAL data for the entire
   LAT tower. 
                                                                          */
/* ---------------------------------------------------------------------- */
unsigned short int  *CFC_latRawUnpack (struct       _CFC_latRawRecord *clr,
                                       const struct _EBF_directory    *dir)
{
   int                  tower; /* Current tower number                      */
   CFC_towerRawRecord    *ctr; /* Pointer to the current CAL tower record   */
   int                 twrMap; /* Accumulates towers that have data         */
   unsigned short int   *phas; /* Output location for pha values            */
   const EBF_contributor *twr; /* Data contributor source                   */
   
    
   tower       = 0;
   ctr         = clr->twrs;
   phas        = clr->phas;
   twr         = dir->contributors + EBF_K_CID_TWR;
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
         const unsigned int *data;
            
         data     = EBF__calLocate (twr->ptr);
         phas     = CFC_towerRawUnpack (ctr, phas, data);
      }
   }
   while (ctr++, twr++, tower++, twrMap <<= 1);


   /* Fill in the global numbers for this event */
   clr->logCnt         = (phas - clr->phas);
    
    
   return phas; 
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn     int CFC_latRawUnpackSizeof (void)
  \brief  Returns the size, in bytes, of a CAL LAT RAW record.
  \return The size, in bytes, of a CAL LAT RAW record.

   This function should be called to determine the size of a \a clr. This
   allows the calling program to avoid including the structure definition
   of a \a clr.

   After allocating a \a tlr, the structure should be initialized using
   \a CFC_latRawInit().
                                                                          */
/* ---------------------------------------------------------------------- */
int CFC_latRawUnpackSizeof (void)
{
   return sizeof (struct _CFC_latRawRecord);
}
/* ---------------------------------------------------------------------- */


