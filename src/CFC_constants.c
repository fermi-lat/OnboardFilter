/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file   CFC_constants.c
   \brief  Initializes the CAL calibration constants to a nominal set
           of values.
   \author JJRussell - russell@slac.stanford.edu

    The CAL calibration constants are initialized to a nominal set of
    values. These values are semi-realistic in the sense that they have
    about the right values as real calibration constants. They do not
    however, represent an real set of calibration constants.
                                                                         */
/* --------------------------------------------------------------------- */

#include "DFC/CFC_constants.h"
               


/* ---------------------------------------------------------------------- *//*!

  \def    CFC_MEV_TO_GAIN(_mev)
  \brief  Converts an energy in MEV to an energy gain.
                                                                          */
/* ---------------------------------------------------------------------- */
#define CFC_MEV_TO_GAIN(_mev) \
       (((_mev) * CFC_K_LEU_PER_MEV * CFC_K_GAIN_PER_LEU)                   \
        / (1 << CFC_S_LOGDATA_VALA))
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \def    CFC_K_LIMIT_RNGA0 
  \brief  Upper limit, in MeV of the range A 0 ADC
                                                                          *//*!
  \def    CFC_K_LIMIT_RNGA1 
  \brief  Upper limit, in MeV of the range A 1 ADC
                                                                          *//*!
  \def    CFC_K_LIMIT_RNGA2 
  \brief  Upper limit, in MeV of the range A 2 ADC
                                                                          *//*!
  \def    CFC_K_LIMIT_RNGA3 
  \brief  Upper limit, in MeV of the range A 3 ADC
                                                                          */
/* ---------------------------------------------------------------------- */
#define CFC_K_LIMIT_RNGA0    205
#define CFC_K_LIMIT_RNGA1   1640
#define CFC_K_LIMIT_RNGA2  13123
#define CFC_K_LIMIT_RNGA3 104986
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \def    CFC_K_PEDESTAL_RNGA0 
  \brief  Pedestal, in ADC units, of range A 0 ADC
                                                                          *//*!
  \def    CFC_K_PEDESTAL_RNGA1
  \brief  Pedestal, in ADC units, of range A 1 ADC  
                                                                          *//*!
  \def    CFC_K_PEDESTAL_RNGA2
  \brief  Pedestal, in ADC units, of range A 2 ADC    
                                                                          *//*!
  \def    CFC_K_PEDESTAL_RNGA3
  \brief  Pedestal, in ADC units, of range A 3 ADC      

                                                                          */
/* ---------------------------------------------------------------------- */
#define CFC_K_PEDESTAL_RNGA0    100  
#define CFC_K_PEDESTAL_RNGA1    100
#define CFC_K_PEDESTAL_RNGA2    100
#define CFC_K_PEDESTAL_RNGA3    100
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!
   
  \def    CFC_K_LIMIT_RNGB0 
  \brief  Upper limit, in MeV of the range B 0 ADC
                                                                          *//*!
  \def    CFC_K_LIMIT_RNGB1 
  \brief  Upper limit, in MeV of the range B 1 ADC
                                                                          *//*!
  \def    CFC_K_LIMIT_RNGB2 
  \brief  Upper limit, in MeV of the range B 2 ADC
                                                                          *//*!
  \def    CFC_K_LIMIT_RNGB3 
  \brief  Upper limit, in MeV of the range B 3 ADC
                                                                          */
/* ---------------------------------------------------------------------- */
#define CFC_K_LIMIT_RNGB0    205
#define CFC_K_LIMIT_RNGB1   1640
#define CFC_K_LIMIT_RNGB2  13123
#define CFC_K_LIMIT_RNGB3 104986
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \def    CFC_K_PEDESTAL_RNGB0
  \brief  Pedestal, in ADC units, of range B 0 ADC  
                                                                          *//*!
  \def    CFC_K_PEDESTAL_RNGB1
  \brief  Pedestal, in ADC units, of range B 0 ADC
                                                                          *//*!
  \def    CFC_K_PEDESTAL_RNGB2
  \brief  Pedestal, in ADC units, of range B 0 ADC  
                                                                          *//*!
  \def    CFC_K_PEDESTAL_RNGB3
  \brief  Pedestal, in ADC units, of range B 0 ADC  

                                                                          */
/* ---------------------------------------------------------------------- */
#define CFC_K_PEDESTAL_RNGB0    100
#define CFC_K_PEDESTAL_RNGB1    100
#define CFC_K_PEDESTAL_RNGB2    100
#define CFC_K_PEDESTAL_RNGB3    100
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!
 
  \fn       void CFC_constantsLatInit (struct _CFC_logConstants *gp)
  \brief    Initializes the calibration constants for all towers to
            a nominal set.
  \param gp The gains and pedestals for the tower to be initialized.

   This is not meant to be real in any sense of the word. It is to be
   used for debugging only.
                                                                          */
/* ---------------------------------------------------------------------- */  
void CFC_constantsLatInit (struct _CFC_logConstants *gp)
{
    int tower;

    for (tower = 0; tower < 16; tower++)
    {
        CFC_constantsTowerInit (gp);
        gp += CFC_K_LOGS_PER_TOWER;
    }
    
    return;
}
/* ---------------------------------------------------------------------- */  





/* ---------------------------------------------------------------------- *//*!
 
  \fn       void CFC_constantsTowerInit (struct _CFC_logConstants *gp)
  \brief    Initializes the calibration constants for this tower to a
            nominal set.
  \param gp The gains and pedestals for the tower to be initialized.

   This is not meant to be real in any sense of the word. It is to be
   used for debugging only.
                                                                          */
/* ---------------------------------------------------------------------- */  
void CFC_constantsTowerInit (struct _CFC_logConstants *gp)
{
    int log;


    
    for (log = 0; log < CFC_K_LOGS_PER_TOWER; log++)
    {
        gp[log].rng[0].gainA     = CFC_MEV_TO_GAIN (CFC_K_LIMIT_RNGA0);
        gp[log].rng[0].pedestalA = CFC_K_PEDESTAL_RNGA0;

        gp[log].rng[0].gainB     = CFC_MEV_TO_GAIN (CFC_K_LIMIT_RNGB0);
        gp[log].rng[0].pedestalB = CFC_K_PEDESTAL_RNGB0;
        
        gp[log].rng[1].gainA     = CFC_MEV_TO_GAIN (CFC_K_LIMIT_RNGA1);
        gp[log].rng[1].pedestalA = CFC_K_PEDESTAL_RNGA1;
        
        gp[log].rng[1].gainB     = CFC_MEV_TO_GAIN (CFC_K_LIMIT_RNGB1);
        gp[log].rng[1].pedestalB = CFC_K_PEDESTAL_RNGB1;

        gp[log].rng[2].gainA     = CFC_MEV_TO_GAIN (CFC_K_LIMIT_RNGA2);
        gp[log].rng[2].pedestalA = CFC_K_PEDESTAL_RNGA2;

        gp[log].rng[2].gainB     = CFC_MEV_TO_GAIN (CFC_K_LIMIT_RNGB2);
        gp[log].rng[2].pedestalB = CFC_K_PEDESTAL_RNGB2;

        gp[log].rng[3].gainA     = CFC_MEV_TO_GAIN (CFC_K_LIMIT_RNGA3);
        gp[log].rng[3].pedestalA = CFC_K_PEDESTAL_RNGA3;

        gp[log].rng[3].gainB     = CFC_MEV_TO_GAIN (CFC_K_LIMIT_RNGB3);
        gp[log].rng[3].pedestalB = CFC_K_PEDESTAL_RNGB3;
    }
    
    return;
    
}
/* ---------------------------------------------------------------------- */










