#ifndef EBF_SWAP_H
#define EBF_SWAP_H



/*------------------------------------------------------------------------
| CVS $Id
+-------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------- *//*!
   
   \file  EBF_swap.h
   \brief DAQE word swapping utilities, Interface
   \author JJRussell - russell@slac.stanford.edu

    Input routines for reading files from DAQEngine.
                                                                         */
/* --------------------------------------------------------------------- */



#ifdef __cplusplus
extern "C" {
#endif


    
/* --------------------------------------------------------------------- */

/* Single word swap routines */    
extern unsigned int EBF_swap32            (unsigned int   w);
extern unsigned int EBF_swap32_lclXbig    (unsigned int   w);
extern unsigned int EBF_swap32_lclXlittle (unsigned int   w);

extern unsigned int EBF_swap16            (unsigned short w);
extern unsigned int EBF_swap16_lclXbig    (unsigned short w);
extern unsigned int EBF_swap16_lclXlittle (unsigned short w);

/* Array swap routines */    
extern void EBF_swap32N            (unsigned int   *w, int n);
extern void EBF_swap32_lclXbigN    (unsigned int   *w, int n);
extern void EBF_swap32_lclXlittleN (unsigned int   *w, int n);

extern void EBF_swap16N            (unsigned short *w, int n);
extern void EBF_swap16_lclXbigN    (unsigned short *w, int n);
extern void EBF_swap16_lclXlittleN (unsigned short *w, int n);
/* --------------------------------------------------------------------- */    

    
    
#ifdef __cplusplus    
}
#endif


#endif







