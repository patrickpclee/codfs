#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <unistd.h>
#include <algorithm>
#include <iostream>

using namespace std; 

typedef long long LL;

typedef chrono::system_clock Clock;

int IOdone = 0;


void work() {
    while (1) {
        printf ("%d\n", IOdone);
        Clock::time_point t0 = Clock::now();
        time_t tt;
        tt = Clock::to_time_t ( t0 );
        cout << "[" << ctime(&tt) << "] IO done = " << IOdone << endl;
        this_thread::sleep_for (chrono::seconds(1));
    }
}


int main() 
{
    srand(time(NULL));
    Clock::time_point t0 = Clock::now();
    time_t tt;
    tt = Clock::to_time_t ( t0 );
    cout << "[" << ctime(&tt) << "] IO Start " << endl;
 
    thread (work).detach();

    long long offset;
    long long length;
    char filepath[1000];
    char *buf=NULL;
    FILE *fp=NULL;

    while (scanf("%s %lld %lld", filepath, &offset, &length) != EOF) {
        if (fp == NULL) fp = fopen(filepath, "rb+");
        if (buf == NULL) buf = (char*)malloc(length);
        memset(buf, rand()%255, length);
        pwrite(fileno(fp), buf, length, offset);
        IOdone++;
    }
    free(buf);
    fclose(fp);
    
    t0 = Clock::now();
    tt = Clock::to_time_t ( t0 );
    cout << "[" << ctime(&tt) << "] IO Finish = " << IOdone << endl;
    return 0;
}
