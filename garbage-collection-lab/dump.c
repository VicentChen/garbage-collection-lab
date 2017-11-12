/* $Id$ */

/*
 *
 *  CS213 - Lab assignment 3
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include "dump.h"

FILE *dumpfile = NULL;

#define DUMP_SUFFIX  ".dump"

char *dump_filename (const char *trace_filename)
{
    static char buf[1025];
    char *p;

    // Isolate base filename
    p = strrchr(trace_filename, '/');
    p = (p == NULL) ? (char *)trace_filename : (p+1);
    strcpy(buf, p);

    // Replace suffix
    p = strchr(buf, '.');
    p = (p == NULL) ? (buf + strlen(buf)) : p;
    strcpy(p, DUMP_SUFFIX);

    return buf;
}

void dump_start (const char *filename)
{
    assert(dumpfile == NULL);
    if ((dumpfile = fopen(filename, "w")) == NULL) {
        fprintf(stderr, "Cannot create dumpfile %s\n", filename);
        exit(1);
    }
}

void dump_stop (void)
{
    assert(dumpfile != NULL);
    fclose(dumpfile);
    dumpfile = NULL;
}

int dump_printf (const char *format, ...)
{
    if (dumpfile != NULL) {
        va_list args;
        int retval;

        va_start(args, format);
        retval = vfprintf(dumpfile, format, args);
        va_end(args);

        return retval;
    }
    return 0;
}


