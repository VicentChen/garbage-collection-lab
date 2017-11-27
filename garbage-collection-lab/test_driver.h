#ifndef TEST_DRIVER_H__
#define TEST_DRIVER_H__
#include <stdio.h>

typedef struct {
	int score;
} test_status;

static char* default_tracefiles[] = {
    "kernels/amptjp.rep",
    "kernels/cccp.rep",
    "kernels/expr.rep",
    "artificial/binary.rep",
    "artificial/binary2.rep",
    "kernels/cp-decl.rep",
    //"artificial/coalescing.rep",
    //"artificial/random.rep",
    //"artificial/random2.rep",
    NULL
};

extern void test_todo();

#endif

