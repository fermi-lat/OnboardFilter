/* ---------------------------------------------------------------------- *//*!
   
   \file   CFC_towerRawRecordPrint.c
   \brief  Implementation of the CAL tower print routine
   \author JJRussell - russell@slac.stanford.edu

\verbatim

   CVS $Id
\endverbatim   

                                                                         */
/* --------------------------------------------------------------------- */


#include <stdio.h>

/* Unfortunately snprintf seems to be somewhat of an orphan in VxWorks   */
#ifdef VXWORKS
#include "private/stdioP.h"
#endif

#include "DFC/CFC_towerRawRecord.h"
#include "DFC/CFC_towerRawRecordPrint.h"





/* --------------------------------------------------------------------- *//*!

  \fn  void CFC_towerRawRecordPrint (const struct _CFC_towerRawRecord *ctr,
                                    int                              tower)
  \brief Prints an ASCII formatted dump of a CTR
  \param   ctr The CTR to display
  \param tower The tower number of the CTR
                                                                         */
/* --------------------------------------------------------------------- */
void CFC_towerRawRecordPrint (const struct _CFC_towerRawRecord *ctr,
                              int                             tower)
{
   int                         layer;
   int                      layerMap;
   const unsigned short int *colMapP;
   const unsigned short int    *phas;

   printf ("\n"
           " %1x "
             "     0     1     2     3     4     5"
             "     6     7     8     9    10    11 %2d\n"  
          " --  ----  ----  ----  ----  ----  ----"
             "  ----  ----  ----  ----  ----  ----\n",
           tower,
           ctr->nphas/2);
   
   
//   printf (
//     "\n"
//   "%1x  Col        PhaA        PhaB\n"
//     "  -.-- ----------- -----------\n",
//     tower);
   
   layer        = 0;
   layerMap     = ctr->layerMap << 16;
   colMapP      = ctr->colMap;
   phas         = ctr->phas;
   
   /* Loop over the struck layers */
   do
   {
       /* MSB (sign bit) indicates whether this tower has something in it */
       if (layerMap > 0) continue;

       {
	 int    colMap;
	 int    column;
	 char   layerNum;
	 char   line[2][80];
	 int    i0, i1, n0, n1;


	 /* Tower has something in it, pick up the column map */
	 colMap   = colMapP[layer] << 16;
	 layerNum = 0x30 | layer;

	 /* Maximum number of characters that can be held in a line */
         n0  = sizeof (line[0]);
         n1  = sizeof (line[1]);

	 n0 -= i0 = snprintf (line[0], 80, " %1d%c",
			      layer % 4, layer/4 ? 'y' : 'x');
	 n1 -= i1 = snprintf (line[1], 80, "   ");
       

	 /* Loop over the columns */
	 for (column = 0;  column < 12; colMap <<= 1, column++)
	 {
           int phaA;
           int phaB;

           /* MSB (sign bit) indicates whether this column has something */
           if (colMap >= 0)
           {
               i0 += snprintf (&line[0][i0], n0, "  ....");
               i1 += snprintf (&line[1][i1], n1, "  ....");
           }
           else
           {
               phaA  = phas[0];
               phaB  = phas[1];
               i0   += snprintf (&line[0][i0], n0, "  %4.4x", phaA);
               i1   += snprintf (&line[1][i1], n1, "  %4.4x", phaB);
               phas += 2;               
           }

           n0 -= i0;
           n1 -= i1;

           
           
           
	   //           printf ("  %c.%2d %5d  %4.4x %5d  %4.4x\n",
	   //                   layerNum,
	   //                   column,
	   //                   phaA, phaA,
	   //                   phaB, phaB);
	   //

	   //           layerNum = ' ';
	 }

       
	 line[0][i0] = ' ';  line[0][i0+1] = 'a';  line[0][i0+2] = 0;
	 line[1][i1] = ' ';  line[1][i1+1] = 'b';  line[1][i1+2] = 0;

       
	 puts (line[0]);
	 puts (line[1]);

       }
       
   }
   while (layer == 3 ? puts ("") : 0, layer++, layerMap <<= 1);   

   return;
}

