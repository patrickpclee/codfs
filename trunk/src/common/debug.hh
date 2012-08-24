#ifndef __DEBUG_HH__
#define __DEBUG_HH__

#define DEBUG 0

// for PRIu64, PRIu32, etc
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define debug(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

void printhex(char* buf, int n);

#endif
