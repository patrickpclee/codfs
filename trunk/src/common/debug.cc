#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "debug.hh"

using namespace std;

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
	sprintf (time_string, "%s.%03ld\n", time_string, milliseconds);
	return time_string;
}
