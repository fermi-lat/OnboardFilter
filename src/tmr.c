#if   defined (TMR_USE_GENERIC)

#   include "src/tmr.c.xx-xxx-xxx"

#elif defined(_ARCH_PPC) && defined(VXWORKS) && defined (__GNUC__)

#   include "src/tmr.c.vx-ppc-gcc"

#elif defined (__sun)

#   include "src/tmr.c.sx-xxx-xxx"

#elif defined (__GNUC__) && defined (__i386)

#   include "src/tmr.c.xx-x86-gcc"

#else

    /* Generic implementation */
#   include "src/tmr.c.xx-xxx-xxx"

#endif
