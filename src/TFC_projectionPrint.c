/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   TFC_projectionPrint.c
   \brief  Defines the implementation for the projection printing routines.
           These routines are for diagnostic and debugging purposes only.
   \author JJRussell - russell@slac.stanford.edu

                                                                         */
/* --------------------------------------------------------------------- */


#include <stdio.h>
#include "TFC_projectionDef.h"



static void printProjections (const TFC_projection *prj,
                              int                  xCnt,
                              int                  yCnt,
                              int               towerId);

/* ---------------------------------------------------------------------- *//*!

  \fn void TFC_projectionsPrint (const struct _TFC_projections  *prjs,
                                 int                          towerId)
  \brief Prints an ASCII dump of the projections for the specified tower
  
  \param prjs     The list of projections for the specified tower
  \param towerId  The tower id associated with this list of projections
                                                                          */
/* ---------------------------------------------------------------------- */
extern void TFC_projectionsPrint (const struct _TFC_projections  *prjs,
                                  int                          towerId)
{
    printProjections (prjs->prjs, prjs->xy[0], prjs->xy[1], towerId);
    return;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn void printProjections (const TFC_projection *prj,
                             int                  xCnt,
                             int                  yCnt,
                             int               towerId)
  \brief Prints an ASCII dump of the X and Y projections from the specified
         list.
          
  \param  prj    The list of projections. It is assumed that the projections
                 are ordered X projections first, followed by the Y
                 projections.
  \param xCnt    The number of X projections
  \param yCnt    The number of Y projections
  \param towerId The tower id associated with this list of projections   
                                                                          */
/* ---------------------------------------------------------------------- */
static void printProjections (const TFC_projection *prj,
                              int                  xCnt,
                              int                  yCnt,
                              int               towerId)
{
   int idx = 1;
   int cnt = xCnt;
   char which = 'X';

   if ((xCnt + yCnt) <= 0)
   {
       printf ("\nTower %1.1x - no projections\n", towerId);
       return;
   }
   else
   {
         
       printf ("\n"
               "%1.1X Min:Max   Layers Nhits Intercept Slope  Acd Top\n"
                   ". ---:--- -------- ----- --------- ----- --------\n",
               towerId);
   }
   

   while (1)
   {

       while (--cnt >= 0)
       {
           int                layer;
           int              nmargin;
           int                 ncol;
           int               layers;
           const signed short *hits;
           
              
           nmargin = printf ("%c  %2d:%2d  %8.8x %5d %9d %5d %8.8x",
                             which,
                             prj->min,
                             prj->max,
                             prj->layers,
                             prj->nhits,
                             prj->intercept,
                             prj->slope,
                             prj->acdTopMask);
           
           which  = ' ';
           layer  = prj->max;
           hits   = prj->hits;
           ncol   = nmargin;

           /* Push the maximum layer to the most significant bit */
           layers = prj->layers << (31 - prj->max);

           /* Print the hits */
           while (1)
           {
               int hit = *hits++;

               
               if (layers < 0) ncol += printf (" %2d:%-4d ", layer, hit);
               else            ncol += printf (" %2d<%-4d>", layer, hit);

                 
               layers <<= 1;
               layer   -= 1;
                 
               if (layers == 0) break;
               
               /* More hits, is a line feed needed */
               if (ncol >= 80 - 8) ncol = printf ("\n%*c", nmargin, ' ')-1;

           }

           /* Next projection */
           printf ("\n");
           prj++;
       }

       
       /* Check if must do the Y projections */
       if (--idx < 0) return;
       cnt   = yCnt;
       which = 'Y';
   }

   return;
   
}
/* ---------------------------------------------------------------------- */   
