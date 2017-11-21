#ifndef __MEMLIB_H_
#define __MEMLIB_H_


/* $Id$ */

/*
 *  Papadimitriou Spiros
 *  spapadim@cs.cmu.edu
 *
 *  CS213 - Lab assignment 3
 *
 */

#include <string.h>

#define DSEG_MAX 16*1024*1024  /* 16 Mb */

extern char *dseg_lo, *dseg_hi;
extern long dseg_size;

extern int mem_init(void);
extern int mem_reinit(long size);
extern void *mem_sbrk(int increment);
extern int mem_pagesize(void);
extern long mem_usage(void);
#endif /* __MEMLIB_H_ */

