/* ---------------------------------------------------------------------- */
/*!
   
   \file   EDM.c
   \brief  Event Diagnostic Message services, implementation
   \author JJRussell - russell@slac.stanford.edu

\verbatim
 CVS $Id: EDM.c,v 1.1 2004/07/12 18:09:12 golpa Exp $
\endverbatim 

   The facility allows one to embed \em printf style statements in the
   program.
									  */
/* ---------------------------------------------------------------------- */



/*
 *   History  Who  What
 *  --------  ---  ----
 *  01.24.04  jjr  Added documentation
 *  01.17.04  jjr  Created
 */



#include <stdio.h>
#include <stdarg.h>
#define   EDM_USE
#include "DFC/EDM.h"




/* --------------------------------------------------------------------- */
/*!

  \fn      int EDM_xPrintf (EDM_level message_level, 
                            EDM_level   print_level,
			    const char      *format,
                            ...)
  \brief   Prints the message if \a message_level is >= \a print_level
  \return  The value of the resulting printf. If the result is 0, except
           in very pathology situations, it means the routine was not
           called because the message level is below the print level

  \param   message_level The message level, one of the enumeration
                         EDM_level.  This is number is generally 
                         associated with a particular message

  \param     print_level The print level, one of the enumeration
                         EDM_level. This is generally a more global
                         number controlling which level of messages
                         get printed.

  \param          format The printf format statement

  \param             ... The printf format arguments
   
   In general the caller should use the macro EDM_XPRINTF to access this
   routine. This macro can be compiled to a NOP when the code writer 
   wishes to configure the same piece of code with and without the EDM
   facility as an active component
									 */
/* --------------------------------------------------------------------- */
int EDM_xPrintf (EDM_level   message_level,
                 EDM_level   print_level,
                 const char *format, ...)
{
  int n = 0;

  /* Should this message be even reported */
  if (message_level >= print_level) 
  {
    va_list ap;
    va_start    (ap, format);
    n = vprintf (format, ap);
    va_end      (ap);
  }

  return n;
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- */
/*!

  \fn      int EDM_debugPrintf (EDM_level   print_level,
			        const char      *format,
                                ...)
  \brief   Prints the message if \a EDM_K_DEBUG is >= \a print_level
  \return  The value of the resulting printf. If the result is 0, except
           in very pathology situations, it means the routine was not
           called because the message level is below the print level

  \param     print_level The print level, one of the enumeration
                         EDM_level. This is generally a more global
                         number controlling which level of messages
                         get printed.

  \param          format The printf format statement

  \param             ... The printf format arguments
   
   This is just a convenience routine for calling EDM_xPrintf with a 
   message level of EDM_K_DEBUG.

   In general the caller should use the macro EDM_DEBUGPRINTF to access this
   routine. This macro can be compiled to a NOP when the code writer 
   wishes to configure the same piece of code with and without the EDM
   facility as an active component.
									 */
/* --------------------------------------------------------------------- */
int EDM_debugPrintf (EDM_level   print_level,
		     const char *format, ...)
{
  int n = 0;

  /* Should this message be even reported */
  if (EDM_K_DEBUG >= print_level) 
  {
    va_list ap;
    va_start    (ap, format);
    n = vprintf (format, ap);
    va_end      (ap);
  }

  return n;
}
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- */
/*!

  \fn      int EDM_infoPrintf (EDM_level   print_level,
			       const char      *format,
                            ...)
  \brief   Prints the message if \a EDM_K_INFO is >= \a print_level
  \return  The value of the resulting printf. If the result is 0, except
           in very pathology situations, it means the routine was not
           called because the message level is below the print level

  \param     print_level The print level, one of the enumeration
                         EDM_level. This is generally a more global
                         number controlling which level of messages
                         get printed.

  \param          format The printf format statement

  \param             ... The printf format arguments
   
   This is just a convenience routine for calling EDM_xPrintf with a 
   message level of EDM_K_INFO.

   In general the caller should use the macro EDM_INFOPRINTF to access this
   routine. This macro can be compiled to a NOP when the code writer 
   wishes to configure the same piece of code with and without the EDM
   facility as an active component.
									 */
/* --------------------------------------------------------------------- */
int EDM_infoPrintf (EDM_level   print_level,
                    const char *format, ...)
{
  int n = 0;

  /* Should this message be even reported */
  if (EDM_K_INFO >= print_level) 
  {
    va_list ap;
    va_start    (ap, format);
    n = vprintf (format, ap);
    va_end      (ap);
  }

  return n;
}
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- */
/*!

  \fn      int EDM_warningPrintf (EDM_level   print_level,
			          const char      *format,
                            ...)
  \brief   Prints the message if \a EDM_K_WARNING is >= \a print_level
  \return  The value of the resulting printf. If the result is 0, except
           in very pathology situations, it means the routine was not
           called because the message level is below the print level

  \param     print_level The print level, one of the enumeration
                         EDM_level. This is generally a more global
                         number controlling which level of messages
                         get printed.

  \param          format The printf format statement

  \param             ... The printf format arguments
   
   This is just a convenience routine for calling EDM_xPrintf with a 
   message level of EDM_K_WARNING.

   In general the caller should use the macro EDM_WARNINGPRINTF to access 
   this routine. This macro can be compiled to a NOP when the code writer 
   wishes to configure the same piece of code with and without the EDM
   facility as an active component.
									 */
/* --------------------------------------------------------------------- */
int EDM_warningPrintf (EDM_level   print_level,
                       const char *format, ...)
{
  int n = 0;

  /* Should this message be even reported */
  if (EDM_K_WARNING >= print_level) 
  {
    va_list ap;
    va_start    (ap, format);
    n = vprintf (format, ap);
    va_end      (ap);
  }

  return n;
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- */
/*!

  \fn      int EDM_errorPrintf (EDM_level   print_level,
			        const char      *format,
                            ...)
  \brief   Prints the message if \a EDM_K_ERROR is >= \a print_level
  \return  The value of the resulting printf. If the result is 0, except
           in very pathology situations, it means the routine was not
           called because the message level is below the print level

  \param     print_level The print level, one of the enumeration
                         EDM_level. This is generally a more global
                         number controlling which level of messages
                         get printed.

  \param          format The printf format statement

  \param             ... The printf format arguments
   
   This is just a convenience routine for calling EDM_xPrintf with a 
   message level of EDM_K_ERROR.

   In general the caller should use the macro EDM_ERRORPRINTF to access this
   routine. This macro can be compiled to a NOP when the code writer 
   wishes to configure the same piece of code with and without the EDM
   facility as an active component.
									 */
/* --------------------------------------------------------------------- */
int EDM_errorPrintf (EDM_level   print_level,
		     const char *format, ...)
{
  int n = 0;

  /* Should this message be even reported */
  if (EDM_K_ERROR >= print_level) 
  {
    va_list ap;
    va_start    (ap, format);
    n = vprintf (format, ap);
    va_end      (ap);
  }

  return n;
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- */
/*!

  \fn      int EDM_fatalPrintf (EDM_level   print_level,
			        const char      *format,
                                ...)
  \brief   Prints the message if \a EDM_K_ERROR is >= \a print_level
  \return  The value of the resulting printf. If the result is 0, except
           in very pathology situations, it means the routine was not
           called because the message level is below the print level

  \param     print_level The print level, one of the enumeration
                         EDM_level. This is generally a more global
                         number controlling which level of messages
                         get printed.

  \param          format The printf format statement

  \param             ... The printf format arguments
   
   This is just a convenience routine for calling EDM_xPrintf with a 
   message level of EDM_K_FATAL.

   In general the caller should use the macro EDM_FATALPRINTF to access this
   routine. This macro can be compiled to a NOP when the code writer 
   wishes to configure the same piece of code with and without the EDM
   facility as an active component.
									 */
/* --------------------------------------------------------------------- */
int EDM_fatalPrintf (EDM_level   print_level,
		     const char *format, ...)
{
  int n = 0;

  /* Should this message be even reported */
  if (EDM_K_FATAL >= print_level) 
  {
    va_list ap;
    va_start    (ap, format);
    n = vprintf (format, ap);
    va_end      (ap);
  }

  return n;

}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- */
/*!

  \fn      int EDM_printf (const char *format, ...)
  \brief   Unconditional print
  \return  The value of the resulting printf. If the result is 0, except
           in very pathology situations, it means the routine was not
           called because the message level is below the print level

  \param          format The printf format statement

  \param             ... The printf format arguments
   

   In general the caller should use the macro EDM_PRINTF to access this
   routine. This macro can be compiled to a NOP when the code writer 
   wishes to configure the same piece of code with and without the EDM
   facility as an active component.
									 */
/* --------------------------------------------------------------------- */
int EDM_printf  (const char *format, ...)
{
  int  n = 0;
  va_list ap;
  va_start (ap, format);
  n = vprintf (format, ap);
  va_end (ap);
  return n;
}
/* --------------------------------------------------------------------- */
