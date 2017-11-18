/* Function timer */

#ifdef ALPHA
# define ftime ftime_alpha
#else
# define ftime ftime_default
#endif

typedef void(*test_funct)(void);
double ftime(test_funct P, double E);

/* Compute clock frequency */
double freq();

