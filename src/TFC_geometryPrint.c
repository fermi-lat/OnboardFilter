/*------------------------------------------------------------------------
| CVS $Id$
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  TFC_geometryPrint.c
   \brief Routines to print the geometry of the LAT. These are primarily
          used for debugging and tracking the TKR geometry structure.

   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */

#include <stdio.h>
#include "DFC/TFC_geometryPrint.h"
#include "TFC_geometryDef.h"

#ifdef __cplusplus
extern "C" {
#endif
    
static void acdTopPrint  (const short edges[6], char xy);
static void acdSidePrint (const short edges[6], char xy, char yx);

#ifdef __cplusplus
}

#endif
    
/* ---------------------------------------------------------------------- *//*!

  \fn void TFC_geometryPrint (const struct _TFC_geometry *geo)
  \brief   Provides an ASCII display of the track geomety structure
  
  \param geo The track geometry to display
                                                                          */
/* ---------------------------------------------------------------------- */
void TFC_geometryPrint (const struct _TFC_geometry *geo)
{
   int layer;
   int   row;
   int  acdZ;
   
   TFC_geometryTkrPrint   (&geo->tkr);
   TFC_geometryAcdPrint   (&geo->acd);
   TFC_geometrySkirtPrint (&geo->skirt);
   

   printf (" \n"
           " Layer X Z position Y Z position  Acd Side Row\n"
           " ----- ------------ ------------ -------------\n");
   


   /* Top of ACD is above the top of the tracker */
   {
       printf ("%32c %10d\n", ' ', geo->acd.zSides[0]);
       row  = 1;
       acdZ = geo->acd.zSides[row];
   }

   
   layer = sizeof (geo->tkr.twr.xy[0].z) / sizeof (*geo->tkr.twr.xy[0].z);   
   while (--layer >= 0)
   {
       int xz = geo->tkr.twr.xy[0].z[layer];
       int yz = geo->tkr.twr.xy[1].z[layer];
       
       printf (" %5d   %10d  %10d", layer, xz, yz);

       if (row >= 0 && acdZ > (xz + yz)/2)
       {
           printf ("  %10d", acdZ);
           row += 1;
           if (row <= 4) acdZ = geo->acd.zSides[row];
       }

       printf ("\n");
   }

   
   /* If haven't printed the last of the ACD rows */
   while (row <= 4)
   {
       printf ("%32c %10d\n", ' ', acdZ);
       row += 1;
       acdZ = geo->acd.zSides[row];
   }

   
   return;
}
/* ---------------------------------------------------------------------- */  





/* ---------------------------------------------------------------------- *//*!

  \fn     void TFC_geometryAcdPrint (const struct _TFC_geometryAcd *acd)
  \brief  Provides an ASCII display of the geometry used when projecting
          tracks to the ACD planes.
  \param  acd The geomety used to project tracks to the ACD planes.
                                                                          */
/* ---------------------------------------------------------------------- */  
void TFC_geometryAcdPrint (const struct _TFC_geometryAcd *acd)
{
  int idx;
 
  printf ("Twice tkr nominal spacing = %d\n", acd->zNominal);
 
  acdTopPrint (acd->xTopEdges, 'X');
  acdTopPrint (acd->yTopEdges, 'Y');

  acdSidePrint (acd->xSideYedges, 'X', 'Y');
  acdSidePrint (acd->ySideXedges, 'Y', 'X');

  printf ("\nAcd Side Faces[X-/+:Y-/+] = %6d %6d %6d %6d\n",
          acd->xySides[0],
          acd->xySides[1],
          acd->xySides[2],
          acd->xySides[3]);

  printf ("\n"
          " Layer X Lay-ACD Y Lay-ACD Projection from TKR[layer] to ACD TOP\n"
          " ----- --------- ---------\n");
  
  for (idx = 17; idx >= 0; idx--)
  {
      printf (" %5d %9d %9d\n",
              idx,
              acd->xProjections[idx],
              acd->yProjections[idx]);
  }
  
  
  return;
}
/* ---------------------------------------------------------------------- */  





/* ---------------------------------------------------------------------- *//*!

  \fn     void TFC_geometryTkrPrint (const struct _TFC_geometryTkr *geo)
  \brief  Provides an ASCII display of the TKR tower geometry.
  \param  geo The TKR tower geometry
                                                                          */
/* ---------------------------------------------------------------------- */
void TFC_geometryTkrPrint (const struct _TFC_geometryTkr *geo)
{
   unsigned int zfind   = geo->zfindMaxMin;
   unsigned int zextend = geo->zextendMaxMin;
   
   
   printf (" Tracker Tower Geometry\n\n");

   printf (" ZFind Max: %6d  Min: %6d    ZExtend Max: %6d  Min: %6d\n",
           (zfind   >> 16) & 0xffff,   zfind & 0xffff,
           (zextend >> 16) & 0xffff, zextend & 0xffff);
           
   
   printf ("\n"
           " Offsets RowCol 0 RowCol 1 RowCol 2 RowCol 3\n"
           " ------- -------- -------- -------- -------\n"
           "       X %8d %8d %8d %8d\n"
           "       Y %8d %8d %8d %8d\n",
           geo->twr.xy[0].offsets[0], geo->twr.xy[0].offsets[1],
           geo->twr.xy[0].offsets[2], geo->twr.xy[0].offsets[3],
           geo->twr.xy[1].offsets[0], geo->twr.xy[1].offsets[1],
           geo->twr.xy[1].offsets[2], geo->twr.xy[1].offsets[3]);
   

   
   return;
   
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn void TFC_geometrySkirtPrint (const struct _TFC_geometrySkirt *skirt)
  \brief   Provides an ASCII display of the geometry used when projecting
           tracks to the skirt (TKR/CAL boundary) plane.
  \param   skirt The geomety used to project tracks to the skirt plane.
                                                                          */
/* ---------------------------------------------------------------------- */  

void TFC_geometrySkirtPrint (const struct _TFC_geometrySkirt *skirt)
{
   int idx;
 
   printf ("\n"
           "Skirt Region Geometry\n");

   printf ("Z = %6d\n"
           "X[-/+:-/+] = %6d:%6d %6d:%6d\n"
           "Y[-/+:-/+] = %6d:%6d %6d:%6d\n",
           skirt->z,           
           skirt->xy[0][0], skirt->xy[0][1], skirt->xy[0][2], skirt->xy[0][3],
           skirt->xy[1][0], skirt->xy[1][1], skirt->xy[1][2], skirt->xy[1][3]);

  printf (
      "\n"
      " Layer X Lay-Skirt Y Lay-Skirt Projection from TKR[layer] to SKIRT\n"
      " ----- ----------- -----------\n");
  
  for (idx = 17; idx >= 0; idx--)
  {
      printf (" %5d   %9d   %9d\n",
              idx,
              skirt->xProjections[idx],
              skirt->yProjections[idx]);
  }

   
   return;
}
/* ---------------------------------------------------------------------- */
   





/* ---------------------------------------------------------------------- *//*!

  \fn          void acdTopPrint (const short edges[6], char xy)
  \brief       Internal utility routine to print the ACD top plane geometry
  \param edges An array of 6 numbers giving the defining edges of the
               the 5 tiles that make up the ACD top plane.
  \param xy    A single character, indicating whether the X or Y boundaries
               are being displayed (should be one of 'X' or 'Y'). 
                                                                          */
/* ---------------------------------------------------------------------- */
static void acdTopPrint (const short edges[6], char xy)
{
  
  printf (
      "\n                  %c Top Edges\n"
      "   +---Row0---+----Row1---+----Row2---+----Row3----+----Row4----+\n"
      "%6d      %6d      %6d      %6d      %6d      %6d\n",
      xy,
      edges[0],
      edges[1],
      edges[2],
      edges[3],
      edges[4],
      edges[5]);

  return;
  
}
/* ---------------------------------------------------------------------- */

   

/* ---------------------------------------------------------------------- *//*!

  \fn          void acdSidePrint (const short edges[6], char xy, char yx)
  \brief       Internal utility routine to print the ACD side plane geometry
  \param edges An array of 6 numbers giving the defining edges of the
               the 5 tiles that make up the ACD top plane.
  \param xy    A single character, indicating whether the X or Y boundaries
               are being displayed (should be one of 'X' or 'Y').
  \parm  yx    A single character, which should the complement of the
               \a xy parameter, ie if xy = X, the yx = Y, and visa-versa.
                                                                          */
/* ---------------------------------------------------------------------- */
static void acdSidePrint (const short edges[6], char xy, char yx)
{
  
   printf (
      "\n                 %c Side %c Edges\n"
      "   +---Col0---+----Col1---+----Col2---+----Col3----+----Col4----+\n"
      "%6d      %6d      %6d      %6d      %6d      %6d\n",
      xy,
      yx,
      edges[0],
      edges[1],
      edges[2],
      edges[3],          
      edges[4],
      edges[5]);

   return;
   
}
/* ---------------------------------------------------------------------- */
