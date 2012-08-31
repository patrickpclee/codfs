#include <iostream>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string>
#include <stdlib.h>
#include <sstream>
#include "debug.hh"
using namespace std;

#define DIM(x) (sizeof(x)/sizeof(*(x)))

static const string sizes[] = { "EiB", "PiB", "TiB", "GiB", "MiB", "KiB", "B" };
static const uint64_t exbibytes = 1024ULL * 1024ULL * 1024ULL * 1024ULL
		* 1024ULL * 1024ULL;


char time_string[40];

void printhex(char* buf, int n) {
	int i;
	for (i = 0; i < n; i++) {
		if (i > 0)
			printf(":");
		printf("%02X", buf[i]);
	}
	printf("\n");

}

char* getTime() {
	struct timeval tv;
	struct tm* ptm;
	long milliseconds;

	/* Obtain the time of day, and convert it to a tm struct. */
	gettimeofday(&tv, NULL);
	ptm = localtime(&tv.tv_sec);
	/* Format the date and time, down to a single second. */
	strftime(time_string, sizeof(time_string), "%H:%M:%S", ptm);
	/* Compute milliseconds from microseconds. */
	milliseconds = tv.tv_usec / 1000;
	/* Print the formatted time, in seconds, followed by a decimal point
	 and the milliseconds. */
	sprintf(time_string, "%s.%03ld", time_string, milliseconds);
	return time_string;
}

string formatSize(uint64_t size) {
	string result;
	uint64_t multiplier = exbibytes;
	int i;

	for (i = 0; i < (int)DIM(sizes); i++, multiplier /= 1024) {
		if (size < multiplier) {
			continue;
		}
		if (size % multiplier == 0) {
			result = to_string(size / multiplier) + sizes[i];
//			sprintf(result, "%" PRIu64 " %s", size / multiplier, sizes[i]);
		}
		else {
			stringstream ss;
			ss.precision(2);
			ss << fixed << (float) size / multiplier << sizes[i];
			result = ss.str();
//			sprintf(result, "%.1f %s", (float) size / multiplier, sizes[i]);
		}
		return result;
	}
	result = "0";
	return result;
}
