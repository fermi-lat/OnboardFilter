/* ---------------------------------------------------------------------- *//*!
   
   \file  TFC_latRecordPrint.c
   \brief Prints an ASCII display of the TKR LAT tracker data.
   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $Id
\endverbatim
                                                                          */
/* ---------------------------------------------------------------------- */



#include <stdio.h>

#include "ffs.h"
#include "DFC/TFC_latRecordPrint.h"
#include "DFC/TFC_latRecord.h"
#include "DFC/TFC_towerRecordPrint.h"


#ifdef __cplusplus
extern "C" {
#endif


void printStrips (int                       layer,
                  const struct _TFC_towerLayer *x,
                  const struct _TFC_towerLayer *y);

static int outputTots (int left, int hiLo, const unsigned char *tots);
    

    
#if 0    
static void printStrips (int            layer,
                         const TFC_strip *cur,
                         const TFC_strip *end);
#endif    

#ifdef __cplusplus
}
#endif



/* ---------------------------------------------------------------------- *//*!

  \fn     void TFC_latRecordPrint (const struct _TFC_latRecord *tlr,
                                   int                        which)
  \brief       Prints an ASCII display of all the towers which have been
               unpacked.
  \param   tlr The tracker LAT record to be displayed.
  \param which A left justified (MSB = Tower 0) of which towers to print
                                                                          */
/* ---------------------------------------------------------------------- */
void TFC_latRecordPrint (const struct _TFC_latRecord *tlr,
                         int                        which)
{
   int twrMap;
   const struct _TFC_towerRecord *twrs;

   
   /* Extract the map of towers that have been unpacked */
   twrMap = tlr->twrMap;
   

   
   /* If there aren't any, just return */
   if (twrMap == 0)
   {
       printf ("\nTFC_latRecordPrint: Empty event\n");
       return;
   }

   
   /* Limit the display to only those requested */
   twrMap &= which;
   if (twrMap == 0)
   {
       printf ("\nTRC_latRecordPrint: Request for only empty towers (%8.8x)\n",
               which);
       return;
   }
   
       

   /* Extract a pointer to the array of TKR tower records */
   twrs = tlr->twr;
   

   /* Loop over and display the unpacked towers */
   do
   {
       int                          tower;
       const struct _TFC_towerRecord *twr;

       
       /*
        | 1. Find the tower number of the first tower with hits.
        | 2. Eliminate this tower from the list to be displayed.
        | 3. Compute address of the TKR tower record.
       */
       tower   = FFS (twrMap);
       twrMap  = FFS_eliminate (twrMap, tower);
       twr     = twrs + tower;

       TFC_towerRecordPrint (twr);
   }
   while (twrMap);

   
   return;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

 \fn  void TFC_towerRecordPrint  (const struct _TFC_towerRecord *ttr)
 \brief     Prints an ASCII display of the tracker data from the TKR
            Tower Record (ttr).
 \param ttr The TKR Tower Record to print.
                                                                          */
/* ---------------------------------------------------------------------- */ 
void TFC_towerRecordPrint  (const struct _TFC_towerRecord *ttr)
{
   int  map;
   int xmap = ttr->layerMaps[0];
   int ymap = ttr->layerMaps[1];
   

   /* Print the tower header line */
   printf ("\n%1.1X.L| X map = 0x%5.5x                     | Y map = 0x%5.5x"
           "               TOWER %1.1x\n",
           ttr->id,
           xmap,
           ymap,
           ttr->id);

   /*
    | The display is a side-by-side display of X and Y. To do this need
    | to output a line if either the X or Y layer has hits.
   */
   map = xmap | ymap;

   /*
    | Loop over the OR of the struck X and Y layers. This loop prints
    | the X and Y hits for the struck layers.
    |
    | Note the x and y hit maps are packed right justified with
    | LSB = layer 0.
   */
   while (map)
   {
       int     n;
       int   bit;
       int layer;

       n     = FFS (map);            /* Get the next set bit         */
       bit   = FFS_mask (n);         /* Convert to a bit mask        */
       map  &= ~bit;                 /* Eliminate from the input map */
       layer = 31 - n;               /* Convert to a layer number    */

       /*
        | Display the hits on this layer. The X and Y hit maps must
        | be consulted before allowing access to the layer descriptors.
        | The information in the layer descriptors is not valid unless
        | the corresponding hit map has the layer bit set. If the layer
        | descriptor is not valid, a NULL is passed in its place.
       */
       printStrips (layer,
                    (xmap & bit) ? &ttr->layers[layer +  0] : NULL,
                    (ymap & bit) ? &ttr->layers[layer + 18] : NULL);
   }
   
   return;
}
/* ---------------------------------------------------------------------- */ 




/* ---------------------------------------------------------------------- *//*!

  \fn  void printStrips (int                       layer,
                         const struct _TFC_towerLayer *x,
                         const struct _TFC_towerLayer *y)

  \brief         Prints an ASCII display of hits strips.
  \param  layer  The layer of the strips to be displayed.
  \param      x  The X layer description of the strips (may be NULL if
                 there are none.
  \param      y  The Y layer description of the strips (may be NULL if
                 there are none.
                                                                          */
/* ---------------------------------------------------------------------- */
void printStrips (int                       layer,
                  const struct _TFC_towerLayer *x,
                  const struct _TFC_towerLayer *y)
{
   int            margin;
   int               col;
   int              xcnt;
   int              ycnt;
   const TFC_strip *xcur;
   const TFC_strip *ycur;


   /* Get the count and beginning address of the strip for both X and Y */
   xcnt = x ? (xcur = x->beg,  x->end - xcur) : (xcur = NULL, -2);
   ycnt = y ? (ycur = y->beg,  y->end - ycur) : (ycur = NULL, -2);   

   /*
    | 
    | Print the introducer
    |
    | MARGIN keeps track of the left margin so that layers which have
    | require more than 1 line can be properly aligned.
    |
    | COL keeps track of the current column number. This is used to limit
    | the X layer strip addresses to the left half of the screen and
    | the Y layer strip addresses to stay in the right half of the screen.
    |
   */
   margin = col =  printf (" %2d|", layer);


   while (1)
   {
       int n;
       
       /*
        | If haven't positioned to the left margin, do it. This is
        | necessary when the number of hits on either the X of Y
        | exceeds the capacity of 1 line.
       */
       if ( (n = margin - col) > 0) col += printf ("%*c", n,  ' ');
       
       
       /*
        | Loop over the X hits to be displayed. The loop is limited
        | by the number of available hits and the right margin.
        | The margin limit must be checked before testing if there
        | are any hits remaining. Doing it the other way around risks
        | decrementing the hit counter before the strip hit is actually
        | printed. The right margin is set by the left edge of the Y display
        | area.
       */
       while (col <= 36 && --xcnt >= 0)
       {
           col += printf (" %4d", *xcur++ & 0x7ff);
       }

       /* If have exhausted the strips and enough space, output TOTs */
       if (xcnt == -1 && (col <= 40 - 8))
       {
           col += outputTots (40 - col, x->hiLo, x->tots);
           xcnt = -2;
       }
           
       
       /* Move to the middle of the page */
       if ( (n = 40 - col) > 0) col += printf ("%*c", n, ' ');
       col += printf (" |");
       
           
       /*
        | Loop over the Y hits to be displayed. The loop is limited
        | by the number of available hits and the right margin.
        | The margin limit must be checked before testing if there
        | are any hits remaining. Doing it the other way around risks
        | decrementing the hit counter before the strip hit is actually
        | printed. The right margin is set by the right edge of the
        | display (ie | 80 columns).
       */
       while (col <= 76 && --ycnt >= 0)
       {
           col +=  printf (" %4d", *ycur++ & 0x7ff);
       }

       
       /* If have exhausted the strips and enough space, output TOTs */
       if (ycnt == -1 && (col <= 80 - 8))
       {
           col += outputTots (80 - col, y->hiLo, y->tots);
           ycnt = -2;
       }

       
       printf ("\n");
       
       
       /* If nothing left to print... */
       if ((xcnt <= 0) && (ycnt <= 0)) break;
       
       /* Start a new line */
       col = 0;
   }
   
   return;
}


/* ---------------------------------------------------------------------- *//*!

  \fn     int outputTots (int left, int hiLo, const unsigned char *tots)

  \brief         Prints an ASCII display of the TOTs
  \param  left   Number of columns before the left margin
  \param  hiLo   A bit mask indicating which TOT(s) are valid, must 0,1,2,3
  \param  tots   The array of (potentially) 2 TOTs
  \return        The number of columns used to print
                                                                          */
/* ---------------------------------------------------------------------- */
static int outputTots (int left, int hiLo, const unsigned char *tots)
{
    int n;
    left -= 8;
    if      (hiLo == 0) n = 0;
    else if (hiLo == 1) n = printf ("%*c [%2.2x,  ]",    left, ' ', tots[0]);
    else if (hiLo == 2) n = printf ("%*c [  ,%2.2x]",    left, ' ', tots[1]);
    else                n = printf ("%*c [%2.2x,%2.2x]", left, ' ', tots[0],
                                                                    tots[1]);

    return n;
}
/* ---------------------------------------------------------------------- */
