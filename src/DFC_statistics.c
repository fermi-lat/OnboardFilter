/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  DFC_statistics.c
   \brief Provides routines to accumululate and output the summary statistics
          of the filtering process
          
   \author JJRussell - russell@slac.stanford.edu
                                                                          */
/* ---------------------------------------------------------------------- */



#include <stdio.h>
#include <string.h>
#include "DFC/DFC_status.h"
#include "DFC/DFC_statistics.h"
#include "DFC/CFC_constants.h"
          
          
#include "DFC_resultsDef.h"

#define ORDER_K_NEW_LINE 0xff
/* ---------------------------------------------------------------------- *//*!
   
  \def     ORDER_K_NEW_LINE
  \brief  Order to print a new line
                                                                          */
/* ---------------------------------------------------------------------- */  


/* ---------------------------------------------------------------------- *//*!

  \def   STAT_K_ALL
  \brief Entry to accumulate statistics for all events
                                                                          *//*!
  \def   STAT_K_ACD_NO_TILES
  \brief Entry to accumulate statistics for events with NO ACD tiles struck
                                                                          *//*!
  \def   STAT_K_ACD_NO_FILTER_TILES
  \brief Entry to accumulate statistics for events with NO ACD filter
         tiles struck. Filter tiles generally exclude the bottom
         two rows of the side faces.
                                                                          *//*!
  \def   STAT_K_ACD_FILTER_TILES
  \brief Entry to accumulate statistics for events with any filter ACD
         tiles are struck. Filter tiles generally exclude the bottom
         two rows of the side faces. This is the complement of
         STAT_K_ACD_NO_FILTER_TILES
                                                                          *//*!
  \def   STAT_K_CAL_LO_ONLY
  \brief Entry to accumulate statistics for events with only CAL_LO
         trigger bits set.
                                                                          *//*!
  \def   STAT_K_CAL_LO_ONLY_ZBOTTOM
  \brief Entry to accumulate statistics for events with only CAL_LO
         trigger bits set and which have show some connection between
         the CAL and TKR sections. Typically this has meant that there
         is some coincidence (4/6) in the 6 layers closest to the CAL.
                                                                          *//*!
  \def   STAT_K_ANY_VETOES
  \brief Entry to accumulate statistics for events with have any VETO
         bit set.
                                                                          *//*!
  \def   STAT_K_NO_VETOES
  \brief Entry to accumulate statistics for events with have no VETO
         bit set.
                                                                          */
/* ---------------------------------------------------------------------- */
#define STAT_K_ALL                 32
#define STAT_K_ACD_NO_TILES        33
#define STAT_K_ACD_NO_FILTER_TILES 34
#define STAT_K_ACD_FILTER_TILES    35
#define STAT_K_CAL_LO_ONLY         36
#define STAT_K_CAL_LO_ONLY_ZBOTTOM 37
#define STAT_K_ANY_VETOES          38
#define STAT_K_NO_VETOES           39
/* ---------------------------------------------------------------------- */


  
#ifdef __cplusplus
extern "C" {
#endif

static void printHeader (const unsigned char *caption);
    
static void printBody   (const DFC_statisticsEnergy *cnts,
                         const unsigned char titles[][20],
                         const unsigned char      *orders,
                         int                         cnt);
#ifdef __cplusplus
}
#endif
    


/* ---------------------------------------------------------------------- *//*!

  \fn      void DFC_statisticsAccumulate (DFC_statistics              *stats,
                                          const struct _DFC_results *results,
                                          int                          count)
  \brief   Accumulates the statistics from a vector of event-by-event
           filtering results
  \param   stats The statistics block to accumulate into
  \param results The event-by-event filtering results
  \param   count The number of elements in the \a results vector 
                                                                          */
/* ---------------------------------------------------------------------- */
void DFC_statisticsAccumulate (DFC_statistics              *stats,
                               const struct _DFC_results *results,
                               int                          count)
{
    int idx;
    int ecnt = 0;


    /* Loop over each result summary vector */
    while (--count >= 0)
    {

        int      ebin;
        int    status;
        int    energy;
        
        ecnt++;
        
        status = results->status;
        energy = results->energy;
        results++;

        
        /* If have a real tower trigger eliminate possible */
        if (status & DFC_M_STATUS_TKR_TRIGGER)
            status &= ~DFC_M_STATUS_TKR_POSSIBLE;
    
        
        /* Figure out which energy bin this event is in  */  
        if       (energy <=                     0 ) ebin = 0; // Unknown
        else if  (energy <= CFC_MEV_TO_LEU (   10)) ebin = 1; // 0  -    10 MEV
        else if  (energy <= CFC_MEV_TO_LEU (  300)) ebin = 2; // 10 -   300 MEV
        else if  (energy <= CFC_MEV_TO_LEU (  350)) ebin = 3; // 300 -  350 MEV
        else if  (energy <= CFC_MEV_TO_LEU (  500)) ebin = 4; // 350 -  500 MEV
        else if  (energy <= CFC_MEV_TO_LEU ( 5000)) ebin = 5; // 500 - 5000 MEV
        else                                        ebin = 6; // > 5 GEV

        
        /* Count the bits */
        for (idx = 0; idx < 32; idx++)
        {
            if (status & (1<<idx))
            {
                /* All statistics */
                stats->cnts[idx].energy[ebin]++;

                /* Only the survivors to the tracking stage */
                if ((status & DFC_M_STATUS_TKR) != 0)
                    stats->cnts[idx+64].energy[ebin]++;
            }
            
        }


        
        /* Distribution of all events */
        stats->cnts[STAT_K_ALL].energy[ebin]++;
        if ((status & DFC_M_STATUS_TKR) != 0)
            stats->cnts[STAT_K_ALL + 64].energy[ebin]++;
                
        
        /* Pick out events with no tiles hit */
        if (!(status & (DFC_M_STATUS_ACD_TOP | DFC_M_STATUS_ACD_SIDE)))
        {
            stats->cnts[STAT_K_ACD_NO_TILES].energy[ebin]++;
            if ((status & DFC_M_STATUS_TKR) != 0)
                stats->cnts[STAT_K_ACD_NO_TILES + 64].energy[ebin]++;
        }

        /* Pick out events with no veto tiles hit */
        if (!(status & (DFC_M_STATUS_ACD_TOP | DFC_M_STATUS_ACD_SIDE_FILTER)))
        {
            stats->cnts[STAT_K_ACD_NO_FILTER_TILES].energy[ebin]++;
            if ((status & DFC_M_STATUS_TKR) != 0)
                stats->cnts[STAT_K_ACD_NO_FILTER_TILES + 64].energy[ebin]++;
        }

        /* Pick out events with no filter tiles hit */
        if ((status & (DFC_M_STATUS_ACD_TOP | DFC_M_STATUS_ACD_SIDE_FILTER)))
        {
            stats->cnts[STAT_K_ACD_FILTER_TILES].energy[ebin]++;
            if ((status & DFC_M_STATUS_TKR) != 0)
                stats->cnts[STAT_K_ACD_FILTER_TILES + 64].energy[ebin]++;
        }

        /* Pick out events with only CAL LO trigger */
        if ((status & (DFC_M_STATUS_CAL_LO | DFC_M_STATUS_CAL_HI |
                       DFC_M_STATUS_TKR_POSSIBLE | DFC_M_STATUS_TKR_TRIGGER))
            == DFC_M_STATUS_CAL_LO)
        {
            stats->cnts[STAT_K_CAL_LO_ONLY].energy[ebin]++;

            /* Count number of CAL only triggers with ZBOTTOM */
            if (status & DFC_M_STATUS_ZBOTTOM)
                stats->cnts[STAT_K_CAL_LO_ONLY_ZBOTTOM].energy[ebin]++;
            
            if ((status & DFC_M_STATUS_TKR) != 0)
            {
                if (status & DFC_M_STATUS_ZBOTTOM)
                   stats->cnts[STAT_K_CAL_LO_ONLY_ZBOTTOM + 64].energy[ebin]++;
                
                stats->cnts[STAT_K_CAL_LO_ONLY + 64].energy[ebin]++;
            }
            
        }

        
        /* Count all possible vetoes */
        if ((status & DFC_M_STATUS_VETOES))
        {
           stats->cnts[STAT_K_ANY_VETOES].energy[ebin]++;
            if ((status & DFC_M_STATUS_TKR) != 0)
                stats->cnts[STAT_K_ANY_VETOES + 64].energy[ebin]++;
        }
        else
        {
           stats->cnts[STAT_K_NO_VETOES].energy[ebin]++;
            if ((status & DFC_M_STATUS_TKR) != 0)
                stats->cnts[STAT_K_NO_VETOES + 64].energy[ebin]++;
        }
        

    }
    
    
    /* Get the totals, ie sum over the energy bins, into the last bin */
    for (idx = 0; idx < 128; idx++)
    {
        int edx;
        
        for (edx = 0; edx < 7; edx++)
        {
            stats->cnts[idx].energy[7] += stats->cnts[idx].energy[edx];
        }
    }
    
    return;
}
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \fn          void DFC_statisticsClear (DFC_statistics  *stats)
  \brief       Clears the statistics block
  \param stats The statistics block to clear.
                                                                          */
/* ---------------------------------------------------------------------- */
void DFC_statisticsClear (DFC_statistics  *stats)
{
   memset (stats, 0, sizeof (*stats));
   return;
}
/* ---------------------------------------------------------------------- */
            



/* ---------------------------------------------------------------------- *//*!

  \fn          void DFC_statisticsPrint (const DFC_statistics *stats)
  \brief       Prints the statistics block in ASCII format to standard output
  \param stats The statistics block to print.
                                                                          */
/* ---------------------------------------------------------------------- */  
void DFC_statisticsPrint (const DFC_statistics *stats)
{
 
   /*  Table of the titles for each summary field */
   static const unsigned char Titles[][20] =
   {
      "ACD  Evaluation....",   /*  0 */
      "DIR  Decoded.......",   /*  1 */      
      "ATF  Evaluation....",   /*  2 */
      "CAL1 Evaluation....",   /*  3 */
      "TKR  Finding.......",   /*  4 */
      "ACD  Top  Tile.....",   /*  5 */
      "ACD  Side Tile.....",   /*  6 */
      "ACD  Side Veto Tile",   /*  7 */
      "Possible...........",   /*  8 */
      "Trigger............",   /*  9 */
      "Cal LO Trigger.....",   /* 10 */      
      "Cal Hi Trigger.....",   /* 11 */
      "Tracks == 1........",   /* 12 */
      "Tracks >= 2........",   /* 13 */
      "TKR Throttle.......",   /* 14 */
      "TKR < 2, E < 350Mev",   /* 15 */
      "Skirt region.......",   /* 16 */      
      "Tracks == 0........",   /* 17 */      
      "Track/Row 2   Acd..",   /* 18 */
      "Track/Row 0,1 Acd..",   /* 19 */      
      "Track/Top     Acd..",   /* 20 */
      "Zbottom............",   /* 21 */
      "Cal E0/Etot   > .90",   /* 22 */
      "Cal E0/Etot   < .01",   /* 23 */
      "ACD Side Tile Veto.",   /* 24 */
      "ACD Top  Tile Veto.",   /* 25 */
      "ACD Splash Veto 1..",   /* 26 */
      "Cal<350Mev + V Tile",   /* 27 */
      "Cal 0 Energy + Tile",   /* 28 */            
      "ACD Splash Veto 0..",   /* 29 */
      "NoCalLo + V Tile...",   /* 30 */
      "Vetoed.............",   /* 31 */      
      "ALL................",   /* 32 */      
      "ACD =0 Tiles Hit...",   /* 33 */
      "ACD =0 Veto Tiles..",   /* 34 */
      "ACD >0 Veto Tiles..",   /* 35 */
      "CAL LO Trigger only",   /* 36 */
      "CAL LO only/ZBOT...",   /* 37 */
      "Any veto...........",   /* 38 */
      "No  vetoes........."    /* 39 */
   };


   /* Indicates the order that the fields should be printed */
   static unsigned char AllOrder[] =
   {
      STAT_K_ALL,
      DFC_V_STATUS_ACD_TOP,
      DFC_V_STATUS_ACD_SIDE,
      DFC_V_STATUS_ACD_SIDE_FILTER,
      STAT_K_ACD_FILTER_TILES,      
      STAT_K_ACD_NO_TILES,
      STAT_K_ACD_NO_FILTER_TILES,
      ORDER_K_NEW_LINE,

      DFC_V_STATUS_CAL_LO,
      STAT_K_CAL_LO_ONLY,
      DFC_V_STATUS_CAL_HI,
      DFC_V_STATUS_TKR_POSSIBLE,
      DFC_V_STATUS_TKR_TRIGGER,
      DFC_V_STATUS_TKR_THROTTLE,
      ORDER_K_NEW_LINE,

      /* ACD prefilter */
      STAT_K_ALL,
      DFC_V_STATUS_NOCALLO_FILTER_TILE,
      DFC_V_STATUS_SPLASH_0,   
      ORDER_K_NEW_LINE,
      
      /* ACD only based cuts */
      DFC_V_STATUS_ACD,
      DFC_V_STATUS_E350_FILTER_TILE,      
      DFC_V_STATUS_E0_TILE,
      DFC_V_STATUS_SPLASH_1,

      ORDER_K_NEW_LINE,

      /* ACD/TKR based cuts */
      DFC_V_STATUS_ATF,      
      DFC_V_STATUS_TOP,
      DFC_V_STATUS_SIDE,
      DFC_V_STATUS_ZBOTTOM,
      STAT_K_CAL_LO_ONLY_ZBOTTOM,
      ORDER_K_NEW_LINE,

      /* CAL based cuts */
      DFC_V_STATUS_CAL1,      
      DFC_V_STATUS_EL0_ETOT_01,
      DFC_V_STATUS_EL0_ETOT_90,
      ORDER_K_NEW_LINE,
      
      /* What's vetoed, whats being passed on to the next stage */
      DFC_V_STATUS_TKR   
   };

   static unsigned char SurvivorOrder[] =
   {
      STAT_K_ALL,
      DFC_V_STATUS_CAL_LO,
      STAT_K_CAL_LO_ONLY,
      DFC_V_STATUS_CAL_HI,
      DFC_V_STATUS_TKR_POSSIBLE,
      DFC_V_STATUS_TKR_TRIGGER,
      DFC_V_STATUS_TKR_THROTTLE,      
      ORDER_K_NEW_LINE,
      
      DFC_V_STATUS_ACD_TOP,
      DFC_V_STATUS_ACD_SIDE,
      DFC_V_STATUS_ACD_SIDE_FILTER,
      STAT_K_ACD_FILTER_TILES,      
      STAT_K_ACD_NO_TILES,
      STAT_K_ACD_NO_FILTER_TILES,
      ORDER_K_NEW_LINE,
      
      DFC_V_STATUS_TKR_EQ_0,
      DFC_V_STATUS_TKR_EQ_1,
      DFC_V_STATUS_TKR_GE_2,
      DFC_V_STATUS_TKR_TOP,
      DFC_V_STATUS_TKR_ROW01,
      DFC_V_STATUS_TKR_ROW2,
      DFC_V_STATUS_TKR_SKIRT,
      DFC_V_STATUS_TKR_LT_2_ELO,
      STAT_K_ANY_VETOES,
      STAT_K_NO_VETOES
   };
   
   
       
   

   printHeader ("All");
   printBody   (&stats->cnts[0],
               Titles,
               AllOrder,
               sizeof (AllOrder) / sizeof (AllOrder[0]));


   printf ("\n");
   printHeader ("Survivors");
   printBody   (&stats->cnts[64],
               Titles,
               SurvivorOrder,
               sizeof (SurvivorOrder) / sizeof (SurvivorOrder[0]));
   
   return;
               
}
/* ---------------------------------------------------------------------- */



   
/* ---------------------------------------------------------------------- *//*!

  \fn            void printHeader (const unsigned char *caption)
  \brief         Prints the header line of the statistics table
  \param caption A short (< 9 characters) description of the statistics
                 being described.
                                                                          */
/* ---------------------------------------------------------------------- */  
static void printHeader (const unsigned char *caption)
{
   printf ("\n"
           "Quantity(%9.9s)"
           "   NONE <10Mev 300Mev 350Mev 500Mev  5 Gev > 5Gev Totals\n"
           "-------------------"
           " ------ ------ ------ ------ ------ ------ ------ ------\n",
           caption);
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn  void printBody (const DFC_statisticsEnergy *cnts,
                       const unsigned char titles[][20],
                       const unsigned char      *orders,
                       int                          cnt)
  \brief        Prints the body of the statistics table.
  \param cnts   The array of statistics to print.
  \param titles A set of titles to be associated with the statistics.
  \param orders A series of orders selecting which statistics are
                to be printed from the \a cnts parameter and, by
                implication, the title string from the \a titles
                parameter. It also contains meta-commands like new
                line orders to aid in the readability
  \param cnt    The number of orders in the \a orders array.
                                                                          */
/* ---------------------------------------------------------------------- */   
static void printBody (const DFC_statisticsEnergy *cnts,
                       const unsigned char titles[][20],
                       const unsigned char      *orders,
                       int                          cnt)
{
   int idx;
 
 
   /* Print the statistics table by 'Order' */
   for (idx = 0; idx < cnt; idx++)
   {
       int jdx;
       int edx;

       jdx = orders[idx];

       if (jdx != ORDER_K_NEW_LINE)
       {
             
           printf (titles[jdx]);
           
           for (edx = 0; edx < 8; edx++)
           {
               int value = cnts[jdx].energy[edx];
               if (value) printf (" %6d", value);
               else       printf ("      .");
           }
       }

       printf ("\n");
   }

   
   return;
}
/* ---------------------------------------------------------------------- */
