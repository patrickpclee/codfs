#include <cstdio>
#include <cstring>

#define BENCHMARK_PATH "./benchmark_up_down.sh"

const int  fileSizeCount = 1;
const char fileSizeParam[][10] = {"2048M"};
const int  osdMaxCount = 5;
const int  segSizeCount = 5;
const char segSizeParam[][10] = {"8M", "16M", "32M", "64M", "128M"};
const int  codingCount = 9;
const char codingParam[][10] = {"0", "1", "2", "3", "4", "5", "6", "7", "8"};
const int  procCount = 1;
const char procParam[][10] = {"6 1"};

int main()
{
	puts("#!/bin/bash");
	printf ("rm ~/ncvfs_log/*\n");
	printf ("rm ~/execute_log\n");
	for (int p = 0; p < procCount; p++)
		for (int k = 0; k < segSizeCount; k++)
			for (int i = 0; i < codingCount; i++) 
				for (int j = 0; j < fileSizeCount; j++)
				{
					printf ("echo \"%s %s %s %d %s %s\"\n", BENCHMARK_PATH, fileSizeParam[j], segSizeParam[k], osdMaxCount, procParam[p], codingParam[i]);
					printf ("%s %s %s %d %s %s\n", BENCHMARK_PATH, fileSizeParam[j], segSizeParam[k], osdMaxCount, procParam[p], codingParam[i]);
					printf ("echo \"`date`: %s %s %d %s %s\" >> ~/execute_log\n", fileSizeParam[j], segSizeParam[k], osdMaxCount, procParam[p], codingParam[i]);
				}
	return 0;
}
