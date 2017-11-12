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

extern int mem_init (void);
extern int mem_reinit (long size);
extern void *mem_sbrk (int increment);
extern int mem_pagesize (void);
extern long mem_usage (void);

typedef struct stats_s {
  /* stats for correctness */
  int garbage_count;
  int times_collected;
  /* stats for efficiency */
  double utilization;
  double max_utilization, min_utilization;
  double overall_utilization;
  long   max_total_size, memory_used;

  /* stats for speed */
  double execution_time;
  double optimal_time;

  /* main stats */
  int num_tracefiles;
  
  int sum_weights;
  double total_time, total_opt_time;
  double sum_overall_utilization;
  double total_ops;
  
  double avg_utilization;
  double perf_index;
  double p1, p2;
  
} stats_t;

#endif /* __MEMLIB_H_ */

