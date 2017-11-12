/* $Id$ */

/*
 *  Khalil Amiri
 *  amiri+@cs.cmu.edu
 *
 *  CS213 - Fall 99 - Lab assignment 3
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <float.h>

#include "ftime.h"
#include "memlib.h"
#include "malloc.h"
#include "dump.h"
#include "useful.h"
#include "getopt.h"
#include "tracefiles.h"

void usage(void);

/* magic numbers */
/* Win32: REF_TIME Adaptive Modification */
#define SPACE_UTIL_METRIC_WEIGHT 0.33
//#define REF_TIME 110.0
#define REF_TIME 0.035
#define MIN(a,b) ( (a) < (b) ? (a):(b) )

/* test data structures */
typedef struct range_t {
    struct range_t *next;
    char *lo, *hi;
} range_t;

typedef struct {
    enum {ALLOC, FREE} type;
    long index;
    long size;
} memop_t;

typedef struct {
    int  weight;
    unsigned  num_blocks;
    long num_ops;
    long suggested_heap_size;
    memop_t *ops;
    char *blocks[20000];
    size_t *block_sizes;
} trace_t;


stats_t *stats;

/* Command-line options */
static int verbose = 1;
static double ftime_tolerance = 0.05;
static int dump = 0;

/*****************
 * Range routines
 *****************/

static range_t *ranges = NULL;
static range_t *garbage = NULL;
static trace_t *t = NULL;

/* Check if a pointer is 4-byte aligned */
#define IS_ALIGNED(p)  ((((unsigned long)(p))%4) == 0)

int add_range (char *lo, long size)
{
    char *hi = lo + size - 1;
    range_t *p;

    assert(size > 0);

    /* Check alignment */
    if (!IS_ALIGNED(lo)) {
        fprintf(stderr, "Misaligned region returned\n");
        if (verbose)
            fprintf(stderr, "Address: %p\n", lo);
        return 0;
    }

    /* Region lies within heap */
    if (lo < dseg_lo || lo > dseg_hi || hi < dseg_lo || hi > dseg_hi) {
        fprintf(stderr, "Region lies outside heap area\n");
        if (verbose) {
            fprintf(stderr, "Region: %p - %p\n", lo, hi);
            fprintf(stderr, "Heap: %p - %p\n", dseg_lo, dseg_hi);
        }
        return 0;
    }

    /* Region does not overlap any other region */
    for (p = ranges;  p != NULL;  p = p->next) {
        if ((lo >= p->lo && lo <= p-> hi) ||
            (hi >= p->lo && hi <= p->hi)) {
            fprintf(stderr, "Region overlap detected\n");
            if (verbose) {
                fprintf(stderr, "Region 1: %p - %p\n", lo, hi);
                fprintf(stderr, "Region 2: %p - %p\n", p->lo, p->hi);
            }
            return 0;
        }
    }

    /* Clobber region (zero miswritten control records) */
    //bzero(lo, size);
	memset(lo, 0, size);

    p = (range_t *)malloc(sizeof(range_t));
    p-> next = ranges;
    p->lo = lo;
    p->hi = hi;
    ranges = p;
    return 1;
}

void remove_range (char *lo)
{
    range_t *p, **prevpp = &ranges;

    for (p = ranges;  p != NULL; p = p->next) {
        if (p->lo == lo) {
            *prevpp = p->next;
            
	    p->next = garbage;
	    garbage = p;
            break;
        }
        prevpp = &(p->next);
    }
}

void verify_garbage(char *lo)
{
    range_t *p, **prevpp = &garbage;
    int i;

    if(ranges==NULL && garbage==NULL) return;

    for (p = garbage;  p != NULL; p = p->next) {
        if (p->lo == lo) {
            int size = p->hi - p->lo + 1;

            *prevpp = p->next;
            
            /* Clobber region again (zero 
             * miswritten control records) */
            //bzero(p->lo, size);
			memset(p->lo, 0, size);
            break;
        }
        prevpp = &(p->next);
    }

    if(p != NULL) {
      free(p);
    } else {
      fprintf(stderr,"Failed correctness test, addr %p (head at %p)",
	      lo, lo - sizeof(Tree)) ;
      fprintf(stderr," not really garbage\n");
      if(verbose) {
	for(p = ranges; p != NULL; p = p->next) {
	  if(p->lo == lo) {
	    fprintf(stderr,"found ptr %p in allocated ranges\n",lo);
	    break;
	  }
	}
	for(i=0; i< 20000; i++) {
	  if(t->blocks[i] == lo) {
	    fprintf(stderr,"found ptr in trace->blocks[%d]\n",i);
	    break;
	  }
	}
      }
      exit(1);
    }
}

void print_unfound()
{
  range_t *p;

  for(p = garbage; p!=NULL; p=p->next) {
      fprintf(stderr,"gc did not find %p (size %d)\n",
	      p->lo, (int)(p->hi - p->lo));
  }

}

void verify_complete()
{
  range_t *p;
  int count = 0;

  /*fprintf(stderr,"Verifying garbage collection is complete\n");*/
  for(p = garbage; p!=NULL; p=p->next) {
    count++;
  }

  stats->garbage_count = count;
  stats->times_collected++;
  /* sometimes a pointer to the last deleted block is left in a reg.
   * so it's ok to miss 1 block as long as it is found on the next
   * pass. (i.e., there should never be more than one)
   */
  if(stats->garbage_count > 1) {
    fprintf(stderr,"Failed correctness test, too much leftover garbage\n");
    if(verbose) {
      print_unfound();
    }
    fprintf(stderr,"Please run your program under gdb.\n\n");
    exit(1);
  }
  
}

void clear_ranges (void)
{
    range_t *p, *pnext;

    for (p = ranges;  p != NULL;  p = pnext) {
        pnext = p->next;
        free(p);
    }

    for(p = garbage; p != NULL; p = pnext) {
      pnext = p->next;
      free(p);
    }

    ranges = NULL;
    garbage = NULL;
}


/*********************
 * Tracefile routines
 *********************/

void
read_trace (trace_t *trace, char *filename)
{
    FILE *tracefile;
    long num_blocks, num_ops;

    tracefile = fopen(filename, "r");
    if (tracefile != NULL) {

        char type[10];
        unsigned index, size;
        unsigned max_index = 0;
        unsigned op_index;

        fscanf(tracefile, "%ld", &(trace->suggested_heap_size));
        fscanf(tracefile, "%ld", &num_blocks);
        fscanf(tracefile, "%ld", &num_ops);
        fscanf(tracefile, "%d", &(trace->weight));
        trace->num_blocks = num_blocks;
        trace->num_ops = num_ops;

        trace->ops = (memop_t *)malloc(num_ops * sizeof(memop_t));
        trace->block_sizes = (size_t *)malloc(num_blocks * sizeof(size_t));

        index = 0;
        op_index = 0;
        while (fscanf(tracefile, "%s", type) != EOF) {

            switch(type[0]) {
            case 'a':
                fscanf(tracefile, "%u %u", &index, &size);
                trace->ops[op_index].type = ALLOC;
                trace->ops[op_index].index = index;
                trace->ops[op_index].size = size;
                max_index = (index > max_index) ? index : max_index;
                break;
            case 'f':
                fscanf(tracefile, "%ud", &index);
                trace->ops[op_index].type = FREE;
                trace->ops[op_index].index = index;
                break;
            default:
                fprintf(stderr, "Bogus type character\n");
                exit(-1);
            }
            op_index++;

        }
        fclose(tracefile);

        assert((signed)max_index == num_blocks-1);
        assert(num_ops == (signed)op_index);

        if (verbose) {
            fprintf(stderr,"Read tracefile: %s\n", filename);
            fprintf(stderr,"Blocks: %ld\n", num_blocks);
            fprintf(stderr,"Operations: %ld\n", num_ops);
            fprintf(stderr,"Weight: %d\n\n", trace->weight);
        }

    }

}

void free_trace (trace_t *trace)
{
    free(trace->ops);
    free(trace->block_sizes);
}

/********************
 * Testing functions
 ********************/

/* Test correctness */

void correctness (trace_t *trace)
{
    long i;
    int total_size = 0;

    mem_reinit(0);
    clear_ranges();
    gc_init();
    t = trace;

    /* Initialize statistics */
    stats->max_utilization = 0.0;
    stats->min_utilization = DBL_MAX;
    stats->max_total_size = 0;

    for (i = 0;  i < trace->num_ops;  i++)
        switch (trace->ops[i].type) {
        case ALLOC:
          {
            int index = trace->ops[i].index;
            int size = trace->ops[i].size;

            char *p = gc_malloc(size);

            /* Test returned region (misalignment,
             * overlap, miswritten control data) */
            if (!add_range(p, size)) {
                fprintf(stderr, "Failed correctness test!\n");
                if (verbose)
                    fprintf(stderr, "Operation: %ld (out of %ld)\n", 
			    i, trace->num_ops);
                fprintf(stderr, "Please run your program under gdb.\n\n");
                exit(1);
            }
            /* Keep track of current total size
             * of all allocated blocks */
            total_size += size;

            /* Update statistics */
            stats->max_total_size = (total_size > stats->max_total_size) ?
                             total_size : stats->max_total_size;
            stats->utilization = (double)total_size / (double)mem_usage();
            stats->max_utilization = (stats->utilization > stats->max_utilization) ?
                              stats->utilization : stats->max_utilization;
            stats->min_utilization = (stats->utilization < stats->min_utilization) ?
                              stats->utilization : stats->min_utilization;

            /* Remember region */
            trace->blocks[index] = p;
            trace->block_sizes[index] = size;

            break;
          }
        case FREE:
          {
            int index = trace->ops[i].index;
            char *block = trace->blocks[index];
	    int size = trace->block_sizes[index];
            /* Keep track of current total size
             * of all allocated blocks */
            total_size -= size;

            /* Remove region from list */
            remove_range(block);

	    trace->blocks[index] = NULL;

            break;
          }
        }

    if(stats->times_collected > 0) {
      fprintf(stderr,"%d garbage blocks not found by garbage collector\n",
	      stats->garbage_count);
      fprintf(stderr,"Correctness test passed\n\n");
    } else {
      fprintf(stderr,"Garbage collection did not run.\n");
    }
  
    stats->memory_used = mem_usage();
    stats->overall_utilization = (double)stats->max_total_size / (double)stats->memory_used;

    printf("Efficiency measurements completed\n");
    if (verbose) {
        printf("Maximum total size: %ld\n", stats->max_total_size);
        printf("Final heap size: %ld\n", mem_usage());
        printf("Overall utilization: %6.3f%%\n", 100.0*stats->overall_utilization);
    }
    printf("\n");

}


/* Trace creation */

void dump_write (trace_t *trace)
{   
    long i;

    mem_reinit(0);
    clear_ranges();
    gc_init();


    for (i = 0;  i < trace->num_ops;  i++)
        switch (trace->ops[i].type) {
        case ALLOC:
          {
            int index = trace->ops[i].index;
            int size = trace->ops[i].size;

            char *p = gc_malloc(size);

            /* Remember region and size */
            trace->blocks[index] = p;
            trace->block_sizes[index] = size;

            /* Write dump entry (do after malloc, so
             * sbrk dump entry will be written */
            dump_printf("m %ld %ld\n", (long)(p - dseg_lo), (long)size);

            break;
          }
        case FREE:
          {
            int index = trace->ops[i].index;
            char *block = trace->blocks[index];
            int size = trace->block_sizes[index];

	    trace->blocks[index] = NULL;
            /* Write dump entry */
            dump_printf("f %ld %ld\n", (long)(block - dseg_lo), (long)size);

            break;
          }
        }

    printf("Dump written\n\n");
}

/* Time speed */

static trace_t *trace;

static void speed_funct (void)
{
    long i;

    mem_reinit(0);
    clear_ranges();
    gc_init();


    for (i = 0;  i < trace->num_ops;  i++)
        switch (trace->ops[i].type) {
        case ALLOC:
          {
            int index = trace->ops[i].index;
            int size = trace->ops[i].size;

            char *p = gc_malloc(size);

            trace->blocks[index] = p;
            break;
          }
        case FREE:
          {
	    int index = trace->ops[i].index;
	    trace->blocks[index] = NULL;
            break;
          }
        }
}


void speed (trace_t *ptrace, stats_t *sp)
{
    trace = ptrace;
    sp->execution_time = ftime(speed_funct, ftime_tolerance);

    printf("Timing complete\n");
    if (verbose) {
       printf("Running time: %10.3f sec (error tolerance: %.3f)\n", 
               sp->execution_time, ftime_tolerance);
    }
    printf("\n");
}

/* No Implementation at all! */
/*extern void test1(void);
extern void test2(void);
extern void test3(void);
extern void test4(void);
extern void test5(void);
extern void tree_test(void);

static test_funct tests[6] = {
  test1,
  test2,
  test3,
  test4,
  test5,
  tree_test
} ;*/

/* No Implementation at all! */
void pgm_test()
{
  /*int i;
  int init_mem;
  for(i=0; i < 6; i++) {
    mem_reinit(0);
    clear_ranges();
    gc_init();
    init_mem = mem_usage();

    stats->execution_time = ftime(tests[i], ftime_tolerance);
    stats->memory_used = mem_usage();
    stats->overall_utilization = (double)stats->max_total_size / (double)stats->memory_used;
    if(i==3 || i==5) {
      stats->sum_overall_utilization  += stats->overall_utilization;
    }
    stats->total_time  += stats->execution_time;

    printf("Test %d: measurements completed\n", i+1);
    if (verbose) {
      printf("Maximum total size: %ld\n", stats->max_total_size);
      printf("Final heap size: %ld\n", mem_usage());
      printf("Overall utilization: %6.3f%%\n", 100.0*stats->overall_utilization);
      printf("Running time: %10.3f sec (error tolerance: %.3f)\n", 
	     stats->execution_time, ftime_tolerance);
    }
    printf("\n");
  }*/
}

/*******
 * Main
 *******/

int main (int argc, char *argv[])
{
    char **fname;
    char c;
    char **tracefiles = default_tracefiles;
    int traces_only = 0;
    int pgms_only = 0;

    stats = (stats_t *)malloc(sizeof(stats_t));
    //bzero(stats, sizeof(stats_t));
	memset(stats, 0, sizeof(stats_t));

    while ((c = getopt(argc, argv, "f:hpsvqt:d")) != EOF)
        switch (c) {
        case 'f':
            ++(stats->num_tracefiles);
            tracefiles = realloc(tracefiles, (stats->num_tracefiles + 1)*sizeof(char *));
            tracefiles[stats->num_tracefiles-1] = strdup(optarg);
            tracefiles[stats->num_tracefiles] = NULL;
	    traces_only = 1;
            break;
        case 'd':
            dump = 1;
            break;
	case 'p':
	    pgms_only = 1;
	    break;
	case 's':
	    traces_only = 1;
	    break;
        case 't':
            ftime_tolerance = atof(optarg);
            break;
        case 'v':
            verbose = 1;
            break;
        case 'q':
            verbose = 0;
            break;
        case 'h':
	    usage();
            exit(0);
        default:
            fprintf(stderr, "Unknown option; use -h for help\n");
            exit(1);
        }
 /*   if (tracefiles == NULL) {
        tracefiles = default_tracefiles;
        stats->num_tracefiles = sizeof(default_tracefiles) / sizeof(char *) - 1;
    }*/

    /* Print team info */
    printf("Teamname: %s\n", team.team);
    printf("Member 1: %s\n", team.name1);
    printf("Email 1: %s\n", team.email1);
    if (*team.name2 || *team.email2) {
      printf("Member 2: %s\n", team.name2);
      printf("Email 2: %s\n", team.email2);
    }
    printf("\n");
    
    if (verbose)
      printf("Running in verbose mode (use -q for quiet mode)...\n\n");
    
    /* Initialize memlib */
	
    mem_init();

    /* initilize stats */
    stats->total_time = 0.0;
    stats->total_opt_time = 0.0;
    stats->sum_overall_utilization = 0.0;


    /* Perform tests */
    if(!pgms_only) {
      for (fname = tracefiles;  *fname;  fname++) {
        trace_t trace;
	
        read_trace(&trace, *fname);
        if (trace.num_ops == 0) {
	  fprintf(stderr, "Failed to load tracefile %s\n", *fname);
	  fprintf(stderr, "Will try next one...\n\n");
	  --(stats->num_tracefiles);
	  continue;
        }
        stats->sum_weights += trace.weight;
	
	stats->total_ops += (double) trace.num_ops;

	/* 
	   test student malloc
	*/
	fprintf(stderr,"---- Your Malloc ----------\n");
	//bzero(trace.blocks, 20000*sizeof(char *)); 
	memset(trace.blocks, 0, 20000*sizeof(char *));
	correctness(&trace);
	stats->sum_overall_utilization  += stats->overall_utilization;
	
	//bzero(trace.blocks, 20000*sizeof(char *)); 
	memset(trace.blocks, 0, 20000*sizeof(char *)); 
	speed(&trace, stats);
	stats->total_time  += stats->execution_time;
	
	if (dump) {
	  dump_start(dump_filename(*fname));
	  dump_write(&trace);
	  dump_stop();
	}	    
	
	free_trace(&trace);
      }
      printf("All tracefiles read!\n\n");
    }

    if (!traces_only) {
      pgm_test();
    }

    /* 
     *  compute/report student malloc stats
     */

    /* Calculate final statistics */
    if(traces_only) {
      stats->avg_utilization  = stats->sum_overall_utilization / (double) stats->num_tracefiles;
    } else if(pgms_only) {
      stats->avg_utilization  = stats->sum_overall_utilization / 2.0;
    } else {
		// Win32: Calculation Method Adaptive Modification.
      //stats->avg_utilization  = stats->sum_overall_utilization / ((double) stats->num_tracefiles + 2);
	  stats->avg_utilization  = stats->sum_overall_utilization / 6.5;
    }

    stats->p1 = SPACE_UTIL_METRIC_WEIGHT * stats->avg_utilization; 
	stats->p2 = ((double)(1.0 - SPACE_UTIL_METRIC_WEIGHT)) * 
      MIN(1.0, REF_TIME/stats->total_time);

    stats->perf_index = stats->p1 + stats->p2;
    
    /* convert to a percentage */
    stats->perf_index *= 100; 

    /* Print final statistics */
    printf("OVERALL STATISTICS (YOUR MALLOC):\n");
    printf("---------------------------------\n");
    printf("Number of trace files: %d\n", 
	   stats->num_tracefiles);
    printf("All correctness tests passed\n");
    printf("\n");
    printf("Average overall utilization: %6.3f%%\n", 
	   100 * stats->avg_utilization);
    printf("Time: %6.3f, total ops: %ld\n",
	   stats->total_time, (long) (stats->total_ops));

    printf(">Performance index: %7.3f\n", stats->perf_index);
    printf("[Note: Performance index is valid only if");
    printf(" you use the default trace set]\n");

    printf("goodbye!\n\n");

    exit(0);
}


void
usage(void) 
{
    
    fprintf(stderr, "Usage: malloc [-ffile] [-h] [-v|-q]");
    fprintf(stderr," [-ttolerance] [-d]\n");
    
    fprintf(stderr, "Options\n");
    fprintf(stderr, "\t -f fname: use fname as the trace file\n");
    fprintf(stderr, "\t -p: program tests only\n");
    fprintf(stderr, "\t -s: synthetic traces only (implied by -f)\n");
    fprintf(stderr, "\t -h: print this help message\n");
    fprintf(stderr, "\t -t tol: Specify an error tolerance for the time measurements (default:0.05)\n");
    fprintf(stderr,"\n\t -d: dump trace of your allocator (malloc/free/sbrk) to a file \n");
    fprintf(stderr, "\t -v: verbose mode\n");
    fprintf(stderr, "\t -q: quiet mode\n");
}
