/* ---------------------------------------------------------------------- *//*!
   
   \file   CFC_latRawRecordPrint.c
   \brief  Defines the implementaion of the CAL LAT record print routine.
   \author JJRussell - russell@slac.stanford.edu

\verbatim
    CVS $id
\endverbatim   
                                                                         */
/* --------------------------------------------------------------------- */


#include <stdio.h>
#include "ffs.h"
#include "DFC/CFC_latRawRecord.h"
#include "DFC/CFC_latRawRecordPrint.h"
#include "DFC/CFC_towerRawRecordPrint.h"


static char get_display_char (const unsigned short int *phas);



/* ---------------------------------------------------------------------- *//*!

  \fn    void CFC_latRawRecordPrint (const struct _CFC_latRawRecord *clr,
                                     int                           which)
  \brief        Produces an ASCII display of the CAL data.
  \param clr    A previously unpacked CAL LAT Raw Record.
  \param which  A left justified mask of which towers to print
                (MSB = TOWER 0)
                                                                          */
/* ---------------------------------------------------------------------- */
void CFC_latRawRecordPrint (const struct _CFC_latRawRecord *clr,
                            int                           which)
{
   int map;
 
  
   map = clr->twrMap & which;
   

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
       int                     tower;
       const CFC_towerRawRecord *ctr;

       tower = FFS (map);
       ctr   = clr->twrs + tower;
       map   = FFS_eliminate (map, tower);

       CFC_towerRawRecordPrint (ctr, tower);
   }
   while (map);


   return;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn    void CFC_latRawRecordMapPrint (const struct _CFC_latRawRecord *clr)
  \brief        Produces an ASCII display of the CAL data in the form
                of a map, giving one an geometrical idea of where the
                energy is deposited.
  \param clr    A previously unpacked CAL LAT Raw Record.
                                                                          */
/* ---------------------------------------------------------------------- */
void CFC_latRawRecordMapPrint (const struct _CFC_latRawRecord *clr)
{
   int twrMap;
   int row;

   /* Map of which towers have hits */
   twrMap = clr->twrMap;
  
    printf (" T.l              C             D             E             F\n"
            " --    ------------  ------------  ------------  ------------\n");
   
   
   for (row = 3; row >= 0; row--)
   {
       int layer;
       int itwr = row * 4;
       
       
       printf (" %1X", itwr);
       for (layer = 0; layer < 8; layer++)
       {
           int col;

           printf (layer ? "  .%1d%c" : ".%1d%c",
                   layer%4, (layer/4) ? 'y' : 'x');

           
           for (col = 0;  col < 4; col++)
           {
               int     tcol;
               int   colMap;
               int layerMap;
               int   twrNum;
               const unsigned short int *phas;
               const CFC_towerRawRecord  *ctr;

               twrNum = itwr + col;
               
               if ((twrMap & FFS_mask (twrNum)) == 0)
               {
		   printf ("  ............\n");
		   continue;
	       }

	       ctr      = &clr->twrs[twrNum];
	       layerMap =  ctr->layerMap;
               colMap   =  ctr->colMap[layer] << 16;
               phas     = &ctr->phas[ctr->ophas[layer]];
               
               printf ("  ");
               for (tcol = 0; tcol < 12;  tcol++)
               {
                   char c;
                   
                   if (colMap < 0)
                   {
                       c = get_display_char (phas);
                       phas += 2;
                   }
                   else
                   {
                       c = '.';
                   }
                       
                   printf ("%c", c);
                   colMap <<= 1;
               }

           }
           
           printf (layer ? "\n" : "  %1X\n", itwr+3);
       }


       /* Fill in the labels for the middle towers */
       if (( row == 3) || row == 1)
       {
           int diff = (row == 3) ? -4 : 0;
           printf ("                   %1x                           %1x\n",
                   itwr + diff + 1, itwr + diff + 2);
       }
       else
       {
           printf ("\n");
       }
       
   }


   printf (" --    ------------  ------------  ------------  ------------\n"
           " T.l              0             1             2             3\n");
           

   return;
}
/* ---------------------------------------------------------------------- */
       


/* ---------------------------------------------------------------------- *//*!

  \fn    char get_display_char (const unsigned short int *phas)
  \brief  Encodes the PHA values of both ends of the log into a single
          character based on their logarithms.
  \return The display character.
  \param phas The array PHA values of the 2 ends.

  The symbol always indicates the largest power of 2 of either end. If the
  largest power of 2 is the same for both ends, a lower case letter is used.
  Otherwise, an upper case letter is used.
									  */
/* ---------------------------------------------------------------------- */
static char get_display_char (const unsigned short int *phas)
{
    const static char Display[2][8] =
    {
        { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'  },
        { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'  },
    };
    
    
    unsigned short int pha_A;
    unsigned short int pha_B;
    int                pwr_A;
    int                pwr_B;
    int                 diff;
    int                  max;
                       
    
    pha_A = phas[0];
    pha_B = phas[1];

    /*
     | Find the log 2 of the pha value. The smallest
     | value FFS should return is 2, which then gets
     | mapped to 14. We will ignore any value under
     | 7. 
    */
    pwr_A = 32 - FFS (pha_A);
    pwr_B = 32 - FFS (pha_B);

    if (pwr_A < 7) pwr_A = 7;
    if (pwr_B < 7) pwr_B = 7;
    
    if (pwr_A > pwr_B)
    {
        diff = pwr_A - pwr_B;
        max  = pwr_A;
    }
    else
    {
        diff = pwr_B - pwr_A;
        max  = pwr_B;
    }
                       

    /* We should now have a number between 7 and 14, change to 0-7 */
    max -= 7;

    return (diff == 0) ? Display[0][max] : Display[1][max];
}
/* ---------------------------------------------------------------------- */

                       
                        



