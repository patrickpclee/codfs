#include "debug.hh"
#include "stdio.h"

void printhex(char* buf, int n) {
	int i;
	for (i = 0; i < n; i++) {
		if (i > 0)
			printf(":");
		printf("%02X", buf[i]);
	}
	printf("\n");

}

