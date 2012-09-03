#ifndef __DEBUG_HH__
#define __DEBUG_HH__

#define DEBUG 1

// for PRIu64, PRIu32, etc
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

// for color
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#define debug(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "[%s] %s:%d:%s(): " fmt,  getTime().c_str(), __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#define debug_yellow(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, YELLOW "[%s] %s:%d:%s(): " fmt RESET,  getTime().c_str(), __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

void printhex(char* buf, int n);
std::string getTime();
std::string formatSize(uint64_t size);

#endif
