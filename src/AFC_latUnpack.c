/* ---------------------------------------------------------------------- *//*!
   
   \file  AFC_latUnpack.c
   \brief Routines to unpack the ACD data.
   \author JJRussell - russell@slac.stanford.edu

\verbatim

 CVS $Id$
\endverbatim 
                                                                          */
/* ---------------------------------------------------------------------- */


#include "ffs.h"
#include "DFC/AFC_latRecord.h"
#include "DFC/AFC_latUnpack.h"
#include "AFC_remapDef.h"


#define XZM   0
#define XZP  16
#define YZM  32
#define YZP  48
#define XY   64
#define RU   96
#define NA  112

#define _a 0
#define _b 1


#define GN(_list, _chn, _side) \
   ((_chn) + _list +  32* EBF_K_ACD_LIST_CNT * _side)
#define LIST(_brd, _chn)       \
  (((BRD_ ## _brd ## _ ## _chn) / 32) % EBF_K_ACD_LIST_CNT)

/*
 |  If the specified channel is a member of the _list,
 |      then value = 1 << num
 |      else value = 0
*/
#define MEMBER(_brd, _num, _list)                       \
              ((LIST(_brd, _num) == _list) ? (0x80000000 >> (_num)) : 0)

/*
 |  This macro selects the geographical channel number
*/
#define BRD_RMA(_brd, _chn)    ((BRD_ ## _brd ## _ ## _chn) & 0x1f)
#define BRD_RMC(_brd, _chn)     (BRD_ ## _brd ## _ ## _chn)

#define AFC_REMAP_BRD(_brd) { BRD_MSKS(_brd), BRD_RMAS(_brd), BRD_RMCS(_brd) }

  
#define BRD_MSK(_brd, _list)   \
   (                           \
     MEMBER(_brd,  0, _list)   \
   | MEMBER(_brd,  1, _list)   \
   | MEMBER(_brd,  2, _list)   \
   | MEMBER(_brd,  3, _list)   \
   | MEMBER(_brd,  4, _list)   \
   | MEMBER(_brd,  5, _list)   \
   | MEMBER(_brd,  6, _list)   \
   | MEMBER(_brd,  7, _list)   \
   | MEMBER(_brd,  8, _list)   \
   | MEMBER(_brd,  9, _list)   \
   | MEMBER(_brd, 10, _list)   \
   | MEMBER(_brd, 11, _list)   \
   | MEMBER(_brd, 12, _list)   \
   | MEMBER(_brd, 13, _list)   \
   | MEMBER(_brd, 14, _list)   \
   | MEMBER(_brd, 15, _list)   \
   | MEMBER(_brd, 16, _list)   \
   | MEMBER(_brd, 17, _list)   \
    )


#define BRD_MSKS(_brd)                 \
   { BRD_MSK(_brd, EBF_K_ACD_LIST_XZ), \
     BRD_MSK(_brd, EBF_K_ACD_LIST_YZ), \
     BRD_MSK(_brd, EBF_K_ACD_LIST_XY), \
     BRD_MSK(_brd, EBF_K_ACD_LIST_RU)  \
   }


#define BRD_RMAS(_brd)    \
   {                      \
     BRD_RMA(_brd,  0),   \
     BRD_RMA(_brd,  1),   \
     BRD_RMA(_brd,  2),   \
     BRD_RMA(_brd,  3),   \
     BRD_RMA(_brd,  4),   \
     BRD_RMA(_brd,  5),   \
     BRD_RMA(_brd,  6),   \
     BRD_RMA(_brd,  7),   \
     BRD_RMA(_brd,  8),   \
     BRD_RMA(_brd,  9),   \
     BRD_RMA(_brd, 10),   \
     BRD_RMA(_brd, 11),   \
     BRD_RMA(_brd, 12),   \
     BRD_RMA(_brd, 13),   \
     BRD_RMA(_brd, 14),   \
     BRD_RMA(_brd, 15),   \
     BRD_RMA(_brd, 16),   \
     BRD_RMA(_brd, 17)    \
   }

#define BRD_RMCS(_brd)    \
{                         \
     BRD_RMC(_brd,  0),   \
     BRD_RMC(_brd,  1),   \
     BRD_RMC(_brd,  2),   \
     BRD_RMC(_brd,  3),   \
     BRD_RMC(_brd,  4),   \
     BRD_RMC(_brd,  5),   \
     BRD_RMC(_brd,  6),   \
     BRD_RMC(_brd,  7),   \
     BRD_RMC(_brd,  8),   \
     BRD_RMC(_brd,  9),   \
     BRD_RMC(_brd, 10),   \
     BRD_RMC(_brd, 11),   \
     BRD_RMC(_brd, 12),   \
     BRD_RMC(_brd, 13),   \
     BRD_RMC(_brd, 14),   \
     BRD_RMC(_brd, 15),   \
     BRD_RMC(_brd, 16),   \
     BRD_RMC(_brd, 17)    \
   }


#define BRD_4LA_2   GN(NA,  0, _a)
#define BRD_4LA_16  GN(NA,  1, _a) 
#define BRD_4RA_13  GN(NA,  2, _a)
#define BRD_4RA_16  GN(NA,  3, _a)
#define BRD_1LA_1   GN(NA,  4, _a)
#define BRD_1LA_3   GN(NA,  5, _a)
#define BRD_2LA_16  GN(NA,  6, _a)
#define BRD_2RA_13  GN(NA,  7, _a)
#define BRD_3LA_1   GN(NA,  8, _a)
#define BRD_3LA_3   GN(NA,  9, _a)
#define BRD_2RA_16  GN(NA, 10, _a)

#define BRD_4LB_15  GN(NA,  0, _b)
#define BRD_4LB_16  GN(NA,  1, _b)
#define BRD_4RB_16  GN(NA,  2, _b)
#define BRD_2LB_16  GN(NA,  3, _b)
#define BRD_1RB_16  GN(NA,  4, _b)
#define BRD_1RB_14  GN(NA,  5, _b)
#define BRD_2LB_13  GN(NA,  6, _b)
#define BRD_2RB_16  GN(NA,  7, _b)
#define BRD_3RB_14  GN(NA,  8, _b)
#define BRD_3RB_16  GN(NA,  9, _b)
#define BRD_4LB_13  GN(NA, 10, _b)

#define BRD_2LA_5   GN(XZM,  0, _a)
#define BRD_2LB_12  GN(XZM,  0, _b)
#define BRD_2LA_11  GN(XZM,  1, _a)
#define BRD_2LB_6   GN(XZM,  1, _b)
#define BRD_2RA_3   GN(XZM,  2, _a)
#define BRD_2RB_14  GN(XZM,  2, _b)
#define BRD_2RA_7   GN(XZM,  3, _a)
#define BRD_2RB_10  GN(XZM,  3, _b)
#define BRD_2RA_12  GN(XZM,  4, _a)
#define BRD_2RB_5   GN(XZM,  4, _b)
#define BRD_2LA_3   GN(XZM,  5, _a)
#define BRD_2LB_14  GN(XZM,  5, _b)
#define BRD_2LA_10  GN(XZM,  6, _a)
#define BRD_2LB_7   GN(XZM,  6, _b)
#define BRD_2RA_2   GN(XZM,  7, _a)
#define BRD_2RB_15  GN(XZM,  7, _b)
#define BRD_2RA_8   GN(XZM,  8, _a)
#define BRD_2RB_9   GN(XZM,  8, _b)
#define BRD_2RA_14  GN(XZM,  9, _a)
#define BRD_2RB_3   GN(XZM,  9, _b)
#define BRD_2LA_2   GN(XZM, 10, _a)
#define BRD_2LB_15  GN(XZM, 10, _b)
#define BRD_2LA_9   GN(XZM, 11, _a)
#define BRD_2LB_8   GN(XZM, 11, _b)
#define BRD_2RA_0   GN(XZM, 12, _a)
#define BRD_2RB_17  GN(XZM, 12, _b)
#define BRD_2RA_9   GN(XZM, 13, _a)
#define BRD_2RB_8   GN(XZM, 13, _b)
#define BRD_2RA_15  GN(XZM, 14, _a)
#define BRD_2RB_2   GN(XZM, 14, _b)
#define BRD_2RA_17  GN(XZM, 15, _a)
#define BRD_2LB_17  GN(XZM, 15, _b)

#define BRD_4RA_12  GN(XZP,  0, _a)
#define BRD_4RB_5   GN(XZP,  0, _b)
#define BRD_4RA_7   GN(XZP,  1, _a)
#define BRD_4RB_10  GN(XZP,  1, _b)
#define BRD_4RA_3   GN(XZP,  2, _a)
#define BRD_4RB_14  GN(XZP,  2, _b)
#define BRD_4LA_12  GN(XZP,  3, _a)
#define BRD_4LB_5   GN(XZP,  3, _b)
#define BRD_4LA_6   GN(XZP,  4, _a)
#define BRD_4LB_11  GN(XZP,  4, _b)
#define BRD_4RA_14  GN(XZP,  5, _a)
#define BRD_4RB_3   GN(XZP,  5, _b)
#define BRD_4RA_8   GN(XZP,  6, _a)
#define BRD_4RB_9   GN(XZP,  6, _b)
#define BRD_4RA_2   GN(XZP,  7, _a)
#define BRD_4RB_15  GN(XZP,  7, _b)
#define BRD_4LA_11  GN(XZP,  8, _a)
#define BRD_4LB_6   GN(XZP,  8, _b)
#define BRD_4LA_5   GN(XZP,  9, _a)
#define BRD_4LB_12  GN(XZP,  9, _b)
#define BRD_4RA_15  GN(XZP, 10, _a)
#define BRD_4RB_2   GN(XZP, 10, _b)
#define BRD_4RA_9   GN(XZP, 11, _a)
#define BRD_4RB_8   GN(XZP, 11, _b)
#define BRD_4RA_0   GN(XZP, 12, _a)
#define BRD_4RB_17  GN(XZP, 12, _b)
#define BRD_4LA_10  GN(XZP, 13, _a)
#define BRD_4LB_7   GN(XZP, 13, _b)
#define BRD_4LA_3   GN(XZP, 14, _a)
#define BRD_4LB_14  GN(XZP, 14, _b)
#define BRD_4RA_17  GN(XZP, 15, _a)
#define BRD_4LB_17  GN(XZP, 15, _b)

#define BRD_2LA_1   GN(YZM,  0, _a)
#define BRD_1RB_3   GN(YZM,  0, _b)
#define BRD_1LA_6   GN(YZM,  1, _a)
#define BRD_1RB_7   GN(YZM,  1, _b)
#define BRD_1LA_9   GN(YZM,  2, _a)
#define BRD_1RB_8   GN(YZM,  2, _b)
#define BRD_1LA_10  GN(YZM,  3, _a)
#define BRD_1RB_11  GN(YZM,  3, _b)
#define BRD_1LA_14  GN(YZM,  4, _a)
#define BRD_4RB_1   GN(YZM,  4, _b)
#define BRD_2LA_0   GN(YZM,  5, _a)
#define BRD_1RB_2   GN(YZM,  5, _b)
#define BRD_1LA_5   GN(YZM,  6, _a)
#define BRD_1RB_6   GN(YZM,  6, _b)
#define BRD_1LA_8   GN(YZM,  7, _a)
#define BRD_1RB_9   GN(YZM,  7, _b)
#define BRD_1LA_11  GN(YZM,  8, _a)
#define BRD_1RB_12  GN(YZM,  8, _b)
#define BRD_1LA_15  GN(YZM,  9, _a)
#define BRD_4RB_0   GN(YZM,  9, _b)
#define BRD_1LA_0   GN(YZM, 10, _a)
#define BRD_1RB_1   GN(YZM, 10, _b)
#define BRD_1LA_4   GN(YZM, 11, _a)
#define BRD_1RB_5   GN(YZM, 11, _b)
#define BRD_1LA_7   GN(YZM, 12, _a)
#define BRD_1RB_10  GN(YZM, 12, _b)
#define BRD_1LA_12  GN(YZM, 13, _a)
#define BRD_1RB_13  GN(YZM, 13, _b)
#define BRD_1LA_16  GN(YZM, 14, _a)
#define BRD_1RB_17  GN(YZM, 14, _b)
#define BRD_1LA_17  GN(YZM, 15, _a)
#define BRD_1RB_0   GN(YZM, 15, _b)

#define BRD_3LA_14  GN(YZP,  0, _a)
#define BRD_2RB_1   GN(YZP,  0, _b)
#define BRD_3LA_10  GN(YZP,  1, _a)
#define BRD_3RB_11  GN(YZP,  1, _b)
#define BRD_3LA_9   GN(YZP,  2, _a)
#define BRD_3RB_8   GN(YZP,  2, _b)
#define BRD_3LA_6   GN(YZP,  3, _a)
#define BRD_3RB_7   GN(YZP,  3, _b)
#define BRD_4LA_1   GN(YZP,  4, _a)
#define BRD_3RB_3   GN(YZP,  4, _b)
#define BRD_3LA_15  GN(YZP,  5, _a)
#define BRD_2RB_0   GN(YZP,  5, _b)
#define BRD_3LA_11  GN(YZP,  6, _a)
#define BRD_3RB_12  GN(YZP,  6, _b)
#define BRD_3LA_8   GN(YZP,  7, _a)
#define BRD_3RB_9   GN(YZP,  7, _b)
#define BRD_3LA_5   GN(YZP,  8, _a)
#define BRD_3RB_6   GN(YZP,  8, _b)
#define BRD_4LA_0   GN(YZP,  9, _a)
#define BRD_3RB_2   GN(YZP,  9, _b)
#define BRD_3LA_16  GN(YZP, 10, _a)
#define BRD_3RB_17  GN(YZP, 10, _b)
#define BRD_3LA_12  GN(YZP, 11, _a)
#define BRD_3RB_13  GN(YZP, 11, _b)
#define BRD_3LA_7   GN(YZP, 12, _a)
#define BRD_3RB_10  GN(YZP, 12, _b)
#define BRD_3LA_4   GN(YZP, 13, _a)
#define BRD_3RB_5   GN(YZP, 13, _b)
#define BRD_3LA_0   GN(YZP, 14, _a)
#define BRD_3RB_1   GN(YZP, 14, _b)
#define BRD_3LA_17  GN(YZP, 15, _a)
#define BRD_3RB_0   GN(YZP, 15, _b)


#define BRD_2LA_6   GN(XY,  0, _a)
#define BRD_2LB_11  GN(XY,  0, _b)
#define BRD_2LA_12  GN(XY,  1, _a)
#define BRD_2LB_5   GN(XY,  1, _b)
#define BRD_2LA_17  GN(XY,  2, _a)
#define BRD_2LB_0   GN(XY,  2, _b)
#define BRD_2RA_6   GN(XY,  3, _a)
#define BRD_2RB_11  GN(XY,  3, _b)
#define BRD_2RA_11  GN(XY,  4, _a)
#define BRD_2RB_6   GN(XY,  4, _b)
#define BRD_2LA_7   GN(XY,  5, _a)
#define BRD_2LB_10  GN(XY,  5, _b)
#define BRD_2LA_13  GN(XY,  6, _a)
#define BRD_2LB_4   GN(XY,  6, _b)
#define BRD_2RA_4   GN(XY,  7, _a)
#define BRD_2RB_13  GN(XY,  7, _b)
#define BRD_2RA_5   GN(XY,  8, _a)
#define BRD_2RB_12  GN(XY,  8, _b)
#define BRD_2RA_10  GN(XY,  9, _a)
#define BRD_2RB_7   GN(XY,  9, _b)
#define BRD_2LA_8   GN(XY, 10, _a)
#define BRD_2LB_9   GN(XY, 10, _b)
#define BRD_2LA_14  GN(XY, 11, _a)
#define BRD_2LB_3   GN(XY, 11, _b)
#define BRD_2LA_15  GN(XY, 12, _a)
#define BRD_2LB_2   GN(XY, 12, _b)
#define BRD_4LA_15  GN(XY, 13, _a)
#define BRD_4LB_2   GN(XY, 13, _b)
#define BRD_4LA_9   GN(XY, 14, _a)
#define BRD_4LB_8   GN(XY, 14, _b)
#define BRD_4RA_10  GN(XY, 15, _a)
#define BRD_4RB_7   GN(XY, 15, _b)
#define BRD_4RA_5   GN(XY, 16, _a)
#define BRD_4RB_12  GN(XY, 16, _b)
#define BRD_4RA_4   GN(XY, 17, _a)
#define BRD_4RB_13  GN(XY, 17, _b)
#define BRD_4LA_14  GN(XY, 18, _a)
#define BRD_4LB_3   GN(XY, 18, _b)
#define BRD_4LA_8   GN(XY, 19, _a)
#define BRD_4LB_9   GN(XY, 19, _b)
#define BRD_4RA_11  GN(XY, 20, _a)
#define BRD_4RB_6   GN(XY, 20, _b)
#define BRD_4RA_6   GN(XY, 21, _a)
#define BRD_4RB_11  GN(XY, 21, _b)
#define BRD_4LA_17  GN(XY, 22, _a)
#define BRD_4LB_0   GN(XY, 22, _b)
#define BRD_4LA_13  GN(XY, 23, _a)
#define BRD_4LB_4   GN(XY, 23, _b)
#define BRD_4LA_7   GN(XY, 24, _a)
#define BRD_4LB_10  GN(XY, 24, _b)

#define BRD_2LA_4   GN(RU,  0, _a)
#define BRD_4RB_4   GN(RU,  0, _b)
#define BRD_4RA_1   GN(RU,  1, _a)
#define BRD_2LB_1   GN(RU,  1, _b)
#define BRD_2RA_1   GN(RU,  2, _a)
#define BRD_4LB_1   GN(RU,  2, _b)
#define BRD_4LA_4   GN(RU,  3, _a)
#define BRD_2RB_4   GN(RU,  3, _b)
#define BRD_3LA_13  GN(RU,  4, _a)
#define BRD_1RB_4   GN(RU,  4, _b)
#define BRD_3LA_2   GN(RU,  5, _a)
#define BRD_1RB_15  GN(RU,  5, _b)
#define BRD_1LA_2   GN(RU,  6, _a)
#define BRD_3RB_15  GN(RU,  6, _b)
#define BRD_1LA_13  GN(RU,  7, _a)
#define BRD_3RB_4   GN(RU,  7, _b)



const AFC_remap AFC_Remap =
{
  {
    AFC_REMAP_BRD(1LA),  /*!< Acd readout board 1LA readout order =  0 */
    AFC_REMAP_BRD(2LA),  /*!< Acd readout board 1LA readout order =  1 */
    AFC_REMAP_BRD(2RA),  /*!< Acd readout board 2RA readout order =  2 */
    AFC_REMAP_BRD(3LA),  /*!< Acd readout board 3LA readout order =  3 */
    AFC_REMAP_BRD(4LA),  /*!< Acd readout board 4LA readout order =  4 */
    AFC_REMAP_BRD(4RA),  /*!< Acd readout board 4RA readout order =  5 */
        
    AFC_REMAP_BRD(1RB),  /*!< Acd readout board 1RB readout order =  6 */
    AFC_REMAP_BRD(2LB),  /*!< Acd readout board 1LB readout order =  7 */
    AFC_REMAP_BRD(2RB),  /*!< Acd readout board 2RB readout order =  8 */
    AFC_REMAP_BRD(3RB),  /*!< Acd readout board 3RB readout order =  9 */
    AFC_REMAP_BRD(4LB),  /*!< Acd readout board 4LB readout order = 10 */
    AFC_REMAP_BRD(4RB)   /*!< Acd readout board 4RB readout order = 11 */
  }
};


static inline unsigned int remap (unsigned int list,
                            const unsigned char *rma);


/* ---------------------------------------------------------------------- *//*!
 
  @fn    unsigned int remap (unsigned int list, const unsigned char *rma)
  @brief Remaps the members in \e list according the remap array \e rma

  @param list The list to remap
  @param rma  The remap array. This array is index by a bit number and
              returns the remapped bit number
  @return     The remapped list
                                                                          */
/* ---------------------------------------------------------------------- */
static unsigned int remap (unsigned int list, const unsigned char *rma)
{
    unsigned int rlist = 0;

    /* Loop while there are still bits remaining */
    while (list)
    {
        int bit = FFS (list);            /* Locate the set bit            */
        list   &= ~ (0x80000000 >> bit); /* Eliminate from consideration  */
        rlist  |= (1 << rma[bit]);       /* Accumulate remap mask         */
    }
    
    return rlist;
}
/* ---------------------------------------------------------------------- */





 

/* ---------------------------------------------------------------------- *//*!

  @fn     int AFC_latUnpack (AFC_latRecord             *acd,
                              const unsigned short int *data)
  @brief  Unpacks an ACD record from the AEM into a standard structure

  @param  acd    Pointer to the data structure to receive the unpacked
                 data
  @param  data   Pointer to the AEM data
  @return        Status
                                                                          */
/* ---------------------------------------------------------------------- */
int AFC_latUnpack (AFC_latRecord *acd, const unsigned short int *data)
{

   int                 side;
   const AFC_remapBrd  *rmb      = AFC_Remap.brds;
   unsigned short int  *phas     = acd->phas;
   unsigned short int *uphas     = acd->uphas;
   unsigned int     start_parity = 0;
   unsigned int unmatched_parity = 0;
   int                    status = 0;
   int                     nphas = 0;
   
   
   /* Unpack a board at a time, first the A side boards, then the B side */
   for (side = 0; side < 2; side++)
   {
       int ibrd;
       
       /* Initialize the masks for this side */
       unsigned int     hit_xz = 0;
       unsigned int     hit_yz = 0;
       unsigned int     hit_xy = 0;
       unsigned int     hit_ru = 0;

       unsigned int paccept_xz = 0;
       unsigned int paccept_yz = 0;
       unsigned int paccept_xy = 0;
       unsigned int paccept_ru = 0;

       unsigned int naccept_xz = 0;
       unsigned int naccept_yz = 0;
       unsigned int naccept_xy = 0;
       unsigned int naccept_ru = 0;
       
       for (ibrd = 0; ibrd < 6; ibrd++, rmb++)
       {
           unsigned short int d0;
           unsigned short int d1;
           unsigned short int d2;
           unsigned int      hit;
           unsigned int  paccept;
           unsigned int  naccept;
           
           
           /* Retrieve the first 3 header words */
           d0     = data[0];
           d1     = data[1];
           d2     = data[2];

           
           /* Check for missing start bit */
           if ((signed short int)d0 >= 0)
               start_parity |= (0x10000 << (6*side + ibrd));

           
           /* Check for header parity error */
           if (d2 & (1 << 9))
               start_parity |= (0x0001 << (6*side + ibrd));
                  

           /* Reconstitute the hit and PHA accept masks */
           hit     = (((d0 & 0x7fff) << 3) | (d1 >> 13)) << 14;
           paccept = (((d1 & 0x1fff) << 5) | (d2 >> 11)) << 14;
           naccept = paccept;
           

           /* Advance the data pointer past the header */
           data  += 3;
           // printf (" hit: %8.8x  paccept: %8.8x\n",
           //          hit >> 14,
           //          paccept >> 14);


           /* Unpack PHAs (if any) */       
           if (d2 & (1<<10))
           {
               int     cnt = EBF_K_ACD_NCHNS_PER_BRD;
               short int w;

               /*
                | Extract each PHA value until the list is exhausted. The
                | PHA values are stored in an array randomly accessed by
                | the geographical channel number. The geographical channel
                | number is from the remapped channel number on this board.
                | Naturally if the accept list has been exhausted, there is
                | no channel to remap. These pha values are simply left
                | where they are and a pointer to them is created.
               */
               do
               {
                   /* Get the PHA value   */
                   w      = *data++;
                   nphas += 1;
                   
                   // printf ("Pha Value = %4.4x %4.4x\n", w, w >> 1);

                   if (w & 1)
                       unmatched_parity |= (0x0001 << (6*side + ibrd));

                   
                   /* Discard parity bit */
                   w >>=1;
                   
                   
                   /*
                    | If more than allowed number of PHAs, this is a fatal
                    | The event is undecodable
                   */
                   if (--cnt < 0)
                   {
                       status = -1;
                       goto EXIT;
                   }
                                      
                   
                   /* Only store in the output array if have more accepts */
                   if (naccept)
                   {
                       int chn  = FFS (naccept);
                       naccept &= ~(0x80000000 >> chn); 
                       phas[rmb->rmc[chn]] = w & ~0x2000;
                       // printf ("phas[%2.2x] = %4.4x\n", rmb->rmc[chn],
                       //                             phas[rmb->rmc[chn]]);
                   }
                   else
                   {
                       unmatched_parity |= (0x10000 << (6*side + ibrd));
                      *uphas++ = w;
                   }

               }
               while (w & (1 << 13));
           }
               
           
           
           /* Need to remap the channels for this board to the planes */
           if (hit | paccept)
           {
               /* Locate the channel remapping array */
               const unsigned char *rma = rmb->rma;
               
               /*
                | These mask off only the bits relevant to the specified plane
               */
               unsigned int xz_m = rmb->msks[EBF_K_ACD_LIST_XZ];
               unsigned int yz_m = rmb->msks[EBF_K_ACD_LIST_YZ];
               unsigned int xy_m = rmb->msks[EBF_K_ACD_LIST_XY];
               unsigned int ru_m = rmb->msks[EBF_K_ACD_LIST_RU];

               /* Any hits to remap ? */
               if (hit)
               {
                   hit_xz |= remap (hit & xz_m, rma);
                   hit_yz |= remap (hit & yz_m, rma);
                   hit_xy |= remap (hit & xy_m, rma);
                   hit_ru |= remap (hit & ru_m, rma);
               }

               /* Any accepts to remap */
               if (paccept)
               {
                   /*
                    | Form a list of only those accepts which have a
                    | corresponding PHA value, then remap that list.
                    | One could check that paccept was non-zero before
                    | doing this, but since remap performs correctly
                    | with an empty list this is not necessary. Basically
                    | the check is just an optimization in the case of
                    | an empty list. However, rarely will this path actually
                    | be taken. One expects if there are accepts, there
                    | will be PHA values to go with them, so the check
                    | is a waste of time.
                   */
                   paccept    &= ~naccept;
                   paccept_xz |= remap (paccept & xz_m, rma);
                   paccept_yz |= remap (paccept & yz_m, rma);
                   paccept_xy |= remap (paccept & xy_m, rma);
                   paccept_ru |= remap (paccept & ru_m, rma);
                   
                   /*
                    |  Now remap those only in the accept list that do
                    |  not have a corresponding value.
                   */

                   /* Are there any ? */
                   if (naccept)
                   {
                       naccept_xz |= remap (naccept & xz_m, rma);
                       naccept_yz |= remap (naccept & yz_m, rma);
                       naccept_xy |= remap (naccept & xy_m, rma);
                       naccept_ru |= remap (naccept & ru_m, rma);
                   }
               }
           }
       }

       
       /* Transfer the temporary lists to their permanent residency */
       acd->hits    [side][EBF_K_ACD_LIST_XZ] = hit_xz;
       acd->hits    [side][EBF_K_ACD_LIST_YZ] = hit_yz;
       acd->hits    [side][EBF_K_ACD_LIST_XY] = hit_xy;
       acd->hits    [side][EBF_K_ACD_LIST_RU] = hit_ru;

       acd->paccepts[side][EBF_K_ACD_LIST_XZ] = paccept_xz;
       acd->paccepts[side][EBF_K_ACD_LIST_YZ] = paccept_yz;
       acd->paccepts[side][EBF_K_ACD_LIST_XY] = paccept_xy;
       acd->paccepts[side][EBF_K_ACD_LIST_RU] = paccept_ru;

       acd->naccepts[side][EBF_K_ACD_LIST_XZ] = naccept_xz;
       acd->naccepts[side][EBF_K_ACD_LIST_YZ] = naccept_yz;
       acd->naccepts[side][EBF_K_ACD_LIST_XY] = naccept_xy;
       acd->naccepts[side][EBF_K_ACD_LIST_RU] = naccept_ru;
       
   }

  EXIT:
   
   acd->start_parity     = start_parity;
   acd->unmatched_parity = unmatched_parity;
   acd->nphas            = nphas;

   return status;
}

