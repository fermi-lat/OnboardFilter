/* ---------------------------------------------------------------------- *//*!
   
   \file  DFC_display.c
   \brief Crude Ascii display of ACD and TKR data
   \author JJRussell - russell@slac.stanford.edu

    This file's routines are documented in the header file. It is far
    to painful to document here. There is a large number of internal
    #defines's which only have meaning within this file. It would add
    nothing to publish these internal details.

\verbatim

  CVS $Id
\endverbatim
    
                                                                          */
/* ---------------------------------------------------------------------- */


#include <stdio.h>
#include "ffs.h"
#include "DFC/DFC_display.h"
#include "DFC/EBF_cid.h"
#include "DFC/EBF_glt.h"
#include "DFC/EBF_tkr.h"
#include "DFC/EBF_locate.h"
#include "DFC/EBF_directory.h"

#include "DFC/DFC_latRecord.h"
#include "DFC/CFC_latRecordPrint.h"
#include "DFC/TFC_latRecordPrint.h"


#include "DFC/DFC_ctl.h"


#ifdef __cplusplus
extern "C" {
#endif



/* ------------------------------------------------------------------------- *\

   +-----------------------------------------------------------------------+ 
   |                                                                       |
   | This code creates a picture of the ACD and TKR that looks roughly     |
   | like that depicted below. All lines are trimmed, ie there are no      |
   | trailing spaces. The axis are provided to read off the row and column |
   | numbers.                                                              |
   |                                                                       |   
   +-----------------------------------------------------------------------+
   
             1         2         3         4         5         6         7    
   01234567890123456789012345678901234567890123456789012345678901234567890123
 0                                                     +-+-+-+-+      
 1               +---------+                          0|       |0     
 2             3 |         |                          2|       |2    
 3               +---------+                          4|       |4    
 4             2 | | | | | |                          6|       |6    
 5               +-+-+-+-+-+                          8|       |8    
 6             1 | | | | | |                          a|       |a    
 7               +-+-+-+-+-+                          c|       |c    
 8             0 | | | | | |              +---------+ e|       |e +---------+ 
 9               +-+-+-+-+-+              |         | g|       |g |         |
10     3 2 1 0                 0 1 2 3    +---------+  +-+-+-+-+  +---------+ 
11    +-+-+-+-+  +-+-+-+-+-+  +-+-+-+-+    02468aceg    c d e f    geca86420
12  4 | | | | |  | | | | | |  | | | | |   +---------+  +-+-+-+-+  +---------+
13    | |-+-+-+  +-+-+-+-+-+  +-+-+-| |   |         | c| | | | |f |         |
14  3 | | | | |  | | | | | |  | | | | |   +         +  +-+-+-+-+  +         +
15    | |-+-+-+  +-+-+-+-+-+  +-+-+-| |   |         | 8| | | | |b |         |
16  2 | | | | |  | | | | | |  | | | | |   +         +  +-+-+-+-+  +         +
17    | |-+-+-+  +-+-+-+-+-+  +-+-+-| |   |         | 4| | | | |7 |         |
18  1 | | | | |  | | | | | |  | | | | |   +         +  +-+-+-+-+  +         +
19    | |-+-+-+  +-+-+-+-+-+  +-+-+-| |   |         | 0| | | | |3 |         |
20  0 | | | | |  | | | | | |  | | | | |   +---------+  +-+-+-+-+  +---------+
21    +-+-+-+-+  +-+-+-+-+-+  +-+-+-+-+    02468aceg    0 1 2 3    geca86420
22                                        +---------+  +-+-+-+-+  +---------+ 
23               +-+-+-+-+-+              |         | g|       |g |         |
24             0 | | | | | |              +---------+ e|       |e +---------+ 
25               +-+-+-+-+-+                          c|       |c 
26             1 | | | | | |                          a|       |a 
27               +-+-+-+-+-+                          8|       |8 
28             2 | | | | | |                          6|       |6 
29               +---------+                          4|       |4 
30             3 |         |                          2|       |2 
31               +---------+                          0|       |0 
32                                                     +-+-+-+-+
\* ------------------------------------------------------------------------- */


/*
 | Define the ASCII characters used for making boxes
*/
typedef enum _Ascii
{
   /* ---------------------------------------------------- */
   /*                  SINGLES                             */
   /*                          CORNERS TL, TR, BR, BL      */
   STSL  = 0xda,  /*!< Single Top,    Single Left  corner  */
   STSR  = 0xbf,  /*!< Single Top,    Single Right         */
   SBSR  = 0xd9,  /*!< Single Bottom, Single Right corner  */
   SBSL  = 0xc0,  /*!< Single Bottom, Single Left corner   */ 

   /*                         TEES    T, R, B, L           */
   STST  = 0xc2,  /*!< Single Top,    Single Tee           */
   SRST  = 0xb4,  /*!< Single Right,  Single Tee           */
   SBST  = 0xc1,  /*!< Single Bottom, Single Tee           */
   SLST  = 0xc3,  /*!< Single Left,   Single Tee           */

   /*                         BARS    V, H, CROSS          */ 
   SVB   = 0xb3,  /*!< Single Vertical Bar                 */
   SHB   = 0xc4,  /*!< Single Horizontal Bar               */
   SX    = 0xc5,  /*!< Single Cross                        */
   /* ---------------------------------------------------- */

   /* ---------------------------------------------------- */
   /*                  SINGLE, DOUBLES                     */
   DTSL  = 0xd5,  /*!< Double Top   , Single Left  corner  */
   DTSR  = 0xb8,  /*!< Double Top   , Single Right corner  */
   DBSR  = 0xbe,  /*!< Double Bottom, Single Right corner  */
   DBSL  = 0xd4,  /*!< Double Bottom, Single Left  corner  */

   SRDT  = 0xb5,  /*!< Single Right , Double Tee           */
   SLDT  = 0xc6,  /*!< Single Left  , Double Tee           */


   STDL  = 0xd6,  /*!< Single Top,  , Double Left  corner  */
   STDR  = 0xb7,  /*!< Single Top   , Double Right corner  */
   SBDR  = 0xbd,  /*!< Single Bottom, Double Right corner  */
   SBDL  = 0xd3,  /*!< Single Bottom, Double Left  corner  */

   STDT  = 0xd2,  /*!< Single Top   , Double Tee           */
   SBDT  = 0xd0,  /*!< Single Top   , Double Tee           */


                  /*!< TEES, DOUBLE BORDERS, SINGLE TEES   */
   DTST  = 0xd1,  /*!< Double Top   , Single Tee           */
   DRST  = 0xb6,  /*!< Double Right, Single Tee            */
   DBST  = 0xcf,  /*!< Double Bottom, Single Tee           */
   DLST  = 0xc7,  /*!< Double Left, Single Tee             */


   /* ---------------------------------------------------  */
   /*                  DOUBLES - DOUBLES TL, TR, BR, BL    */
   DTDL  = 0xc9,  /*!< Double Top,    Double Left  corner  */
   DTDR  = 0xbb,  /*!< Double Top   , Double Right corner  */
   DBDR  = 0xbc,  /*!< Double Bottom, Double Right corner  */
   DBDL  = 0xc8,  /*!< Double Bottom, Double Left  corner  */

                  /*   TEES, BARS and CROSSES              */
   DTDT  = 0xcb,  /*!< Double Top   , Double Tee           */
   DRDT  = 0xb9,  /*!< Double Right , Double Tee           */
   DBDT  = 0xca,  /*!< Double Bottom, Double Tee           */
   DLDT  = 0xcc,  /*!< Double Left  , Double Tee           */

   DVST  = 0xd7,  /*!< Double Vertical  , Single Tee       */
   DHST  = 0xd8,  /*!< Double Horizontal, Single Tee       */
   DVB   = 0xba,  /*!< Double Vertical Bar                 */
   DHB   = 0xcd,  /*!< Double Horizontal Bar               */
   DX    = 0xce,  /*!< Double Cross                        */
   /* ---------------------------------------------------- */


   /* ---------------------------------------------------- */
   /*                  BOXES                               */
   FB    = 0xdb,  /*!< Full Box                            */
   SB    = 0xfe,  /*!< Small Box                           */
   LGB   = 0xb0,  /*!< Light Gray Box                      */
   MGB   = 0xb1,  /*!< Medium Gray Box                     */
   DGB   = 0xb2,  /*!< Dark   Gray Box                     */
   BHB   = 0xdc,  /*!< Bottom Half Box                     */
   LHB   = 0xdd,  /*!< Left   Half Box                     */
   RHB   = 0xde,  /*!< Right  Half Box                     */
   THB   = 0xdf,  /*!< Top    Half Box                     */
   /* ---------------------------------------------------- */

   /* ---------------------------------------------------- */
   /*                 ODDS and ENDS                        */
   MU    = 0xbd,  /*!< Greek mu                            */
   MU1   = 0xd0,  /*!< Greek mu, looks like 0xbd           */
   /* ---------------------------------------------------- */

   SP    = ' ',   /*!< Space                               */
   NL    = '\n'   /*!< New Line                            */
} Ascii;

/*
 |  These define the graphical characters used to create the picture. They
 |  are full in the sense that they also logically specify the components
 |  of the picture. These characters can be substituted with other characters.
 |  For instance the corner characters can all be changed to a '+'. The
 |  set being used specifies these from the graphical character set.
*/

#define VB SVB   /* Vertical bar        */
#define HB SHB   /* Horizontal bar      */

#define TL STSL  /* Top    left  corner */
#define TR STSR  /* Top    right corner */
#define BR SBSR  /* Bottom right corner */
#define BL SBSL  /* Bottom left  corner */

#define TT STST  /* Top    tee          */
#define RT SRST  /* Right  tee          */
#define BT SBST  /* Bottom tee          */
#define LT SLST  /* Left   tee          */
#define _X  SX   /* Cross               */

/*
 | If the character set can only render a cross as a tee, it may be useful
 | to use just a horizontal or vertical bar. These must be translated
*/
#define Tt 1     /* Top    tee, optional */
#define Rt 2     /* Right  tee, optional */
#define Bt 3     /* Bottom tee, optional */
#define Lt 4     /* Left   tee, optional */


#define C0 '0'
#define C1 '1'
#define C2 '2'
#define C3 '3'
#define C4 '4'
#define C5 '5'
#define C6 '6'
#define C7 '7'
#define C8 '8'
#define C9 '9'

#define Ca 'a'
#define Cb 'b'
#define Cc 'c'
#define Cd 'd'
#define Ce 'e'
#define Cf 'f'
#define Cg 'g'
#define Ch 'h'

#define SP ' '
#define NL '\n'


/* Define shorthand notation for a bunch of spaces, these come in handy */
#define _1   SP
#define _2   _1,  _1
#define _3   _1,  _1,  _1
#define _4   _1,  _1,  _1,  _1
#define _5   _1,  _1,  _1,  _1,  _1
#define _6   _1,  _1,  _1,  _1,  _1, _1
#define _7   _1,  _1,  _1,  _1,  _1, _1, _1
#define _8   _1,  _1,  _1,  _1,  _1, _1, _1, _1
#define _9   _1,  _1,  _1,  _1,  _1, _1, _1, _1, _1
#define _10  _1,  _1,  _1,  _1,  _1, _1, _1, _1, _1, _1
#define _11 _10,  _1
#define _12 _10,  _2
#define _13 _10,  _3
#define _14 _10,  _4
#define _15 _10,  _5
#define _16 _10,  _6,
#define _17 _10,  _7,    
#define _18 _10,  _8,
#define _19 _10,  _9,
#define _20 _10, _10,
#define _21 _10, _10, _1 
#define _22 _10, _10, _2
#define _23 _10, _10, _3    
#define _24 _10, _10, _4
#define _25 _10, _10, _5    


/* Defines the ACD Y+ FACE display                      */
#define ACDT__0 SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP  /*             */
#define ACDT__1 TL, HB, HB, HB, HB, HB, HB, HB, HB, HB, TR  /* +---------+ */
#define ACDT__2 VB, SP, SP, SP, SP, SP, SP, SP, SP, SP, VB  /* |         | */
#define ACDT__3 LT, HB, Tt, HB, Tt, HB, Tt, HB, Tt, HB, RT  /* +---------+ */
#define ACDT__4 VB, SP, VB, SP, VB, SP, VB, SP, VB, SP, VB  /* | | | | | | */ 
#define ACDT__5 LT, HB, _X, HB, _X, HB, _X, HB, _X, HB, RT  /* +-+-+-+-+-+ */
#define ACDT__6 VB, SP, VB, SP, VB, SP, VB, SP, VB, SP, VB  /* | | | | | | */
#define ACDT__7 LT, HB, _X, HB, _X, HB, _X, HB, _X, HB, RT  /* +-+-+-+-+-+ */
#define ACDT__8 VB, SP, VB, SP, VB, SP, VB, SP, VB, SP, VB  /* | | | | | | */
#define ACDT__9 BL, HB, BT, HB, BT, HB, BT, HB, BT, HB, BR  /* +-+-+-+-+-+ */


/* Defines the ACD X- FACE display              */
#define ACDL_10 SP, C3, SP, C2, SP, C1, SP, C0, SP          /*  3 2 1 0  */
#define ACDL_11 TL, HB, TT, HB, TT, HB, TT, HB, TR          /* +-+-+-+-+ */
#define ACDL_12 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define ACDL_13 VB, SP, Lt, HB, _X, HB, _X, HB, RT          /* | +-+-+-+ */
#define ACDL_14 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define ACDL_15 VB, SP, Lt, HB, _X, HB, _X, HB, RT          /* | +-+-+-+ */
#define ACDL_16 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define ACDL_17 VB, SP, Lt, HB, _X, HB, _X, HB, RT          /* | +-+-+-+ */
#define ACDL_18 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define ACDL_19 VB, SP, Lt, HB, _X, HB, _X, HB, RT          /* | +-+-+-+ */
#define ACDL_20 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define ACDL_21 BL, HB, BT, HB, BT, HB, BT, HB, BR          /* +-+-+-+-+ */
#define ACDL_22 SP, SP, SP, SP, SP, SP, SP, SP, SP          /*           */ 

    
/* Defines the ACD TOP FACE display                     */
#define ACDC_10 SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP  /*             */
#define ACDC_11 TL, HB, TT, HB, TT, HB, TT, HB, TT, HB, TR  /* +-+-+-+-+-+ */
#define ACDC_12 VB, SP, VB, SP, VB, SP, VB, SP, VB, SP, VB  /* | | | | | | */
#define ACDC_13 LT, HB, _X, HB, _X, HB, _X, HB, _X, HB, RT  /* +-+-+-+-+-+ */
#define ACDC_14 VB, SP, VB, SP, VB, SP, VB, SP, VB, SP, VB  /* | | | | | | */
#define ACDC_15 LT, HB, _X, HB, _X, HB, _X, HB, _X, HB, RT  /* +-+-+-+-+-+ */
#define ACDC_16 VB, SP, VB, SP, VB, SP, VB, SP, VB, SP, VB  /* | | | | | | */
#define ACDC_17 LT, HB, _X, HB, _X, HB, _X, HB, _X, HB, RT  /* +-+-+-+-+-+ */
#define ACDC_18 VB, SP, VB, SP, VB, SP, VB, SP, VB, SP, VB  /* | | | | | | */
#define ACDC_19 LT, HB, _X, HB, _X, HB, _X, HB, _X, HB, RT  /* +-+-+-+-+-+ */
#define ACDC_20 VB, SP, VB, SP, VB, SP, VB, SP, VB, SP, VB  /* | | | | | | */
#define ACDC_21 BL, HB, BT, HB, BT, HB, BT, HB, BT, HB, BR  /* +-+-+-+-+-+ */
#define ACDC_22 SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP  /*             */


/* Define the ACD X+ FACE display               */
#define ACDR_10 SP, C0, SP, C1, SP, C2, SP, C3              /*  0 1 2 3  */
#define ACDR_11 TL, HB, TT, HB, TT, HB, TT, HB, TR          /* +-+-+-+-+ */
#define ACDR_12 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define ACDR_13 LT, HB, _X, HB, _X, HB, Rt, SP, VB          /* +-+-+-| | */
#define ACDR_14 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define ACDR_15 LT, HB, _X, HB, _X, HB, Rt, SP, VB          /* +-+-+-| | */
#define ACDR_16 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define ACDR_17 LT, HB, _X, HB, _X, HB, Rt, SP, VB          /* +-+-+-| | */
#define ACDR_18 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define ACDR_19 LT, HB, _X, HB, _X, HB, Rt, SP, VB          /* +-+-+-| | */
#define ACDR_20 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define ACDR_21 BL, HB, BT, HB, BT, HB, BT, HB, BR          /* +-+-+-+-+ */
#define ACDR_22 SP, SP, SP, SP, SP, SP, SP, SP, SP          /*           */


/* Defines the ACD Y- FACE display                      */
#define ACDB_22 SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP  /*             */
#define ACDB_23 TL, HB, TT, HB, TT, HB, TT, HB, TT, HB, TR  /* +-+-+-+-+-+ */
#define ACDB_24 VB, SP, VB, SP, VB, SP, VB, SP, VB, SP, VB  /* | | | | | | */
#define ACDB_25 LT, HB, _X, HB, _X, HB, _X, HB, _X, HB, RT  /* +-+-+-+-+-+ */
#define ACDB_26 VB, SP, VB, SP, VB, SP, VB, SP, VB, SP, VB  /* | | | | | | */ 
#define ACDB_27 LT, HB, _X, HB, _X, HB, _X, HB, _X, HB, RT  /* +-+-+-+-+-+ */
#define ACDB_28 VB, SP, VB, SP, VB, SP, VB, SP, VB, SP, VB  /* | | | | | | */
#define ACDB_29 LT, HB, Bt, HB, Bt, HB, Bt, HB, Bt, HB, RT  /* +---------+ */
#define ACDB_30 VB, SP, SP, SP, SP, SP, SP, SP, SP, SP, VB  /* |         | */
#define ACDB_31 BL, HB, HB, HB, HB, HB, HB, HB, HB, HB, BR  /* +---------+ */
#define ACDB_32 SP, SP, SP, SP, SP, SP, SP, SP, SP, SP, SP  /*             */

/* Positions the ACD Y+ FACE display above the TOP display */
#define ACD__0 _11, SP, SP, ACDT__0
#define ACD__1 _11, SP, SP, ACDT__1
#define ACD__2 _11, C3, SP, ACDT__2
#define ACD__3 _11, SP, SP, ACDT__3
#define ACD__4 _11, C2, SP, ACDT__4
#define ACD__5 _11, SP, SP, ACDT__5
#define ACD__6 _11, C1, SP, ACDT__6
#define ACD__7 _11, SP, SP, ACDT__7
#define ACD__8 _11, C0, SP, ACDT__8
#define ACD__9 _11, SP, SP, ACDT__9

/* Assembles the ACD X-, ACD TOP AND ACD X+ display            */    
#define ACD_10  SP, SP, ACDL_10, SP, SP, ACDC_10, SP, SP, ACDR_10
#define ACD_11  SP, SP, ACDL_11, SP, SP, ACDC_11, SP, SP, ACDR_11
#define ACD_12  C4, SP, ACDL_12, SP, SP, ACDC_12, SP, SP, ACDR_12
#define ACD_13  SP, SP, ACDL_13, SP, SP, ACDC_13, SP, SP, ACDR_13
#define ACD_14  C3, SP, ACDL_14, SP, SP, ACDC_14, SP, SP, ACDR_14
#define ACD_15  SP, SP, ACDL_15, SP, SP, ACDC_15, SP, SP, ACDR_15
#define ACD_16  C2, SP, ACDL_16, SP, SP, ACDC_16, SP, SP, ACDR_16
#define ACD_17  SP, SP, ACDL_17, SP, SP, ACDC_17, SP, SP, ACDR_17
#define ACD_18  C1, SP, ACDL_18, SP, SP, ACDC_18, SP, SP, ACDR_18
#define ACD_19  SP, SP, ACDL_19, SP, SP, ACDC_19, SP, SP, ACDR_19
#define ACD_20  C0, SP, ACDL_20, SP, SP, ACDC_20, SP, SP, ACDR_20
#define ACD_21  SP, SP, ACDL_21, SP, SP, ACDC_21, SP, SP, ACDR_21
#define ACD_22  SP, SP, ACDL_22, SP, SP, ACDC_22, SP, SP, ACDR_22

/* Positions the ACD Y- FACE display */    
#define ACD_23 _11, SP, SP, ACDB_23
#define ACD_24 _11, C0, SP, ACDB_24
#define ACD_25 _11, SP, SP, ACDB_25
#define ACD_26 _11, C1, SP, ACDB_26
#define ACD_27 _11, SP, SP, ACDB_27
#define ACD_28 _11, C2, SP, ACDB_28
#define ACD_29 _11, SP, SP, ACDB_29
#define ACD_30 _11, C3, SP, ACDB_30
#define ACD_31 _11, SP, SP, ACDB_31
#define ACD_32 _11, SP, SP, ACDB_32


/* Defines the TKR Y+ SIDE display              */    
#define TKRT__0 TL, HB, TT, HB, TT, HB, TT, HB, TR          /* +-+-+-+-+ */
#define TKRT__1 Lt, SP, SP, SP, SP, SP, SP, SP, Rt          /* |       | */
#define TKRT__2 Lt, SP, SP, SP, SP, SP, SP, SP, Rt          /* |       | */
#define TKRT__3 Lt, SP, SP, SP, SP, SP, SP, SP, Rt          /* |       | */
#define TKRT__4 Lt, SP, SP, SP, SP, SP, SP, SP, Rt          /* |       | */
#define TKRT__5 Lt, SP, SP, SP, SP, SP, SP, SP, Rt          /* |       | */
#define TKRT__6 Lt, SP, SP, SP, SP, SP, SP, SP, Rt          /* |       | */
#define TKRT__7 Lt, SP, SP, SP, SP, SP, SP, SP, Rt          /* |       | */
#define TKRT__8 Lt, SP, SP, SP, SP, SP, SP, SP, Rt          /* |       | */
#define TKRT__9 Lt, SP, SP, SP, SP, SP, SP, SP, Rt          /* |       | */
#define TKRT_10 BL, HB, BT, HB, BT, HB, BT, HB, BR          /* +-+-+-+-+ */


/* Defines the TKR X- SIDE display                      */    
#define TKRL_11 SP, C0, C2, C4, C6, C8, Ca, Cc, Ce, Cg, SP  /*  02468aceg  */
#define TKRL_12 TL, Tt, Tt, Tt, Tt, Tt, Tt, Tt, Tt, Tt, TR  /* +---------+ */
#define TKRL_13 VB, SP, SP, SP, SP, SP, SP, SP, SP, SP, VB  /* |         | */
#define TKRL_14 LT, SP, SP, SP, SP, SP, SP, SP, SP, SP, RT  /* +         + */
#define TKRL_15 VB, SP, SP, SP, SP, SP, SP, SP, SP, SP, VB  /* |         | */
#define TKRL_16 LT, SP, SP, SP, SP, SP, SP, SP, SP, SP, RT  /* +         + */
#define TKRL_17 VB, SP, SP, SP, SP, SP, SP, SP, SP, SP, VB  /* |         | */
#define TKRL_18 LT, SP, SP, SP, SP, SP, SP, SP, SP, SP, RT  /* +         + */
#define TKRL_19 VB, SP, SP, SP, SP, SP, SP, SP, SP, SP, VB  /* |         | */
#define TKRL_20 BL, Bt, Bt, Bt, Bt, Bt, Bt, Bt, Bt, Bt, BR  /* +---------+ */
#define TKRL_21 SP, C0, C2, C4, C6, C8, Ca, Cc, Ce, Cg, SP  /*  02468aceg  */


/* Defines the TKR TOP display                  */    
#define TKRC_11 SP, Cc, SP, Cd, SP, Ce, SP, Cf, SP          /*           */
#define TKRC_12 TL, HB, TT, HB, TT, HB, TT, HB, TR          /* +-+-+-+-+ */
#define TKRC_13 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define TKRC_14 LT, HB, _X, HB, _X, HB, _X, HB, RT          /* +-+-+-+-+ */
#define TKRC_15 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define TKRC_16 LT, HB, _X, HB, _X, HB, _X, HB, RT          /* +-+-+-+-+ */
#define TKRC_17 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define TKRC_18 LT, HB, _X, HB, _X, HB, _X, HB, RT          /* +-+-+-+-+ */
#define TKRC_19 VB, SP, VB, SP, VB, SP, VB, SP, VB          /* | | | | | */
#define TKRC_20 BL, HB, BT, HB, BT, HB, BT, HB, BR          /* +-+-+-+-+ */
#define TKRC_21 SP, C0, SP, C1, SP, C2, SP, C3, SP          /*           */

    
/* Defines the TKR X+ SIDE display                  */
#define TKRR_11 SP, Cg, Ce, Cc, Ca, C8, C6, C4, C2, C0
#define TKRR_12 TKRL_12
#define TKRR_13 TKRL_13
#define TKRR_14 TKRL_14
#define TKRR_15 TKRL_15
#define TKRR_16 TKRL_16
#define TKRR_17 TKRL_17
#define TKRR_18 TKRL_18
#define TKRR_19 TKRL_19
#define TKRR_20 TKRL_20
#define TKRR_21 SP,  Cg, Ce, Cc, Ca, C8, C6, C4, C2, C0

/* Defines the TKR Y- SIDE display                  */    
#define TKRB_22 TKRT__0
#define TKRB_23 TKRT__1
#define TKRB_24 TKRT__2
#define TKRB_25 TKRT__3
#define TKRB_26 TKRT__4
#define TKRB_27 TKRT__5
#define TKRB_28 TKRT__6
#define TKRB_29 TKRT__7
#define TKRB_30 TKRT__8
#define TKRB_31 TKRT__9
#define TKRB_32 TKRT_10

/* Defines a core tower display */     
#define TKR_C0 TL, HB, HB, HB, HB, HB, HB, HB, HB, HB, TR
#define TKR_C1 VB, SP, SP, SP, SP, SP, SP, SP, SP, SP, VB
#define TKR_C2 BL, HB, HB, HB, HB, HB, HB, HB, HB, HB, BR    
    

/* Positions the TKR Y+ display above the TKR TOP display, adds TWR 9 and 10 */
#define TKR__0  _22,     SP, SP, SP, TKRT__0
#define TKR__1  _22,     SP, SP, C0, TKRT__1, C0
#define TKR__2  _22,     SP, SP, C2, TKRT__2, C2
#define TKR__3  _22,     SP, SP, C4, TKRT__3, C4
#define TKR__4  _22,     SP, SP, C6, TKRT__4, C6
#define TKR__5  _22,     SP, SP, C8, TKRT__5, C8
#define TKR__6  _22,     SP, SP, Ca, TKRT__6, Ca
#define TKR__7  _22,     SP, SP, Cc, TKRT__7, Cc
#define TKR__8  _12, TKR_C0, SP, Ce, TKRT__8, Ce, SP, TKR_C0
#define TKR__9  _12, TKR_C1, SP, Cg, TKRT__9, Cg, SP, TKR_C1
#define TKR_10   _2, TKR_C2, SP, SP, TKRT_10, SP, SP, TKR_C2



/* Assembles the TKR X-, TKR TOP and TKR X+ displays       */
#define TKR_11  SP, TKRL_11, SP, SP, TKRC_11, SP, SP, TKRR_11
#define TKR_12  SP, TKRL_12, SP, SP, TKRC_12, SP, SP, TKRR_12
#define TKR_13  SP, TKRL_13, SP, Cc, TKRC_13, Cf, SP, TKRR_13
#define TKR_14  SP, TKRL_14, SP, SP, TKRC_14, SP, SP, TKRR_14
#define TKR_15  SP, TKRL_15, SP, C8, TKRC_15, Cb, SP, TKRR_15
#define TKR_16  SP, TKRL_16, SP, SP, TKRC_16, SP, SP, TKRR_16
#define TKR_17  SP, TKRL_17, SP, C4, TKRC_17, C7, SP, TKRR_17
#define TKR_18  SP, TKRL_18, SP, SP, TKRC_18, SP, SP, TKRR_18
#define TKR_19  SP, TKRL_19, SP, C0, TKRC_19, C3, SP, TKRR_19
#define TKR_20  SP, TKRL_20, SP, SP, TKRC_20, SP, SP, TKRR_20
#define TKR_21  SP, TKRL_21, SP, SP, TKRC_21, SP, SP, TKRR_21

    
/* Positions the Y- display below the TKR TOP display, adds TWR 5 & TWR 6*/    
#define TKR_22   SP, TKR_C0, SP, SP, TKRB_22, SP, SP, TKR_C0
#define TKR_23  _12, TKR_C1, SP, Cg, TKRB_23, Cg, SP, TKR_C1
#define TKR_24  _12, TKR_C2, SP, Ce, TKRB_24, Ce, SP, TKR_C2
#define TKR_25  _22,     SP, SP, Cc, TKRB_25, Cc
#define TKR_26  _22,     SP, SP, Ca, TKRB_26, Ca
#define TKR_27  _22,     SP, SP, C8, TKRB_27, C8
#define TKR_28  _22,     SP, SP, C6, TKRB_28, C6
#define TKR_29  _22,     SP, SP, C4, TKRB_29, C4
#define TKR_30  _22,     SP, SP, C2, TKRB_30, C2
#define TKR_31  _22,     SP, SP, C0, TKRB_31, C0
#define TKR_32  _22,     SP, SP, SP, TKRB_32

    
/*
 | Macro to compose a full display line.
 | This is really nothing more than a
 |
 |    <left-margin> <acd-display> <margin> <tkr-display> <newline>
*/
#define DL(_n) SP, ACD_##_n, SP, SP, TKR_##_n, NL

/*
 | There are only 4 line lengths in the display, this defines
 | the symbols for these line lengths
*/
#define NSEP1 62
#define NSIDE 63
#define NSEP2 74
#define NMAIN 75


/* Define the length of each line in the display template */
#define NLINE_0     NSEP1
#define NLINE_1     NSIDE
#define NLINE_2     NSIDE
#define NLINE_3     NSIDE
#define NLINE_4     NSIDE
#define NLINE_5     NSIDE
#define NLINE_6     NSIDE
#define NLINE_7     NSIDE
#define NLINE_8     NMAIN
#define NLINE_9     NMAIN
#define NLINE10     NMAIN
#define NLINE11     NSEP2   
#define NLINE12     NMAIN
#define NLINE13     NMAIN
#define NLINE14     NMAIN
#define NLINE15     NMAIN
#define NLINE16     NMAIN
#define NLINE17     NMAIN
#define NLINE18     NMAIN
#define NLINE19     NMAIN
#define NLINE20     NMAIN
#define NLINE21     NSEP2
#define NLINE22     NMAIN
#define NLINE23     NMAIN
#define NLINE24     NMAIN
#define NLINE25     NSIDE
#define NLINE26     NSIDE
#define NLINE27     NSIDE
#define NLINE28     NSIDE
#define NLINE29     NSIDE
#define NLINE30     NSIDE
#define NLINE31     NSIDE
#define NLINE32     NSEP1


#define BLINE_0                  0
#define BLINE_1 (BLINE_0 + NLINE_0)
#define BLINE_2 (BLINE_1 + NLINE_1)
#define BLINE_3 (BLINE_2 + NLINE_2)
#define BLINE_4 (BLINE_3 + NLINE_3)
#define BLINE_5 (BLINE_4 + NLINE_4)
#define BLINE_6 (BLINE_5 + NLINE_5)
#define BLINE_7 (BLINE_6 + NLINE_6)
#define BLINE_8 (BLINE_7 + NLINE_7)
#define BLINE_9 (BLINE_8 + NLINE_8)

#define BLINE10 (BLINE_9 + NLINE_9)
#define BLINE11 (BLINE10 + NLINE10)
#define BLINE12 (BLINE11 + NLINE11)
#define BLINE13 (BLINE12 + NLINE12)
#define BLINE14 (BLINE13 + NLINE13)
#define BLINE15 (BLINE14 + NLINE14)
#define BLINE16 (BLINE15 + NLINE15)
#define BLINE17 (BLINE16 + NLINE16)
#define BLINE18 (BLINE17 + NLINE17)
#define BLINE19 (BLINE18 + NLINE18)

#define BLINE20 (BLINE19 + NLINE19)
#define BLINE21 (BLINE20 + NLINE20)
#define BLINE22 (BLINE21 + NLINE21)
#define BLINE23 (BLINE22 + NLINE22)
#define BLINE24 (BLINE23 + NLINE23)
#define BLINE25 (BLINE24 + NLINE24)
#define BLINE26 (BLINE25 + NLINE25)
#define BLINE27 (BLINE26 + NLINE26)
#define BLINE28 (BLINE27 + NLINE27)
#define BLINE29 (BLINE28 + NLINE28)

#define BLINE30 (BLINE29 + NLINE29)
#define BLINE31 (BLINE30 + NLINE30)
#define BLINE32 (BLINE31 + NLINE31)


#define BAD_INPUT BLINE_0

/*
 | Simple macro to lay down a tower number as a hex number. Its main
 | virtue is that is documents nicely, one just does see a bare number
 | 0xe, one sees TWR (E) and knows that it is a tower number
*/
#define TWR(_n) (0x ##_n)

    
/* These structures determine the hit symbol set used for ACD and TKR */
struct _SymSetAcd
{
    unsigned char    x;
    unsigned char    y;
    unsigned char  top;
    unsigned char wide;
};


/* These number the 4 types to TKR side sets                       */
enum _SymSetTkrSideNum
{
    TKR_SIDE_SET_HP = 0,  /* Set for increasing horizontal display */
    TKR_SIDE_SET_HM = 1,  /* Set for decreasing horizontal display */
    TKR_SIDE_SET_VM = 2,  /* Set for decreasing vertical   display */
    TKR_SIDE_SET_VP = 3   /* Set for increasing vertical   display */
};


struct _SymSetTkr
{
    unsigned char top[18];
    unsigned char side[4][4];
};


struct _SymSet
{
    struct _SymSetAcd acd;
    struct _SymSetTkr tkr;
};


/*
 |  This determines where the hits get placed in the picture, giving the
 |  absolute position of the coordinate.
*/
struct _HitMapAcd
{
    unsigned short int top   [32];  /* Where the ACD top hits go      */
    unsigned short int x     [32];  /* Where the ACD X   hits go      */
    unsigned short int y     [32];  /* Where the ACD Y   hits go      */
    unsigned short int wide[4][8];  /* Wide tile positions (X-+, Y-+) */
};



struct _HitMapTkrSide
{
    unsigned char            twr; /* Side tower number              */
    unsigned char            set; /* Fill set   number              */
    unsigned short int    pos[9]; /* Fill position                  */
};


struct _HitMapTkr
{
    unsigned short int     top[16]; /* Where the TKR top hits go      */
    struct _HitMapTkrSide side[20]; /* Where the TKR side lines go    */
                                    /* The last 4 are the 5, 6, 9, 10 */
};


struct _HitMap
{
    struct _HitMapAcd acd;          /* Where the ACD hits go          */ 
    struct _HitMapTkr tkr;          /* Where the TKR hits go          */  
};


static void displayAcd      (unsigned char                *d,
                             const struct _HitMapAcd    *hit,
                             const struct _SymSetAcd    *sym,
                             int                     acd_top,
                             int                       acd_x,
                             int                       acd_y);

static void fillAcdWideTile (unsigned char                *d,
                             const unsigned short int hit[8],
                             char                        sym);

static void displayTkr      (unsigned char                *d,
                             const struct _HitMapTkr    *hit,
                             const struct _SymSetTkr    *sym,
                             const int               tkr[16]);

static void translate       (unsigned char                *d,
                             const unsigned char  *dtemplate,
                             int                           n,
                             const unsigned char      ds[16],
                             const unsigned char      ts[16]);

static void printAcdTkr     (const int *tkr,
                             int    acd_top,
                             int      acd_x,
                             int      acd_y);

static void printAcd        (int    acd_top,
                             int      acd_x,
                             int      acd_y);


#ifdef __cplusplus
}
#endif





/* ---------------------------------------------------------------------- */
void DFC_display (struct _DFC_latRecord *dlr)
{
   const EBF_glt *glt;

   printf ("List of CTIDS: %8.8x\n", dlr->dir.ctids);

   glt = (const EBF_glt *)dlr->dir.contributors[EBF_K_CID_GLT].ptr;   
   CFC_latRecordPrint (&dlr->cal, -1);
   TFC_latRecordPrint (&dlr->tkr, -1);
   DFC_displayDir  (0, &dlr->dir);

   return;
}
/* ---------------------------------------------------------------------- */
   




void  DFC_displayDir (unsigned int style, const struct _EBF_directory *dir)
{
   const EBF_contributor *contributors;
   int                          xy[16];
   int                            tids;
   int                         acd_top;
   int                           acd_x;
   int                           acd_y;
   
   tids         = dir->ctids << 16;
   contributors = dir->contributors;

   // printf ("Contributors = %8.8x\n", tids);
   
   
   /* Loop over all towers that may have tracker data */
   while (tids)
   {
       const EBF_contributor *contributor;
       int                       xcapture;
       int                       ycapture;
       const EBF_tkr                 *tkr;
       int                            cid;
       int                      xycapture;

       /*
        | Find the next tower with tracker hits and then eliminate it
        | from further consideration
       */
       cid   = FFS (tids);
       tids &= ~(0x80000000 >> cid);

       /* Locate the TEM contributor and its tracker data */
       contributor = &contributors[cid];
       tkr         = EBF__tkrLocate (contributor->ptr, contributor->calcnt);
       
       EBF_TKR_ACCEPTS_RIGHT_ORED_UNPACK (tkr->accepts, xcapture, ycapture);
       xycapture = xcapture & ycapture;
           
       // printf (" Filling contributor %d X:%8.8x Y:%8.8x\n",
       //         cid, xcapture, ycapture);

       /* Use the X/Y coincidence for the track trigger data */
       xy[cid - EBF_K_CID_TWR] = EBF_TKR_RIGHT_ACCEPT_REMAP (xycapture);
   }


   acd_top = 0;
   acd_x   = 0;
   acd_y   = 0;


   /* Is there an GLT contribution?  */
   if (dir->cids & (0x80000000 >> EBF_K_CID_GLT))
   {
       const EBF_glt *glt;

       glt     = (const EBF_glt *)dir->contributors[EBF_K_CID_GLT].ptr;
       acd_top = glt->acd.vetoes[EBF_K_GLT_ACD_VETO_XY];
       acd_x   = glt->acd.vetoes[EBF_K_GLT_ACD_VETO_YZ];
       acd_y   = glt->acd.vetoes[EBF_K_GLT_ACD_VETO_XZ];
   }
   
   
   DFC_displayAcdTkr (style, xy, acd_top, acd_x, acd_y);
   
   return;
   
}



void  DFC_displayAcdTkr (unsigned int  style,
                         const int   tkr[16],
                         int         acd_top,
                         int           acd_x,
                         int           acd_y)
{
   /*
    | Defines the Outline Set, define the set composing the template outline
    | Typically these characters are substituted (logically translated) into
    | other sets that render the picture. The last entry indicates the
    | translation set that this matches. Here it is set 1. If the set being
    | used as a translation set matches this set, only a copy is needed.
   */ 
  const unsigned char TemplateSet[16] =
     {  VB, HB, TL, TR, BR, BL, TT, RT, BT, LT, _X, Tt, Rt, Bt, Lt, -1};
  
  const unsigned char OutlineSets[2][16] =
  {
     {'|','-','+','+','+','+','+','+','+','+','+','-','|','-','|', 0 },
     { VB, HB, TL, TR, BR, BL, TT, RT, BT, LT, _X, TT, RT, BT, LT, 1 }
  };

  static const struct _SymSet SymSets[2] =
  {
    { /* Straight ASCII set                   */
      { /* ACD #0                             */
        '*',                      /* ACD_X    */
        '*',                      /* ACD_Y    */
        '*',                      /* ACD_TOP  */
        '*',                      /* ACD_WIDE */
      },
     
      { /* TKR #0  */
        { '0', '1', '2', '3', '4','5', '6', '7', '8',
          '9', 'a', 'b', 'c', 'd', 'e','f', 'g', 'h'
        },

        {
          { ' ', '>', '<', '-' }, /* TKR X-   */
          { ' ', '<', '>', '-' }, /* TKR X+   */
          { ' ', '^', 'v', '|' }, /* TKR Y-   */        
          { ' ', 'v', '^', '|' }  /* TKR Y+   */
        }
      }
    },
    
    { /* Fancy graphics set                   */
      { /* ACD #1 */
        FB,                       /* ACD_X    */
        FB,                       /* ACD_Y    */
        FB,                       /* ACD_TOP  */
        FB,                       /* ACD_WIDE */
      },
      
      { /* TKR #1 */
        {
          '0', '1', '2', '3', '4','5', '6', '7', '8',
          '9', 'a', 'b', 'c', 'd', 'e','f', 'g', 'h'
        },
        
        {
          { ' ', LHB, RHB, FB  }, /* TKR X-   */
          { ' ', RHB, LHB, FB  }, /* TKR X+   */
          { ' ', BHB, THB, FB  }, /* TKR Y-   */        
          { ' ', THB, BHB, FB  }  /* TKR Y+   */
        }
      }
    }
  };


   /* This defines the pictures outline template */
   static const unsigned char Template[] =
   {
     DL(_0), DL(_1), DL(_2), DL(_3), DL(_4), DL(_5), DL(_6), DL(_7),
     DL(_8), DL(_9), DL(10), DL(11), DL(12), DL(13), DL(14), DL(15),
     DL(16), DL(17), DL(18), DL(19), DL(20), DL(21), DL(22), DL(23),
     DL(24), DL(25), DL(26), DL(27), DL(28), DL(29), DL(30), DL(31),
     DL(32),
     0
   };

   
   /* Defines where the hits go in the picture template */
   static const struct _HitMap HitMap =
   { 
    { /* ACD Hit Map */
     { /* Indicates where the ACD top hits should be placed */     
       BLINE20 + 15, BLINE20 + 17, BLINE20 + 19, BLINE20 + 21, BLINE20 + 23,
       BLINE18 + 15, BLINE18 + 17, BLINE18 + 19, BLINE18 + 21, BLINE18 + 23,
       BLINE16 + 15, BLINE16 + 17, BLINE16 + 19, BLINE16 + 21, BLINE16 + 23,
       BLINE14 + 15, BLINE14 + 17, BLINE14 + 19, BLINE14 + 21, BLINE14 + 23,
       BLINE12 + 15, BLINE12 + 17, BLINE12 + 19, BLINE12 + 21, BLINE12 + 23,
       BAD_INPUT,    BAD_INPUT,    BAD_INPUT,    BAD_INPUT,    BAD_INPUT,
       BAD_INPUT,    BAD_INPUT     
     },

     {
       /* ACD X- mapping */
       BLINE20 + 10, BLINE18 + 10, BLINE16 + 10, BLINE14 + 10, BLINE12 + 10,
       BLINE20 +  8, BLINE18 +  8, BLINE16 +  8, BLINE14 +  8, BLINE12 +  8,
       BLINE20 +  6, BLINE18 +  6, BLINE16 +  6, BLINE14 +  6, BLINE12 +  6,
       BLINE20 +  4,

       /* ACD X+ mapping */
       BLINE20 + 28, BLINE18 + 28, BLINE16 + 28, BLINE14 + 28, BLINE12 + 28,
       BLINE20 + 30, BLINE18 + 30, BLINE16 + 30, BLINE14 + 30, BLINE12 + 30,
       BLINE20 + 32, BLINE18 + 32, BLINE16 + 32, BLINE14 + 32, BLINE12 + 32,
       BLINE20 + 34
     },

     { /* ACD Y- mapping */
       BLINE24 + 15, BLINE24 + 17, BLINE24 + 19, BLINE24 + 21, BLINE24 + 23,
       BLINE26 + 15, BLINE26 + 17, BLINE26 + 19, BLINE26 + 21, BLINE26 + 23,
       BLINE28 + 15, BLINE28 + 17, BLINE28 + 19, BLINE28 + 21, BLINE28 + 23,
       BLINE30 + 15, 

       /* ACD Y+ mapping */
       BLINE_8 + 15, BLINE_8 + 17, BLINE_8 + 19, BLINE_8 + 21, BLINE_8 + 23,
       BLINE_6 + 15, BLINE_6 + 17, BLINE_6 + 19, BLINE_6 + 21, BLINE_6 + 23,
       BLINE_4 + 15, BLINE_4 + 17, BLINE_4 + 19, BLINE_4 + 21, BLINE_4 + 23,
       BLINE_2 + 15
     },
      
     {   /* ACD WIDE TILE mapping */ 
         
         { /* Fill in the extended X- tile */
           BLINE12 +  4, BLINE13 +  4, BLINE14 +  4, BLINE15 +  4,
           BLINE16 +  4, BLINE17 +  4, BLINE18 +  4, BLINE19 +  4
         },
    
         { /* Fill in the extended X+ tile */
           BLINE12 + 34, BLINE13 + 34, BLINE14 + 34, BLINE15 + 34,
           BLINE16 + 34, BLINE17 + 34, BLINE18 + 34, BLINE19 + 34
         },

         { /* Fill in the extended Y- tile */
           BLINE30 + 16, BLINE30 + 17, BLINE30 + 18, BLINE30 + 19,
           BLINE30 + 20, BLINE30 + 21, BLINE30 + 22, BLINE30 + 23
         },
         
         { /* Fill in the extended Y+ tile */
           BLINE_2 + 16, BLINE_2 + 17, BLINE_2 + 18, BLINE_2 + 19,
           BLINE_2 + 20, BLINE_2 + 21, BLINE_2 + 22, BLINE_2 + 23
         }
     }
    },

    
    { /* TKR Mappings */
      { /* TKR Top Mapping */
        BLINE19 + 53, BLINE19 + 55, BLINE19 + 57, BLINE19 + 59,
        BLINE17 + 53, BLINE17 + 55, BLINE17 + 57, BLINE17 + 59,
        BLINE15 + 53, BLINE15 + 55, BLINE15 + 57, BLINE15 + 59,
        BLINE13 + 53, BLINE13 + 55, BLINE13 + 57, BLINE13 + 59
      },

      { /* TKR Side Mapping */
        { /* 0: TWR 0 XM0, ROW 0 */
          TWR (0),
          0, //TKR_SIDE_SET_HP,
          { BLINE19 + 40, BLINE19 + 41, BLINE19 + 42,
            BLINE19 + 43, BLINE19 + 44, BLINE19 + 45,
            BLINE19 + 46, BLINE19 + 47, BLINE19 + 48
          },
        },

       { /* 1: TWR 4 XM1, ROW 1 */
         TWR (4),
         TKR_SIDE_SET_HP,
         { BLINE17 + 40, BLINE17 + 41, BLINE17 + 42,
           BLINE17 + 43, BLINE17 + 44, BLINE17 + 45,
           BLINE17 + 46, BLINE17 + 47, BLINE17 + 48
         },
       },
        
       
       { /* 2: TWR 8 XM2, ROW 2 */
         TWR (8),
         TKR_SIDE_SET_HP,
         { BLINE15 + 40, BLINE15 + 41, BLINE15 + 42,
           BLINE15 + 43, BLINE15 + 44, BLINE15 + 45,
           BLINE15 + 46, BLINE15 + 47, BLINE15 + 48
         },
       },

       
       { /* 3: TWR C XM3, ROW 3 */
         TWR (C),
         TKR_SIDE_SET_HP,
         { BLINE13 + 40, BLINE13 + 41, BLINE13 + 42,
           BLINE13 + 43, BLINE13 + 44, BLINE13 + 45,
           BLINE13 + 46, BLINE13 + 47, BLINE13 + 48
         },
       },
        
       
       { /* 4: TWR 3 XP0, ROW 0 */
         TWR (3),
         TKR_SIDE_SET_HM,
         { BLINE19 + 72, BLINE19 + 71, BLINE19 + 70,
           BLINE19 + 69, BLINE19 + 68, BLINE19 + 67,
           BLINE19 + 66, BLINE19 + 65, BLINE19 + 64
         },
       },
        
       
       { /* 5: TWR 7 XP1, ROW 1 */
         TWR (7),
         TKR_SIDE_SET_HM,
         { BLINE17 + 72, BLINE17 + 71, BLINE17 + 70,
           BLINE17 + 69, BLINE17 + 68, BLINE17 + 67,
           BLINE17 + 66, BLINE17 + 65, BLINE17 + 64
         },
       },
        

       { /* 6: TWR B XP2, ROW 2 */
         TWR (B),
         TKR_SIDE_SET_HM,
         { BLINE15 + 72, BLINE15 + 71, BLINE15 + 70,
           BLINE15 + 69, BLINE15 + 68, BLINE15 + 67,
           BLINE15 + 66, BLINE15 + 65, BLINE15 + 64
         },
       },

       { /* 7: TWR F XP3, ROW 3 */
         TWR (F),
         TKR_SIDE_SET_HM,
         { BLINE13 + 72, BLINE13 + 71, BLINE13 + 70,
           BLINE13 + 69, BLINE13 + 68, BLINE13 + 67,
           BLINE13 + 66, BLINE13 + 65, BLINE13 + 64
         },
       },


       { /* 8: TWR 0 YM0, COL 0 */
         TWR (0),
         TKR_SIDE_SET_VM,
         { BLINE31 + 53, BLINE30 + 53, BLINE29 + 53,
           BLINE28 + 53, BLINE27 + 53, BLINE26 + 53,
           BLINE25 + 53, BLINE24 + 53, BLINE23 + 53
         },
       },
       
       { /* 9: TWR 1 YM1, COL 1 */
         TWR (1),
         TKR_SIDE_SET_VM,
         { BLINE31 + 55, BLINE30 + 55, BLINE29 + 55,
           BLINE28 + 55, BLINE27 + 55, BLINE26 + 55,
           BLINE25 + 55, BLINE24 + 55, BLINE23 + 55
         },
       },

       
       { /* A: TWR 2 YM2, COL 2 */
         TWR (2),
         TKR_SIDE_SET_VM,
         { BLINE31 + 57, BLINE30 + 57, BLINE29 + 57,
           BLINE28 + 57, BLINE27 + 57, BLINE26 + 57,
           BLINE25 + 57, BLINE24 + 57, BLINE23 + 57
         }
       },
       
       { /* B: TWR 3 YM3, COL 3 */
         TWR (3),
         TKR_SIDE_SET_VM,
         { BLINE31 + 59, BLINE30 + 59, BLINE29 + 59,
           BLINE28 + 59, BLINE27 + 59, BLINE26 + 59,
           BLINE25 + 59, BLINE24 + 59, BLINE23 + 59
         },
       },
       
       
       { /* C: TWR C YP0, COL 0 */
         TWR (C),
         TKR_SIDE_SET_VP,
         { BLINE_1 + 53, BLINE_2 + 53, BLINE_3 + 53,
           BLINE_4 + 53, BLINE_5 + 53, BLINE_6 + 53,
           BLINE_7 + 53, BLINE_8 + 53, BLINE_9 + 53
         },
       },


       { /* D: TWR D YP1, COL 1 */
         TWR (D),
         TKR_SIDE_SET_VP,
         { BLINE_1 + 55, BLINE_2 + 55, BLINE_3 + 55,
           BLINE_4 + 55, BLINE_5 + 55, BLINE_6 + 55,
           BLINE_7 + 55, BLINE_8 + 55, BLINE_9 + 55
         },
       },
       
       { /* E: TWR E YP2, COL 2 */
         TWR (E),
         TKR_SIDE_SET_VP,
         { BLINE_1 + 57, BLINE_2 + 57, BLINE_3 + 57,
           BLINE_4 + 57, BLINE_5 + 57, BLINE_6 + 57,
           BLINE_7 + 57, BLINE_8 + 57, BLINE_9 + 57
         }
       },

       { /* F: TWR F YP3, COL 3 */
         TWR (F),
         TKR_SIDE_SET_VP,
         { BLINE_1 + 59, BLINE_2 + 59, BLINE_3 + 59,
           BLINE_4 + 59, BLINE_5 + 59, BLINE_6 + 59,
           BLINE_7 + 59, BLINE_8 + 59, BLINE_9 + 59
         },
       },
       
       { /* 10: TWR 5 */
         TWR (5),
         TKR_SIDE_SET_HP,
         { BLINE23 + 40, BLINE23 + 41, BLINE23 + 42,
           BLINE23 + 43, BLINE23 + 44, BLINE23 + 45,
           BLINE23 + 46, BLINE23 + 47, BLINE23 + 48
         },
       },
       
       { /* 11: TWR 6 */
         TWR (6),
         TKR_SIDE_SET_HM,
         { BLINE23 + 72, BLINE23 + 71, BLINE23 + 70,
           BLINE23 + 69, BLINE23 + 68, BLINE23 + 67,
           BLINE23 + 66, BLINE23 + 65, BLINE23 + 64
         },
       },
       
       { /* 12: TWR 9 */
         TWR (9),
         TKR_SIDE_SET_HP,
         { BLINE_9 + 40, BLINE_9 + 41, BLINE_9 + 42,
           BLINE_9 + 43, BLINE_9 + 44, BLINE_9 + 45,
           BLINE_9 + 46, BLINE_9 + 47, BLINE_9 + 48
         },
       },
       
       { /* 13: TWR A */
         TWR (A),
         TKR_SIDE_SET_HM,
         { BLINE_9 + 72, BLINE_9 + 71, BLINE_9 + 70,
           BLINE_9 + 69, BLINE_9 + 68, BLINE_9 + 67,
           BLINE_9 + 66, BLINE_9 + 65, BLINE_9 + 64
         },
       }
        
      }
    }
   };


   unsigned char d[sizeof(Template)];
   const struct         _SymSet *sym;

   
   /* Get a copy of the display outline */
   translate (d, Template, sizeof(Template), OutlineSets[style], TemplateSet);

   /* Pick up the symbols to use */
   sym = &SymSets[style];
   
   
   /* Do the ACD display */
   if (acd_top | acd_x | acd_y)
   {
       displayAcd (d, &HitMap.acd, &sym->acd, acd_top, acd_x, acd_y);
   }

   
   
   /* Do the tracker trigger display */
   if (tkr != NULL)
   {
       displayTkr (d, &HitMap.tkr, &sym->tkr, tkr);
       puts ((const char *)d);
       printAcdTkr (tkr, acd_top, acd_x, acd_y);
   }
   else
   {
      puts ((const char *)d);
      printAcd (acd_top, acd_x, acd_y);
   }
   
   return; 
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn  void translate (unsigned char       *d,
                       const unsigned char *dtemplate,
                       int                  n,
                       const unsigned char  ds[16],
                       const unsigned char  ts[16])

  \brief Translate the template of the detector outline using the specified
         graphics set.
  \param         d The display to receive the translated outline.
  \param dtemplate The template of the outline.
  \param         n The number of characters in the outline, ie how many
                  to translate.
  \param        ds The display set to translate to
  \param        ts The template set to translate from

   The template outline is more or less a logical description of the
   picture. The 16 or so logical characters that compose the outline
   must be translated to a set of ASCII characters to render the
   outline on the screen.

   This is not a efficient implementation, but is sufficient for a
   display routine.
                                                                          */
/* ---------------------------------------------------------------------- */
static void translate (unsigned char       *d,
                       const unsigned char *dtemplate,
                       int                  n,
                       const unsigned char  ds[16],
                       const unsigned char  ts[16])
{
   int idx;

   
   /* Loop over all the characters in the template */
   for (idx = 0; idx < n; idx++)
   {
       int         idy;
       unsigned char c;

       /*
        | Get the character and see if it is a member of the set that needs to
        | be translated. If so, substitute the character from the display set,
        | otherwise just use it.
        |
        | To speed the translation up, skip the translation if the character
        | is a space. This is the most common character and does not need
        | to be translated
       */
       c = dtemplate[idx];

       if (c != ' ')
       {
         for (idy = 0; idy < 16; idy++)
         {
             if (c == ts[idy]) { c = ds[idy]; break; }
         }
       }
       
       d[idx] = c;
   }
   
   return;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn  void displayAcd (unsigned char             *d,
                        const struct _HitMapAcd *hit,
                        const struct _SymSetAcd *sym,
                        int                  acd_top,
                        int                    acd_x,
                        int                    acd_y)

  \brief         Displays the tracker layer hit patterns
  \param hit     A structure indicating where the hits should be placed
                 in the display.
  \param sym     A structure defining the hit display symbols
  \param acd_top Bit pattern of the struck tiles in the top face
  \param acd_x   Bit pattern of the struck tiles in the   x faces
  \param acd_y   Bit pattern of the struct tiles in the   y faces
                                                                          */
/* ---------------------------------------------------------------------- */
static void displayAcd (unsigned char             *d,
                        const struct _HitMapAcd *hit,
                        const struct _SymSetAcd *sym,
                        int                  acd_top,
                        int                    acd_x,
                        int                    acd_y)
{
   int  idx;
   
   char sym_x = sym->x;
   char sym_y = sym->y;
   char sym_t = sym->top;
   char sym_w = sym->wide;

   
   /*
    | Generate the displays for the ACD and the TKR top.
    | Limit the patterns in th ACD top to only legitimate values.
   */
   acd_top &= (1 << 25) - 1;
   for (idx = 0; idx < 32; idx++)
   {
       int mask = 1 << idx;

       if (acd_x    & mask) d[hit->x  [idx]] = sym_x;
       if (acd_y    & mask) d[hit->y  [idx]] = sym_y;
       if (acd_top  & mask) d[hit->top[idx]] = sym_t;
   }

   
   /* Check if have hits in the wide tiles */
   if (acd_x & 0x00008000) fillAcdWideTile (d, hit->wide[0], sym_w);
   if (acd_x & 0x80000000) fillAcdWideTile (d, hit->wide[1], sym_w);
   if (acd_y & 0x00008000) fillAcdWideTile (d, hit->wide[2], sym_w);
   if (acd_y & 0x80000000) fillAcdWideTile (d, hit->wide[3], sym_w);

   
   return;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn  void fillAcdWideTile (unsigned char                *d,
                             const unsigned short int hit[8],
                             char                        sym)
  \brief Fills in the hits for the ACD wide tiles.
  \param     d The display to be filled in.
  \param   hit Array of the 8 hit positions to fill in.
  \param   sym The hit symbol.

   There are actually 9 locations to be filled in. This routine only does
   8. It is assumed that the previous code has filled in the remaining
   spot.
                                                                          */
/* ---------------------------------------------------------------------- */
static void fillAcdWideTile (unsigned char                *d,
                             const unsigned short int hit[8],
                             char                        sym)
{
   d[hit[0]] = sym;
   d[hit[1]] = sym;
   d[hit[2]] = sym;
   d[hit[3]] = sym;
   d[hit[4]] = sym;
   d[hit[5]] = sym;
   d[hit[6]] = sym;
   d[hit[7]] = sym;

   return;
}
/* ---------------------------------------------------------------------- */ 




/* ---------------------------------------------------------------------- */ 




/* ---------------------------------------------------------------------- *//*!

  \fn  void displayTkr (unsigned char             *d,
                        const struct _HitMapTkr *hit,
                        const struct _SymSetTkr *sym,
                        const int            tkr[16])
  \brief     Displays the tracker layer hit patterns
  \param hit A structure indicating where the hits should be placed in
             the display.
  \param sym A structure defining the hit display symbols
  \param tkr The 18 layer hit patterns
                                                                          */
/* ---------------------------------------------------------------------- */
static void displayTkr (unsigned char             *d,
                        const struct _HitMapTkr *hit,
                        const struct _SymSetTkr *sym,
                        const int            tkr[16])
{
   int idx;

   /*
    | Loop over all the towers to display.
    |
    | SIDE TOWER DISPLAY
    | There are 16 side towers to display because the corner towers
    | are duplicate in the X and Y views. A symbol is placed in each
    | layer where a hit occurs.
    |
    | TOP TOWER DISPLAY
    | A symbol, portional to the first layer hit (from the ACD perspective
    | is placed in each of the core towers.
    |
    | NOTE
    | It is just lucky that the side tower this produces 16 display
    | towers. For example if the LAT would have been 5x5 there would
    | have been 20 towers to display on the perimeter (16 + duplicating
    | the 4 corners). This code takes advantage of this, combining the
    | side and top tower displays in the same loop.
   */
   for (idx = 0; idx < 20 /* sizeof (TkrCtl)/sizeof(TkrCtl[0]) */; idx++)
   {
       int                               idy;
       unsigned int                  pattern;
       const unsigned char         *sym_side;
       const struct _HitMapTkrSide *hit_side;
       int                               twr;


       /* SIDE TOWER DISPLAY */
       for (hit_side = &hit->side[idx],
            twr      =  hit_side->twr,
            sym_side =  sym->side[hit_side->set],
            pattern  =  tkr[twr],
            idy      = 0;
            idy     < 9;
            idy++)
       {
           d[hit_side->pos[idy]] = sym_side[pattern & 0x3];
           pattern >>= 2;
       }


       /* TOP TOWER DISPLAY */
       if ( (idx < 16) && (pattern = tkr[idx]))
       {
           idy = 17;
           while ((pattern & (1<<idy)) == 0)  idy--;
           d[hit->top[idx]] = sym->top[idy];
       }
   }

   return;
}
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \fn  void printAcd (int    acd_top,
                      int      acd_x,
                      int      acd_y)
  \brief Provides a hex display of the raw bit patterns
  \param acd_top The ACD top hit map
  \param acd_x   The ACD X   hit map
  \param acd_y   The ACD Y   hit map
                                                                          */
/* ---------------------------------------------------------------------- */
static void printAcd (int acd_top,
                      int acd_x,
                      int acd_y)
{
   printf (" ACD Top: %8.8x\n"
           "       X: %8.8x\n"
           "       Y: %8.8x\n",
           acd_top,
           acd_x,
           acd_y);
   return;
}


/* ---------------------------------------------------------------------- *//*!

  \fn  void printAcdTkr (const int *tkr,
                         int    acd_top,
                         int      acd_x,
                         int      acd_y)
  \brief Provides a hex display of the raw bit patterns
  \param tkr     The 16 tower trigger patterns
  \param acd_top The ACD top hit map
  \param acd_x   The ACD X   hit map
  \param acd_y   The ACD Y   hit map
                                                                          */
/* ---------------------------------------------------------------------- */
static void printAcdTkr (const int *tkr,
                         int    acd_top,
                         int      acd_x,
                         int      acd_y)
{
   printf (" ACD Top: %8.8x     TKR C: %8.8x D: %8.8x E: %8.8x F: %8.8x\n" 
           "       X: %8.8x         8: %8.8x 9: %8.8x A: %8.8x B: %8.8x\n"
           "       Y: %8.8x         4: %8.8x 5: %8.8x 6: %8.8x 7: %8.8x\n"
        "                           0: %8.8x 1: %8.8x 2: %8.8x 3: %8.8x\n",
           acd_top,              tkr[0xc],tkr[0xd],tkr[0xe],tkr[0xf],
           acd_x,                tkr[0x8],tkr[0x9],tkr[0xa],tkr[0xb],
           acd_y,                tkr[0x4],tkr[0x5],tkr[0x6],tkr[0x7],
           tkr[0x0],tkr[0x1],tkr[0x2],tkr[0x3]);
}
/* ---------------------------------------------------------------------- */
