#ifndef _WINDOWS_COMPAT_H
#define _WINDOWS_COMPAT_H

#ifdef _WIN32

#include <sys/timeb.h>
#include <sys/types.h>
#include <string.h>

typedef struct timeval {
  long tv_sec;
  long tv_usec;
} timeval;

#define inline __inline

inline void gettimeofday(struct timeval *t,void *timezone){
    struct _timeb timebuffer;
    _ftime(&timebuffer);
    t->tv_sec=timebuffer.time;
    t->tv_usec=timebuffer.millitm;
}
        

#endif

#endif

