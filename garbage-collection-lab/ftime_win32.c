/* Utility for timing function evaluations */

/**
 * Win32 Realization
 */

#include <stdio.h>
#include <Windows.h>
#include <Mmsystem.h>

#include "ftime.h"


/* Do I want clock speed info? */
#define VERBOSE !IS_ALPHA

#define MAX_ETIME 86400   

#define DYNAMIC_ACCURACY 0

/*
 * Timing routines
 */

#if DYNAMIC_ACCURACY > 0 

 /* Win32: No NEED ON DYNAMIC ACCURACY,
  * As the system-wide ticker is accurate enough without extra cost.
  */


#else /* DYNAMIC_ACCURACY > 0 */

 /* (Win32 Adaptive Realization)
    Quicker timing version. Uses fixed repeat count
    and uses gettimeofday. On Linux it is accurate to the microsecond
    thanks to an implementation using the Pentium cycle counters */

#define REP_COUNT 10

double ftime_default(test_funct P, double E)
{
    int cnt = REP_COUNT;
    int stv, etv;
    double diff;

    stv = GetTickCount();
    while (cnt) {
        P();
        cnt--;
    }
    etv = GetTickCount();
    diff = (double)(etv - stv);
    diff /= REP_COUNT;
    return (1E-3*diff);
}

#endif /* DYNAMIC_ACCURACY > 0 */




