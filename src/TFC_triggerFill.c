/* ---------------------------------------------------------------------- *//*!
   
   \file   TFC_triggerFill.c
   \brief  The implementation of the TFC_triggerFill routine. 

   \author JJRussell - russell@slac.stanford.edu

\verbatim

 CVS $Id
\endverbatim 

   The trigger fill routine calculates the 3-in-a-row trigger mask for
   each tower.
                                                                          */
/* ---------------------------------------------------------------------- */


#include "DFC/EBF_directory.h"
#include "DFC/TFC_triggerFill.h"
#include "DFC/EBF_cid.h"
#include "TFC_triggerForm.h"



/* ---------------------------------------------------------------------- *//*!

  \fn int TFC_triggerFill (const struct _EBF_directory *dir, int tkr_trg[16])
  \brief  Fills the tracker trigger information
  \param  dir     The event directory
  \param  tkr_trg The array of tracker tower triggers
  \return         The upper 16 bits contain a mask of the towers
                  with a 6/6 or 7/8 trigger, the lower 16 bits
                  contain a mask of the towers with only a 4/4
                  possibility.

   This routine will fill in the tracker tower trigger mask for each of
   the 16 towers. 
                                                                          */
/* ---------------------------------------------------------------------- */
int TFC_triggerFill (const struct _EBF_directory *dir,
                     int                  tkr_trg[16])
{
   const EBF_contributor *contributors;
   int                           tcids;
   int                            cids;
   int                             cid;


   tcids        = 0;
   cids         = EBF_DIRECTORY_TWRS_TKR (dir->ctids);
   contributors = dir->contributors;

   
   /* Loop over all towers that may have tracker data */
   for (cid = EBF_K_CID_TWR; cid < EBF_K_CID_TWR + 16; cid++, cids <<= 1)
   {
       /* If this tower had data... */
       if (cids < 0)
       {
           const EBF_contributor *contributor;           
           const EBF_tkr                 *tkr;
           unsigned int              xcapture;
           unsigned int              ycapture;
           unsigned int                    xy;


           /* Point to this contributors CAL/TKR data */
           contributor = &contributors[cid];
           tkr         = EBF__tkrLocate (contributor->ptr,
                                         contributor->calcnt);

           /*
            | Extract the x & y track hit maps, the XY layer maps are
            | separated two ends of each layer are OR'd together.
           */
           EBF_TKR_ACCEPTS_RIGHT_ORED_UNPACK (tkr->accepts, xcapture,ycapture);
           
           /* Keep track of the XY coincidences */
           xy = xcapture & ycapture;
#ifdef GLEAM
           TDS_layers[cid-EBF_K_CID_TWR]=xy;
#endif

           /* Anything interesting ? */
           if (xcapture & ycapture)
           {
               TFC_trigger t;

               /*
                | Form the trigger and record which towers had 6/6 or
                | 7/8 coincidences in the upper 16 bits of a bit list
                | and the towers with a 4/4 in the lower bits.
               */                  
               t.si = TFC__triggerForm (xcapture, ycapture);
           
               if (t.si < 0)
               {
                   tcids |= 0x80000000 >> cid;
               }
               else if (t.bf.type == TFC_K_TRIGGER_TYPE_4_OF_4)
               {
                   tcids |= 0x8000 >> cid;
               }
               
               tkr_trg[cid - EBF_K_CID_TWR] = EBF_TKR_RIGHT_ACCEPT_REMAP(xy);
               continue;
           }
       }

       
       /* Either no data at all in this tower or no X/Y coincidences */
       tkr_trg[cid - EBF_K_CID_TWR] = 0;
   }

   return tcids;
}
/* ---------------------------------------------------------------------- */
  
