/* ---------------------------------------------------------------------- */
/*!
   
   \file   DFC_rto.c
   \brief  Utility routines to parse the command line parameters and 
           fill in the run time filter options, implementation.
   \author JJRussell - russell@slac.stanford.edu
   
\verbatim

  CVS $Id
\endverbatim
    
    There is really no good reason why this code is not included directly
    in the filter.c code other than it was getting unwieldy large.
                                                                          */
/* ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

#include "DFC/EDM.h"
#include "DFC_rto.h"
#include "DFC/DFC_ss.h"
#include "DFC/DFC_status.h"
//#include "DFC/DFC_results.h"

#ifdef   __linux
#include <getopt.h>
#endif




#ifndef VXWORKS

static unsigned int     getSummaryOpts (const char *opts);
static unsigned int     getPrintOpts   (const char *opts);

#ifdef EDM_USE
static MessageObjLevels getMessageOpts   (const char     *opts, 
                                          MessageObjLevels lvl);
#endif


/* ---------------------------------------------------------------------- */
/*!

  \fn        int DFC_rtoFill (DFC_rto *rto, int argc, char *argv[])
  \brief         Examines the command line parameters and fills the print
                 control structure.
  \retval     0  Everything is okay.
  \retval    -1  Incorrect command usage. Usually this means that the
                 input file name was absent.

  \param   rto  The runtime options structure to fill.
  \param   argc The number of command line parameters.
  \param   argv The array of pointers to the command line parameters.


  \par Usage and Warnings
   This routine uses 'getopt' to process the command line parameters.
   This is routine does not provide for lot of flexibility. It rigidly
   demands that all optional arguments, i.e. the -x type must precede
   the command line parameters (here the input file name). Getopt
   provides no positive binding, so all command line options which can
   accept an option must be provided with one. For instance

  \code

      $> filter -n10 -s -PACG my_file

  \endcode


   The \e -s parameter is without a value, well kind of. Its, value,
   whether intentional or not, is \e my_file. This behaviour makes
   providing different defaults whether the option is specified or not
   specified, impossible. (Here a reasonable interpretation is to set
   the skip count to 1, rather than 0 if it wasn't specified.)

  \par List options
   
   In order to make the best use of the limited number of available
   options ([a-zA-Z0-9], some options introduce a list. What follows
   the list is another set of single character options, but these 
   options are interpreted in the context of that list. This allows 
   characters to be reused.

   The current set of lists are

    - M = Message Level List
    - P = Print List
    - S = Summary List

  \par Message Level List
   A message level list -M options is followed by one or more characters
   from one of the two sets. The first set establishes the current 
   message level, the second character or characters identifies a target
   that participates in the message services. The current message level 
   is applied to any and all targets that follow, until a new current 
   message level is establish. The message levels are:

        - D = DEBUG
        - I = INFO
        - W = WARNING
        - E = ERROR
        - F = FATAL

   The list of targets is

        - x = all
        - d = DFC_filter, driver routine
        - p = TFC projection tower find
        - s = TFC skirt, track skirt projections
        - f = filter
        - v = TFC_acd, track/acd match

   Example

  \code
        -MDpsIfv
  \endcode

   This indicates the TFC projection tower finding and the TFC skirt 
   matching routines should issue messages at the DEBUG level or higher
   and that the filter and TFC ACD matching routines should issue messages
   at the INFO level or higher. Hey, it ain't pretty, but what you gonna
   do with such a brain-dead facility?

  \par Print List Options
   The Print list options are

        - X = all
        - A = Acd
        - C = Cal
        - G = Gem
        - T = Tkr
        - P = Projections
        - g = geometry

   Example
  \code
        -PACPg
  \endcode

   This displays information about the ACD, CAL and found Projections 
   along with a dump of the geometry.

  \par Summary List Options

   The summary list options are

        - e = short event summary
        - E = Event-by-Event summary

  \code
       -Se
  \endcode

   This requests a short event summary
   
                                                                          */
/* ---------------------------------------------------------------------- */
int DFC_rtoFill (DFC_rto *rto, int argc, char *argv[])
{
   extern int     optind;
   extern char   *optarg;
   const char     *ofile;
   char                c;
   int           nevents;
   int            nprint;
   int             nskip;
   int             quiet;
   int              list;
   unsigned int   vetoes;
   unsigned int       ss;
   unsigned int esummary;
   unsigned int geometry;
   TFC_geoId      geo_id;

   EDM_CODE(
   EDM_level      dlevel;
   MessageObjLevels 
                  levels;)
   
   
   /* Establish the defaults */   
   nevents  = -1;
   ofile    =  NULL;
   nskip    =  0;
   nprint   =  0;
   ss       =  0;
   quiet    =  0;
   list     =  0;
   esummary =  0;
   geometry =  0;
   geo_id   = TFC_K_GEO_ID_DEFAULT;
   vetoes   = 0;


   EDM_CODE
   (
      /* Establish the default message level */
      dlevel            = EDM_K_FATAL;   
      levels.filter     = dlevel;
      levels.dfc_filter = dlevel;
      levels.tfc_ptf    = dlevel;
      levels.tfc_acd    = dlevel;
      levels.tfc_skirt  = dlevel;
   )
      

   /* Process the command line options */
   /*while ((c = getopt (argc, argv, "lqg:o::n:s:p:v:P:M:S:")) != (char)EOF)
   {
       switch (c)
       {
           case 'l':  { list      = 1;                         break; }
           case 'q':  { quiet     = 1;                         break; }
           case 'g':  { geo_id    = atoi (optarg);             break; }
           case 'o':  { ofile     = optarg;                    break; }
           case 'n':  { nevents   = atoi (optarg);             break; }
           case 's':  { nskip     = atoi (optarg);             break; }
           case 'p':  { nprint    = atoi (optarg);             break; }
           case 'v':  { vetoes    = atoi (optarg);             break; }
           case 'P':  { ss        = getPrintOpts   (optarg);   break; }
           case 'S':  { esummary  = getSummaryOpts (optarg);   break; }
 
	   EDM_CODE(
           case 'M':  { levels    = getMessageOpts (optarg, 
                                                    levels);   break; }
           )
       }
   }*/
          
   
   /* After processing all the options, make sure have an input file left */
   //if (optind >= argc) return -1;


   /* Pick off one time initialization */
   geometry = ss & FLT_M_GEO;


   /* If nothing selected, default to all */
   if (ss == 0) ss = -1;

   
   /* Fill in the control structure */
   rto->ifile       = "unused";
   rto->ofile       = ofile;
   rto->data        = NULL;
   rto->to_process  = nevents;
   rto->to_skip     = nskip;
   rto->to_print    = nprint;
   rto->ss_to_print = ss;
   rto->quiet       = quiet;
   rto->geo_id      = TFC_K_GEO_ID_DEFAULT;
   rto->list        = list;
   rto->vetoes      = vetoes;
   rto->esummary    = esummary;
   rto->geometry    = geometry;
   rto->geo_id      = geo_id;   

   EDM_CODE(rto->levels = levels);

   return 0;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
#ifdef EDM_USE
/* ---------------------------------------------------------------------- */
/*!

  \fn          MessageObjLevels getMessageOpts (const char     *opts,
                                                MessageObjLevels lvl)
  \brief       Extracts the message options
  \return      Bit mask of the print options
  \param opts  Character string of the options
  \param def   Current set of levels

								          */
/* ---------------------------------------------------------------------- */
static MessageObjLevels getMessageOpts (const char     *opts,
					MessageObjLevels lvl)
{
  char              opt;
  unsigned int level = EDM_K_FATAL;
  int last_was_level = 0;

  /* Run through the list of option characters */
   while ((opt = *opts++))
   {
     switch (opt)
     {
        /* This is the set of message level specifiers */
        case 'D':  { level = EDM_K_DEBUG;    last_was_level = 1; break; }
        case 'I':  { level = EDM_K_INFO;     last_was_level = 1; break; }
        case 'W':  { level = EDM_K_WARNING;  last_was_level = 1; break; }
        case 'E':  { level = EDM_K_ERROR;    last_was_level = 1; break; }
        case 'F':  { level = EDM_K_FATAL;    last_was_level = 1; break; }

        /* This is the set of objects to apply a message level to */

        case 'd':  { lvl.dfc_filter = level; last_was_level = 0; break; }
        case 'p':  { lvl.tfc_ptf    = level; last_was_level = 0; break; }
        case 's':  { lvl.tfc_skirt  = level; last_was_level = 0; break; }
        case 'f':  { lvl.filter     = level; last_was_level = 0; break; }
        case 'v':  { lvl.tfc_acd    = level; last_was_level = 0; break; }

        case 'x':  { /* This is the everything option */
	             lvl.filter     = level;
                     lvl.dfc_filter = level;
                     lvl.tfc_ptf    = level;
		     lvl.tfc_acd    = level;
                     lvl.tfc_skirt  = level;
		     last_was_level = 0;
                     break;                      
	           }
     }
   }


   /* 
    | If last option was a level, set everything to that level.
    | This just allows one to shorthand, i.e. -MI, would set all
    | message levels to EDM_K_INFO. It is entirely equivalent to -MIx
   */
   if (last_was_level)
   {
     lvl.filter     = level;
     lvl.dfc_filter = level;
     lvl.tfc_ptf    = level;
     lvl.tfc_acd    = level;
     lvl.tfc_skirt  = level;
   }

   return lvl;
}
/* ---------------------------------------------------------------------- */
#endif
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

  \fn          unsigned int getPrintOpts (const char *opts)
  \brief       Extracts the print options
  \return      Bit mask of the print options
  \param opts  Character string of the options

								          */
/* ---------------------------------------------------------------------- */
static unsigned int getPrintOpts (const char *opts)
{
   char opt;
   unsigned int ss = 0;

   while ((opt = *opts++))
   {
     switch (opt)
     {
           case 'A':  { ss       |= DFC_M_SS_ACD;              break; }
           case 'C':  { ss       |= DFC_M_SS_CAL;              break; }
           case 'G':  { ss       |= DFC_M_SS_GEM;              break; }
           case 'T':  { ss       |= DFC_M_SS_TKR;              break; }
           case 'P':  { ss       |= FLT_M_PRJS;                break; }
           case 'g':  { ss       |= FLT_M_GEO;                 break; }    
     }
   }

   return ss;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

  \fn          unsigned int getSummaryOpts (const char *opts)
  \brief       Extracts the event summary options
  \return      Bit mask of the display options
  \param opts  Character string of the options

								          */
/* ---------------------------------------------------------------------- */
static unsigned int getSummaryOpts (const char *opts)
{
   char opt;
   unsigned int esummary = 0;
/*
   while ((opt = *opts++))
   {
     switch (opt)
     {
           case 'e':  { esummary |= DFC_M_RESULTS_OPT_SUMMARY; break; }
           case 'E':  { esummary |= DFC_M_RESULTS_OPT_EVENT;   break; }
     }
   }
*/
   return esummary;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

 \fn     void DFC_rtoUsageReport (void)
 \brief  Describes the command line parameters for daqeb_print.
                                                                          */
/* ---------------------------------------------------------------------- */
void DFC_rtoUsageReport (void)
{
   printf (
 "Usage:\n"
 "prompt> filter -o <out_file_name> -n <count> -s <skip> <in_file_name>\n"
 "       in_file_name: The name of the data file to analzye, required\n"
 "         -n <count>: The number of events to analyze, optional\n"
 "         -s  <skip>: The number of events to initially skip, optional\n"
 "         -p <count>: The number of events to print the print list\n"
 "         -S <slist>: Results summary options, where slist is\n"
 "                     e : Brief summary\n"
 "                     E : Event-by-event summary\n"
 "         -g    <id>: Geometry id\n"
 "         -v  <mask>: Set the mask of vetoes, 0 = pass through mode\n"
 "         -l        : Print list of events not vetoed\n"
 "         -q        : Minimal summary information\n"
 "         -P <plist>: Print list where plist is a list of the following:\n"
 "                     A : Print ACD display\n"
 "                     C : Print CAL display\n"
 "                     G : Print GEM display\n"
 "                     T : Print TKR displaydata\n"
 "                     P : Print TKR projections\n"
 "                     g : Print the geometry\n"
 "         -M <mlist>: Sets the message level, this is an option draw\n"
 "                     from two sets, a set of message levels and a\n"
 "                     set of objects. A message level specifier must\n"
 "                     be the first option, followed by any number of\n"
 "                     objects which should have their message level\n" 
 "                     set to that message level\n"
 "                       Message Levels       Objects\n"
 "                       D : debug            f : the control program\n"
 "                       I : informational    d : the driver program\n"
 "                       W : warning          p : Tracker projection finding\n"
 "                       E : error            v : Tracker/Acd matching\n"
 "                       F : fatal            s : Tracker/Skirt matching\n"
 "\n");
}
/* ---------------------------------------------------------------------- */
#endif
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

  \fn        void DFC_rtoPrint (const DFC_rto *rto)
  \brief     Prints the contents of the print control context.
  \param rto Pointer to the print control context block.
                                                                          */
/* ---------------------------------------------------------------------- */
void DFC_rtoPrint (const DFC_rto *rto)
{
   printf ("Input  file: %s\n",  rto->ifile);
 
   if (rto->to_process < 0) printf ("Processing : All\n");
   else                     printf ("Processing : %d\n", rto->to_process);

   
   if (rto->to_skip   == 0) printf ("Skipping   : None\n");
   else                     printf ("Skipping   : %d\n", rto->to_skip);

      
   if (rto->to_print  == 0) printf ("Printing   : None\n");
   else                     printf ("Printing   : %d\n", rto->to_print);

   if (rto->geo_id    == 0) printf ("Geometry Id: Default\n");
   else                     printf ("Geometry Id: %d\n");
   
   printf ("Veto Mask  : %8.8x\n", rto->vetoes);

}
/* ---------------------------------------------------------------------- */




