#ifndef TMR_H
#define TMR_H


/*----------------------------------------------------------------------- */
/*!

    \file   tmr.h
    \brief  Provides a set of timing macros
    \author JJRussell - russell@slac.stanford.edu

\verbatim

CVS $Id
\endverbatim


   \b ABSTRACT \n
      -------- \n
    This implements a facility to time accurately short duration events. The
    implementation is platform dependent, seeking to use the highest resolution
    clock that is available. The facility provides a uniform and consistent
    compile time interface. Note that this is not the same as a uniform and
    consistent run time interface. In particular the timer units may vary
    platform to platform. This difference is taken out by a typedef of the
    TMR units (TMR_ticks). The resolution is also platform dependent, so a
    function to convert process timer units to NSECS is provided.
  
    Finally, to keep the overhead low, consistent with timing short duration
    events, certain compromises where made. In general, do not use this
    facility to time events > 200 secs. The normal system clocks should be
    used instead.

  \b EXAMPLE \n
     ------- \n
  \code  
    TMR_tick     beg;
    TMR_tick     end;
    TMR_tick elapsed;
  
    beg     = TMR_GET();
    .
    eventToTime ();
    .
    end     = TMR_GET();
    elapsed = end - beg;
  
    printf ("Begin   Time: " TMR_FORMAT "tickss\n", beg);
    printf ("End     Time: " TMR_FORMAT "ticks\n", end);
    printf ("Elasped Time: " TMR_FORMAT "tickss\n", elapsed);
    printf ("Elasped Time: " TMR_FORMAT "ticks\n",
                             TMR_TO_NSECS (elapsed);
  \endcode
                                                                          */
/*----------------------------------------------------------------------- */



/*
 |  HISTORY
 |  -------
 |  01.24.04 jjr - Added routines to support usecs.nsecs
 |  02.04.02 jjr - Cloned from BBC version to make DFC self-contained
 |
 */


/*----------------------------------------------------------------------- */
/*!

 \typedef TMR_tick
 \brief   The abstract type of a TMR tick.

  This is the abstract type returned by TMR_GET(). Since it varies on
  a machine-by-machine basis, no direct manipulation of this data type
  should be attempted. Use only the TMR routines and macros with this
  type.
									  */
/*----------------------------------------------------------------------- */




/*----------------------------------------------------------------------- */
/*!

  \struct _TMR_usecs_nsecs
  \brief   Structure to hold the time in terms of two integers, one
           giving the number of micro-seconds, the other the number of
           milliseconds
                                                                          *//*!
  \typedef TMR_usecs_nsecs
  \brief   Typedef for struct \e _TMR_usecs_nsecs
                                                                          */
/*----------------------------------------------------------------------- */
typedef struct _TMR_usecs_nsecs
{
  int          usecs; /*!< The number of micro-seconds                    */
  unsigned int nsecs; /*!< The number of nano-seconds                     */
}
TMR_usecs_nsecs;
/*----------------------------------------------------------------------- */




/*----------------------------------------------------------------------- */
/*!
  
    \def    TMR_GET()
    \brief  Macro to get the processor time
                                                                          */
/*----------------------------------------------------------------------- */




/*----------------------------------------------------------------------- */
/*!
  
    \def    TMR_TO_NSECS(_delta)
    \brief  Convert a delta time in processor time units to nanoseconds
                                                                          */
/*----------------------------------------------------------------------- */





/*----------------------------------------------------------------------- */
/*!
  
    \def    TMR_FORMAT()
    \brief  Provides a uniform way to format timer ticks
                                                                          */
/*----------------------------------------------------------------------- */




/*----------------------------------------------------------------------- */
/*!
  
    \def    TMR_DELTA(_beg, _end)
    \brief  Subtracts two times in ticks, returning the result in ticks
                                                                          */
/*----------------------------------------------------------------------- */




/*----------------------------------------------------------------------- */
/*!
  
    \def    TMR_DELTA_IN_NSECS(_beg, _end)
    \brief  Subtracts two times in tickss, returning the result in
            nanoseconds
                                                                          */
/*----------------------------------------------------------------------- */
    



/*----------------------------------------------------------------------- */
/*               These are processor specific definitions                 */
/*----------------------------------------------------------------------- */
#if    defined(TMR_USE_GENERIC)

   #include "src/tmr.h.xx-xxx-xxx"

#elif    defined(__sun)

   #include "src/tmr.h.sx-xxx-xxx"

#elif defined(_ARCH_PPC) && defined (__GNUC__)

   #include "src/tmr.h.xx-ppc-gcc"

#elif defined (__i386)   && defined (__GNUC__)

   #include "src/tmr.h.xx-x86-gcc"

#else

   /* Generic implementation */
   #include "src/tmr.h.xx-xxx-xxx"

#endif




/* ---------------------------------------------------------------------- */
/*!
   
   \fn                int TMR_initialize (void)
   \brief             Initializes the TMR facility

   This is a one time only call to initialize internal data structures.
   This routine should be called as part of the library initialization
   sequence.
                                                                          */
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

   \fn                int TMR_nsecs_to_ticks (int nanoseconds)
   \brief             Convert a number of nanoseconds to PTUs
   \param nanoseconds The number of nanoseconds to convert to PTUs
   \return            The number of equivalent PTUs
                                                                          */
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- */
/*!
   
   \fn                unsigned int TMR_ticks_to_nsecs (TMR_tick ticks)
   \brief             Convert a number of ticks to nanoseconds.
   \param       ticks The number of ticks to convert to nanoseconds
   \return            The number of equivalent nanoseconds
                                                                          */
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!
   
   \fn                TMR_usecs_nsecs TMR_ticks_to_usecs_nsecs (TMR_tick ticks)
   \brief             Convert a number of ticks useconds.nanoseconds
   \param       ticks The number of ticks to convert
   \return            The number of equivalent useconds, nanoseconds
                                                                          */
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- */
/*!

   \fn                unsigned int TMR_frequency (void)
   \brief             Get the frequency of the processor timer.
   \return            The frequency, in Hertz, to the processor timer
                                                                          */
/* ---------------------------------------------------------------------- */



#ifdef __cplusplus
extern "C" {
#endif    

    
/*----------------------------------------------------------------------- */
/*                     FUNCTION PROTOTYPES                                */
/*----------------------------------------------------------------------- */
extern int             TMR_initialize           (void);
extern TMR_tick        TMR_nsecs_to_ticks       (int nanoseconds);
extern unsigned int    TMR_ticks_to_nsecs       (TMR_tick  ticks);
extern TMR_usecs_nsecs TMR_ticks_to_usecs_nsecs (TMR_tick  ticks);
extern unsigned int    TMR_frequency            (void);
/*----------------------------------------------------------------------- */

    
#ifdef __cplusplus
}
#endif





#endif
