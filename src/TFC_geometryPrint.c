/* ---------------------------------------------------------------------- */
/*!
   
   \file  TFC_geometryPrint.c
   \brief Routines to print the geometry of the LAT. These are primarily
          used for debugging and tracking the TKR geometry structure.

   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $Id
\endverbatim
                                                                          */
/* ---------------------------------------------------------------------- */

#include <stdio.h>
#include "DFC/TFC_geometryPrint.h"
#include "../src/TFC_geometryDef.h"


#ifdef __cplusplus
extern "C" {
#endif


/* ---------------------------------------------------------------------- */
/*  Primary dispatch routines                                             */
/* ---------------------------------------------------------------------- */
static void tagHdrPrint (void);
static void tagPrint    (const TFC_tag             *tag);
static void tkrPrint    (const TFC_geometryTkr     *tkr);
static void acdPrint    (const TFC_geometryAcd     *acd, 
                         unsigned int        stripPitch);
static void skirtPrint  (const TFC_geometrySkirt *skirt);
static void tkrAcdPrint (const TFC_geometryTkr     *tkr,
                         const TFC_geometryAcd     *acd);
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- */
/*  Helper routines                                                       */
/* ---------------------------------------------------------------------- */
static void acdTopPrint    (const short      edges[6], 
			    const int   boundaries[2],
			    unsigned int   stripPitch,
                            char                  xy);

static void acdSidePrint   (const short      edges[6], 
                            const int   boundaries[2],
			    unsigned int   stripPitch,
                            char                   xy,
                            char                 sign,
                            char                   yx);

static void acdRowColPrint (const short      edges[6],
			    const int   boundaries[2],
			    unsigned int   stripPitch,
			    const char      rowCol[4]);
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- */
/*  Macros                                                                */
/* ---------------------------------------------------------------------- */
/*!

  \def    SCALE(_v, _by)
  \brief  Used for formatting a scale number as xx.yyy, where xx is the
          whole piece of the scaled number / 1000 and yyy is the remainder.
  \return A comma separate list meant to be used as a pair of arguments
          in a printf format statement.

  \param  _v The number to scale and format
  \param _by The scale factor.

  Scales \a _v by the value \a _by and returns two integers representing 
  the scale number / 1000 and % 1000 as a comma separated list. This makes
  it suitable for printing as a pseudo floating point number, with the
  first number representing the whole integer piece and the second 
  representing the fractional piece.
									  */
/* ---------------------------------------------------------------------- */
#define SCALE(_v, _by)   ((_v) * ((int)(_by)))/1000,                      \
                       ( ((_v) >= 0 ? (_v) : -(_v)) *((int)(_by)) )%1000
/* ---------------------------------------------------------------------- */



#ifdef __cplusplus
}
#endif
 

   
/* ---------------------------------------------------------------------- */
/*!

  \fn void TFC_geometryPrint (const struct _TFC_geometry *geo,
                              unsigned int               opts)
  \brief   Provides an ASCII display of the track geomety structure
  
  \param  geo The track geometry to display
  \param opts Display options, see _enum _TFC_geometryPrintOpts

   This routines examines the display options and calls the appropriate
   print routine.
                                                                          */
/* ---------------------------------------------------------------------- */
void TFC_geometryPrint (const struct _TFC_geometry *geo,
			unsigned int               opts)
{
   unsigned int stripPitch = geo->tkr.stripPitch;

   if (opts & TFC_M_GEO_OPTS_TAG_HDR) tagHdrPrint ();  
   if (opts & TFC_M_GEO_OPTS_TAG    ) tagPrint    (&geo->tag);
   if (opts & TFC_M_GEO_OPTS_TKR    ) tkrPrint    (&geo->tkr);
   if (opts & TFC_M_GEO_OPTS_ACD    ) acdPrint    (&geo->acd, stripPitch);
   if (opts & TFC_M_GEO_OPTS_SKIRT  ) skirtPrint  (&geo->skirt);
   if (opts & TFC_M_GEO_OPTS_TKR_ACD) tkrAcdPrint (&geo->tkr, &geo->acd);

   return;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- */
/*!

  \fn    void tagHdrPrint (void)
  \brief Prints the header line for the TAG output
									  */
/* ---------------------------------------------------------------------- */
static void tagHdrPrint (void)
{
    printf ("  Id  Geo Type         CMT Tag     Created     Revised\n"
	    " ---  --------  --------------  ----------  ----------\n");
    return;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

  \fn    void tagPrint (const TFC_tag *tag)
  \brief Prints the tag fields

  \param tag  The geometry's tag structure
									  */
/* ---------------------------------------------------------------------- */
static void tagPrint (const TFC_tag *tag)
{
  const char                 *name;
  char                  cmttag[14];
  TFC_tagtype type = tag->cmt.type;


  /* Associate a name with the tag type                       */
  if      (type == TFC_K_TAGTYPE_LAT  ) name =      "LAT";
  else if (type == TFC_K_TAGTYPE_GLEAM) name = "GLEAM MC";
  else                                  name =  "UNKNOWN";


  /* Format the CMT tag fields into a compact string          */
  sprintf (cmttag, "v%d/r%d/p%d",
	   tag->cmt.version,
	   tag->cmt.revision,
	   tag->cmt.patch);


  /* Print the tag identifier fields                          */
  printf (" %3d  %8.8s  %14.14s  %2d/%2d/%4d  %2d/%2d/%4d\n",
	  tag->id,
	  name,
	  cmttag,
	  tag->date.creation.month,
	  tag->date.creation.day,
	  tag->date.creation.year,
	  tag->date.revision.month,
	  tag->date.revision.day,
	  tag->date.revision.year);

  return;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

  \fn     acdPrint (const TFC_geometryAcd *acd, unsigned int stripPitch)
  \brief  Provides an ASCII display of the geometry used when projecting
          tracks to the ACD planes.

  \param        acd The geomety used to project tracks to the ACD planes.
  \param stripPitch The scale factor to apply to edges to give mm.
                                                                          */
/* ---------------------------------------------------------------------- */  
void acdPrint (const TFC_geometryAcd *acd, unsigned int stripPitch)
{
  int idx;


  printf ("\n"
          " ACD geometry\n"
          " ------------\n"
          " Twice tkr nominal spacing = %d\n", acd->zNominal);
 

  /* Give a representation of the ACD TOP plane X and Y boundaries */
  acdTopPrint (acd->xTopEdges, acd->xySides + 0, stripPitch, 'X');
  acdTopPrint (acd->yTopEdges, acd->xySides + 2, stripPitch, 'Y');


  /* 
   | Give a representation of the ACD SIDE plane XZ and YZ boundaries.
   | Note that the boundaries are kind of cross threaded, that is, since
   | the X planes really display the Y segmentation, the associations is
   | with the boundaries of the y planes.
  */
  acdSidePrint (acd->xmSideYedges, acd->xySides + 2, stripPitch, 'X','-','Y');
  acdSidePrint (acd->xpSideYedges, acd->xySides + 2, stripPitch, 'X','+','Y');
  acdSidePrint (acd->ymSideXedges, acd->xySides + 0, stripPitch, 'Y','-','X');
  acdSidePrint (acd->ypSideXedges, acd->xySides + 0, stripPitch, 'Y','+','X');

  
  /* 
   | This prints the projection factor to get from a given tracker layer
   | to the ACD top plane.
  */
  printf ("\n"
          " Layer X Lay-ACD Y Lay-ACD Projection from TKR[layer] to ACD TOP\n"
          " ----- --------- ---------\n");
  
  /* 
   | Note that since the projection always starts from the top of a triple
   | of adjacent layers, only layers 2-17 are relevant.
  */
  for (idx = 17; idx >= 2; idx--)
  {
      printf (" %5d %9d %9d\n",
              idx,
              acd->xProjections[idx],
              acd->yProjections[idx]);
  }
  
  
  return;
}
/* ---------------------------------------------------------------------- */  




/* ---------------------------------------------------------------------- */
/*!

  \fn          void acdTopPrint (const short      edges[6],
                                 const int   boundaries[2], 
  			         unsigned int   stripPitch,
                                 char                   xy)
  \brief       Internal utility routine to print the ACD top plane geometry

  \param      edges An array of 6 numbers giving the defining edges of
                    the 5 tiles that make up the ACD top plane.
  \param boundaries The X / Y boundaries of the side ACD planes.
  \param stripPitch The scale factor to apply to edges to give mm.
  \param xy         A single character, indicating whether the X or Y
                    boundaries are being displayed (should be one of 'X'
                    or 'Y'). 
                                                                          */
/* ---------------------------------------------------------------------- */
static void acdTopPrint (const short      edges[6],
			 const int   boundaries[2],
			 unsigned int   stripPitch, 
                         char                   xy)
{
  printf ("\n                  %c Top Edges\n", xy);
  acdRowColPrint (edges, boundaries, stripPitch, xy == 'X' ? "Col" : "Row");
}
/* ---------------------------------------------------------------------- */

   


/* ---------------------------------------------------------------------- */
/*!

  \fn          void acdSidePrint (const short      edges[6], 
                                  const int   boundaries[2],
                                   unsigned int  stripPitch,
                                  char                   xy,
                                  char                 sign,
                                  char                   yx)
  \brief            Internal utility routine to print the ACD side plane
                    geometry.
  \param edges      An array of 6 numbers giving the defining edges of
                    the 5 tiles that make up the ACD top plane.
  \param boundaries The '-' and '+' boundaries of the other
                    face.
  \param stripPitch The scale factor to apply to edges to give mm.
  \param xy         A single character, indicating whether the X or Y 
                    boundaries are being displayed. Should be one of 'X'
                    or 'Y'.
  \param sign       A single character, indicating whether this is a '-' 
                    or '+' plane being displayed. Should be one of '-'
                    or '+'.
  \param yx         A single character, which must be the complement of
                    the \a xy parameter, ie if xy = X, the yx = Y, and
                    visa-versa.
                                                                          */
/* ---------------------------------------------------------------------- */
static void acdSidePrint (const short      edges[6], 
                          const int   boundaries[2], 
			  unsigned int   stripPitch,
                          char                   xy,
                          char                 sign,
                          char                   yx)
{
    printf ("\n               %c%c Side %c Edges\n", xy, sign, yx);
    acdRowColPrint (edges, boundaries, stripPitch, "Col");
    return;
}

/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
#ifndef CMX_DOXYGEN
/* ---------------------------------------------------------------------- *\
 |                                                                        
 |  These are just formating statements that are too big to fit on one
 |  line. 
\* ---------------------------------------------------------------------- */   

#define ACD_FMT_ROWCOL_TOPBOT \
" +-+       +----------+----------+----------+----------+----------+     +-+\n"


#define ACD_FMT_ROWCOL_MIDDLE \
" | |       |   %3s0   |   %3s1   |   %3s2   |   %3s3   |   %3s4   |     | |\n"

/* ---------------------------------------------------------------------- */
#endif
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

  \fn     void acdRowColPrint (const short    edges[6],
		               const int boundaries[2],
                               unsigned int stripPitch,
			       const char    rowCol[4])
  \brief  Prints a representation of the edges with the boundaries of
          corresponding side tiles

  \param      edges An array of 6 numbers giving the defining edges of
                    the 5 tiles that make up the ACD top plane.
  \param boundaries The X / Y boundaries of the side ACD planes.
  \param stripPitch The scale factor to apply to edges to give mm.
  \param     rowCol The string 'Row' or 'Col' as appropriate
									  */
/* ---------------------------------------------------------------------- */
static void acdRowColPrint (const short    edges[6],
			    const int boundaries[2],
			    unsigned int stripPitch,
			    const char    rowCol[4])
{
    printf (ACD_FMT_ROWCOL_TOPBOT
	    ACD_FMT_ROWCOL_MIDDLE
	    ACD_FMT_ROWCOL_TOPBOT
	    "%6d %6d     %6d     %6d     %6d     %6d     %6d  %6d\n"
            "%4d.%03d %4d.%03d %4d.%03d   %4d.%03d   %4d.%03d   %4d.%03d   "
            "%4d.%03d %4d.%03d\n",
	    rowCol, rowCol, rowCol, rowCol, rowCol,
	    boundaries[0],
	    edges[0],
	    edges[1],
	    edges[2],
	    edges[3],
	    edges[4],
	    edges[5],
	    boundaries[1],
	    SCALE(boundaries[0], stripPitch),
            SCALE(edges[0],      stripPitch),
	    SCALE(edges[1],      stripPitch),
	    SCALE(edges[2],      stripPitch),
	    SCALE(edges[3],      stripPitch),
	    SCALE(edges[4],      stripPitch),
	    SCALE(edges[5],      stripPitch),
	    SCALE(boundaries[1], stripPitch));

  return;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

  \fn     void tkrPrint (const TFC_geometryTkr *tkr)
  \brief  Provides an ASCII display of the TKR tower geometry.
  \param  tkr The TKR tower geometry
                                                                          */
/* ---------------------------------------------------------------------- */
void tkrPrint (const TFC_geometryTkr *tkr)
{
   int               ixy;
   int       stripPitch = tkr->stripPitch;
   unsigned int   zfind = tkr->zfindMaxMin;
   unsigned int zextend = tkr->zextendMaxMin;

   printf (
       "\n"
       " Tracker Tower Geometry\n"
       " ----------------------\n"
       " ZFind Max: %6d  Min: %6d    ZExtend Max: %6d  Min: %6d\n"
       "\n",
       (zfind   >> 16) & 0xffff,   zfind & 0xffff,
       (zextend >> 16) & 0xffff, zextend & 0xffff);


   printf (
" XY       RowCol 0         RowCol 1          RowCol 2          RowCol 3\n"
" -- --------:-------- --------:-------- --------:-------- --------:--------\n");

   for (ixy = 0; ixy < 2; ixy++)
   {
     int                    iedge;
     int dxy = tkr->xyWidths[ixy];

     if (ixy == 1) printf ("\n");     
     printf ("  %c",  ixy == 0 ? 'X' : 'Y');


     for (iedge = 0; iedge < 4; iedge++)
     {
       int xyBeg = tkr->twr.xy[ixy].offsets[iedge];
       int xyEnd = xyBeg + dxy;

       printf (" %8d:%8d", xyBeg, xyEnd);
     }

     printf ("\n"
             "   ");

     for (iedge = 0; iedge < 4; iedge++)
     {
       int xyBeg = tkr->twr.xy[ixy].offsets[iedge];
       int xyEnd = xyBeg + dxy;

       printf (" %4d.%03d:%4d.%03d", 
	       SCALE(xyBeg, stripPitch), 
	       SCALE(xyEnd, stripPitch));
     }

     printf ("\n");

   }

   return;
   
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

  \fn void skirtPrint (const TFC_geometrySkirt *skirt)
  \brief   Provides an ASCII display of the geometry used when projecting
           tracks to the skirt (TKR/CAL boundary) plane.
  \param   skirt The geomety used to project tracks to the skirt plane.
                                                                          */
/* ---------------------------------------------------------------------- */  
void skirtPrint (const TFC_geometrySkirt *skirt)
{
   int idx;
 
   printf ("\n"
           " Skirt Region Geometry\n"
           " ---------------------\n"
           " Z skirt    = %6d\n"
           " X[-/+:-/+] = %6d:%6d %6d:%6d\n"
           " Y[-/+:-/+] = %6d:%6d %6d:%6d\n",
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

  \fn    void tkrAcdPrint (const TFC_geometryTkr *tkr,
                           const TFC_geometryAcd *acd)
  \brief Prints a display showing the relationship of the TKR and ACD.

  \param tkr  The tracker geometry
  \param acd  The acd     geometry
									  */
/* ---------------------------------------------------------------------- */
static void tkrAcdPrint (const TFC_geometryTkr *tkr,
                         const TFC_geometryAcd *acd)
{
   int layer;
   int   row;
   int  acdZ;


   printf (
    " \n"
    " TKR / ACD Z geometry\n"
    " --------------------\n"
    "\n"
    " Layer  X Z pos (  dLayer )  Y Z pos (  dLayer )  Acd Side Row\n"
    " -----  -------------------  -------------------  -------------\n");

   /* Top of ACD is above the top of the tracker */
   {
       printf ("%53c%10d\n", ' ', acd->zSides[0]);
       row  = 1;
       acdZ = acd->zSides[row];
   }

   
   layer = sizeof (tkr->twr.xy[0].z) / sizeof (*tkr->twr.xy[0].z);   
   while (--layer >= 0)
   {
       int xz = tkr->twr.xy[0].z[layer];
       int yz = tkr->twr.xy[1].z[layer];

       if (layer == 0)
       {
	 printf (" %5d  %7d              %7d       ", layer, xz, yz);
       }
       else 
       {
	 int dxz = xz - tkr->twr.xy[0].z[layer-1];
         int dyz = yz - tkr->twr.xy[1].z[layer-1];
         if (layer == 1)
         {
	   printf (" %5d  %7d (%4d     )  %7d (%4d     )", layer,
                     xz, dxz, yz, dyz);
	 }
         else
	 {
	   int ddxz = xz - tkr->twr.xy[0].z[layer-2];
           int ddyz = yz - tkr->twr.xy[1].z[layer-2];
	   printf (" %5d  %7d (%4d,%4d)  %7d (%4d,%4d)", layer,
                     xz, dxz, ddxz, yz, dyz, ddyz);
	 }
       }
           

       if (row >= 0 && acdZ > (xz + yz)/2)
       {
           printf ("     %10d", acdZ);
           row += 1;
           if (row <= 4) acdZ = acd->zSides[row];
       }

       printf ("\n");
   }

   
   /* If haven't printed the last of the ACD rows */
   while (row <= 4)
   {
       printf ("%53c%10d\n", ' ', acdZ);
       row += 1;
       acdZ = acd->zSides[row];
   }

   
   return;
}
/* ---------------------------------------------------------------------- */  
   













