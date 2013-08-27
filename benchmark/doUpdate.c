#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[]) {
    FILE* fp = fopen(argv[1], "rb+");
    long long offset;
    long long length;
    char *buf;
    sscanf(argv[2], "%lld", &offset);
    sscanf(argv[3], "%lld", &length);
    srand(offset);
    buf = (char*)malloc(length);
    memset(buf, rand()%255, length);
    pwrite(fileno(fp), buf, length, offset);
    free(buf);
    fclose(fp);
}

