#include "ftime.h"
#include <Windows.h>

#define REP_COUNT 10

double ftime_default(test_funct P, double E) {
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
    return diff*0.001;
}




