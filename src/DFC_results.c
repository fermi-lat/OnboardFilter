/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  DFC_results.c
   \brief Provides routines to output the event-by-event results vector
          to either a terminal (ASCII) or a file (BINARY)
          
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */


#include <stdio.h>
#include <stdlib.h>
#include "tmr.h"

#include "DFC/CFC_constants.h"
#include "DFC/DFC_status.h"
#include "DFC/DFC_results.h"


/* Private include files */
#include "DFC_resultsDef.h"


static void printSizes (const DFC_resultSizes *sizes, int nevts);


/* ---------------------------------------------------------------------- *//*!

  \struct _OutRecord
  \brief  The event-by-event output summary record
                                                                          *//*!
  \typedef OutRecord
  \brief   Typedef for struct _OutRecord

   This record is a slight rearrangement of the DFC_results vector. In
   particular, times have been converted from internal units to
   nanoseconds and energies have been converted to Mev. Thus, except for
   endianness, the interpretation of this record is machine independent.
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct _OutRecord
{
    int    eventNum;  /*!< The DAQE sequence number                       */
    int      status;  /*!< The filter status summary word                 */
    int      energy;  /*!< The total CAL energy of the event in Mev       */
    int     calSize;  /*!< The number of logs struck in the CAL           */
    int     tkrSize;  /*!< The number of strips struck in the TKR         */
    int cal0Elapsed;  /*!< The amount of time to do the CAL0 analysis     */
    int  acdElapsed;  /*!< The amount of time to do the ACD analysis      */
    int  atfElapsed;  /*!< The amount of time to do the ACD/TKR veto      */
    int cal1Elapsed;  /*!< The amount of time to do the CAL0 analysis     */
    int  tkrElapsed;  /*!< The amount of time to do the TKR finding       */
}
OutRecord;
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn     int DFC_resultsSizeof (void)
  \brief  Returns the size, in bytes, of a DFC results record
  \return The size, in bytes, of a DFC results record.
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_resultsSizeof (void)
{
 return sizeof (DFC_results);
}
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \fn    int DFC_resultsPrint (const struct _DFC_results *results,
                               int                          count,
                               int                            opt)
  \brief          Prints the event-by-event result vector to the terminal
  \param results  The results vector to print.
  \param   count  The number of elements in the results vector
  \param     opt  An options vector, expressed as a bit mask, controlling
                  how verbose the display is.

   Currently only two options are supported DFC_M_RESULTS_OPT_EVENT and
   DFC_M_RESULTS_OPT_SUMMARY. These are documented in the interface file.
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_resultsPrint (const struct _DFC_results *results,
                      int                          count,
                      int                            opt)
{
   TMR_tick      beg;
   TMR_tick      end;
   int       elapsed;
   int           idx;
   int       printIt;
   int           tot;
   int        dirTot,  dirCnt;
   int        acdTot,  acdCnt;
   int       cal0Tot, cal0Cnt;
   int        atfTot,  atfCnt;
   int       cal1Tot, cal1Cnt;
   int        tkrTot,  tkrCnt;
   DFC_resultSizes sizes;
   
   memset (&sizes, 0, sizeof (DFC_resultSizes));
   
   /* If no output options, just return */
   if (opt == 0) return 0;
   

   cal0Tot = cal0Cnt = 0;
   acdTot  = acdCnt  = 0;   
   dirTot  = dirCnt  = 0;
   atfTot  = atfCnt  = 0;
   cal1Tot = cal1Cnt = 0;
   tkrTot  = tkrCnt  = 0;
   tot     = 0;


   /* Check if wish to print event-by-event information */
   printIt = (opt & DFC_M_RESULTS_OPT_EVENT) ? 2 : -1;
   for (idx = 0; idx < count; idx++)
   {
       int status;
       int    twr;

       sizes.evt += results->sizes.evt;
       sizes.glt += results->sizes.glt;
       sizes.acd += results->sizes.acd;
       for (twr = 0; twr < 16; twr++)
       {
           sizes.tem[twr].tem += results->sizes.tem[twr].tem;
           sizes.tem[twr].cal += results->sizes.tem[twr].cal;
           sizes.tem[twr].tkr += results->sizes.tem[twr].tkr;
       }
       
           
       
       /* Print the title line every 25 lines */
       if (--printIt == 1)
       {
           printf
          ("\n"    
      "    Event   Status Energy  CAL0    ACD    DIR    ATF   CAL1    TKR\n"
      " -------- -------- ------ ------ ------ ------ ------ ------ ------\n");
           printIt = 25+1;
       }

       
       status   = results->status;
       beg      = results->beg;
       end      = results->cal0;
       elapsed  = TMR_DELTA_IN_NSECS (beg, end);
       cal0Tot += elapsed;
       cal0Cnt += 1;

       
       if (printIt > 0)
           printf (" %8d %8.8x %6d %6d",
                   results->evtNum,
                   results->status,
                   CFC_LEU_TO_MEV(results->energy),
                   elapsed);
       


       if (status & DFC_M_STATUS_ACD)
       {
           beg     = end;
           end     = results->acd;
           elapsed = TMR_DELTA_IN_NSECS (beg, end);

           acdTot += elapsed;
           acdCnt += 1;
           if (printIt > 0) printf (" %6d", elapsed);
           
           if (status & DFC_M_STATUS_DIR)
           {
               beg      = end;               
               end      = results->dir;
               elapsed  = TMR_DELTA_IN_NSECS (beg, end);
               dirTot  += elapsed;
               dirCnt  += 1;
               if (printIt > 0) printf (" %6d", elapsed);

               if (status & DFC_M_STATUS_ATF)
               {
                   beg     = end;
                   end     = results->atf;
                   elapsed = TMR_DELTA_IN_NSECS (beg, end);
                   atfTot += elapsed;
                   atfCnt += 1;
                   if (printIt > 0) printf (" %6d", elapsed);

                   
                   if (status & DFC_M_STATUS_CAL1)
                   {
                       beg     = end;
                       end     = results->cal1;
                       elapsed = TMR_DELTA_IN_NSECS (beg, end);
                       cal1Tot += elapsed;
                       cal1Cnt += 1;
                       if (printIt > 0) printf (" %6d", elapsed);

               
                       if (status & DFC_M_STATUS_TKR)
                       {
                           beg     = end;
                           end     = results->tkr;
                           elapsed = TMR_DELTA_IN_NSECS (beg, end);
                           tkrTot += elapsed;
                           tkrCnt += 1;
                           if (printIt > 0) printf (" %6d", elapsed);
                       }
                   }
               }
           }
       }
   
       if (printIt > 0) printf ("\n");
       tot += TMR_DELTA_IN_NSECS (results->beg, end);
       results++;
   }


   /* Check if want summary block printed */
   if (opt & DFC_M_RESULTS_OPT_SUMMARY)
   {
       printf ("\n"
               "TIMING      Elapsed Time   Nevts  Per Event\n"
               "CAL0 Average: %10d / %5d = %7d nsecs\n"
               "ACD  Average: %10d / %5d = %7d nsecs\n"               
               "DIR  Average: %10d / %5d = %7d nsecs\n"               
               "ATF  Average: %10d / %5d = %7d nsecs\n"
               "CAL1 Average: %10d / %5d = %7d nsecs\n"
               "TKR0 Average: %10d / %5d = %7d nsecs\n"               
               "TOT  Average: %10d / %5d = %7d nsecs\n",
               cal0Tot, cal0Cnt, cal0Tot/(cal0Cnt != 0 ? cal0Cnt : 1),
                acdTot,  acdCnt,  acdTot/( acdCnt != 0 ?  acdCnt : 1),
                dirTot,  dirCnt,  dirTot/( dirCnt != 0 ?  dirCnt : 1),
                atfTot,  atfCnt,  atfTot/( atfCnt != 0 ?  atfCnt : 1),
               cal1Tot, cal1Cnt, cal1Tot/(cal1Cnt != 0 ? cal1Cnt : 1),
                tkrTot,  tkrCnt,  tkrTot/( tkrCnt != 0 ?  tkrCnt : 1),
                   tot,   count,    tot/ (  count != 0 ?   count : 1));

       printSizes (&sizes, count);
   }
   
       
   return 0;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn    int DFC_resultsWrite (const struct _DFC_results *results,
                               int                          count,
                               const char               *fileName)
  \brief          Writes the event-by-event result vector to a file (binary).
  \param  results The results vector to write.
  \param    count The number of elements in the results vector
  \param fileName The name of the output file.
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_resultsWrite (const struct _DFC_results *results,
                      int                          count,
                      const char               *fileName)
{
    FILE *file;
    int    idx;
 
 
    file = fopen (fileName, "wb");
    if (file == NULL)
    {
        printf ("Unable to open output file %s\n", fileName);
        return -1;
    }
    else
    {
        printf ("Opened timing file: %s\n", fileName);
    }
  
  
  
    for (idx = 0; idx < count; idx++)
    {
        int    status;
        OutRecord out;
        
        out.eventNum    = results->evtNum;
        out.status      = results->status;

        out.energy      = results->energy/ CFC_K_LEU_PER_MEV;
        out.cal0Elapsed = TMR_DELTA_IN_NSECS (results->beg,  results->cal0);
        out.acdElapsed  = TMR_DELTA_IN_NSECS (results->cal0, results->acd);
        out.atfElapsed  = TMR_DELTA_IN_NSECS (results->acd,  results->atf);
        out.cal1Elapsed = TMR_DELTA_IN_NSECS (results->atf,  results->cal1);
        out.tkrElapsed  = TMR_DELTA_IN_NSECS (results->cal1, results->tkr);

        
        out.calSize    = results->calSize >> 1;
        out.tkrSize    = results->tkrSize;
        
        status         = fwrite (&out, sizeof (out), 1, file);
        results++;
    }
    
    fclose (file);

    return 0;
}
/* ---------------------------------------------------------------------- */




static void printSizes (const DFC_resultSizes *sizes, int nevts)
{
   int                  twr;
   unsigned int temAllSize = 0;
   unsigned int calAllSize = 0;
   unsigned int tkrAllSize = 0;
   unsigned int noHdrSize  = 0;

   noHdrSize = sizes->glt + sizes->acd;
   
   
   /* Sizes of ACD and GLT contributions */
   printf ("\n"
           " Subsystem  Total Bytes   Per Event            CAL          TKR\n"
           " GLT         %10u %7u\n"
           " ACD         %10u %7u\n",           
           sizes->glt, sizes->glt/nevts,
           sizes->acd, sizes->acd/nevts);


   /* Sizes of each TEM contribution */
   for (twr = 0; twr < 16; twr++)
   {

       unsigned int temSize = sizes->tem[twr].tem;
       unsigned int calSize = sizes->tem[twr].cal;
       unsigned int tkrSize = sizes->tem[twr].tkr;

       temAllSize += temSize;
       calAllSize += calSize;
       tkrAllSize += tkrSize;
       
       printf (" TEM[%2d]     %10u %7u  %10u %7u  %10u %7u\n",
               twr,
               temSize, temSize/nevts,
               calSize, calSize/nevts,
               tkrSize, tkrSize/nevts);
   }

   noHdrSize += temAllSize;

   
   /* Summary sizes */
   printf (" TEM[all]    %10u %7u  %10u %7u  %10u %7u\n"
           " Tot[noHdr]  %10u %7u\n"
           " Total       %10u %7u\n",
           temAllSize,    temAllSize/nevts,
           calAllSize,    calAllSize/nevts,
           tkrAllSize,    tkrAllSize/nevts,
           noHdrSize,     noHdrSize/nevts,
           sizes->evt,    sizes->evt/nevts);

   return;

}

