#ifndef __MALLOC_H_
#define __MALLOC_H_

/* $Id$ */

/*
 *  Papadimitriou Spiros
 *  spapadim@cs.cmu.edu
 *
 *  CS213 - Lab assignment 3
 *
 */
# define GC_VERSION_INFO "GC version: Origin version\n"

extern int gc_init(void);
extern void *gc_malloc(size_t size);

#endif /* __MALLOC_H_ */

