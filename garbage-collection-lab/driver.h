/* magic numbers */
/* Win32: REF_TIME Adaptive Modification */
#define SPACE_UTIL_METRIC_WEIGHT 0.33
//#define REF_TIME 110.0
#define REF_TIME 0.035
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#include <stddef.h>

/* test data structures */
typedef struct range_t {
    struct range_t *next;
    char *lo, *hi;
} range_t;

typedef struct {
    enum { ALLOC, FREE } type;
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
