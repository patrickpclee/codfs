#ifndef __DEBUG_HH__
#define __DEBUG_HH__

#define DEBUG 1

// for PRIu64, PRIu32, etc
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define debug(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "[%s] %s:%d:%s(): " fmt,  getTime().c_str(), __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

void printhex(char* buf, int n);
std::string getTime();
std::string formatSize(uint64_t size);

#endif
