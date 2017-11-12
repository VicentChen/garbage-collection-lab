#ifndef __DUMP_H_
#define __DUMP_H_

/* $Id$ */

/*
 *  Papadimitriou Spiros
 *  spapadim@cs.cmu.edu
 *
 *  CS213 - Lab assignment 3
 *
 */

#include <stdio.h>

extern FILE *dumpfile;

extern char *dump_filename (const char *trace_filename);
extern void dump_start (const char *filename);
extern void dump_stop (void);
extern int dump_printf (const char *format, ...);


#endif /* __DUMP_H_ */

