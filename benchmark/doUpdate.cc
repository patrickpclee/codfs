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

typedef struct {
    LL offset;
    LL length;
    string file;
} TraceInput;

// Settings

const int WORKER_NUM = 1;
string TARGET_FILE;
vector <TraceInput> traceInput;
mutex inputMutex;
int inputIndex;
atomic<int> done;

void readTrace() {
    char filename[1000];
    LL offset, length;
    while (scanf("%s %lld %lld\n", filename, &offset, &length) != EOF) {
        TraceInput input;
        input.offset = offset;
        input.length = length;
        input.file = string(filename);
        traceInput.push_back(input);
    }
    TARGET_FILE = traceInput[0].file;
}

void work() {
    char* buf = NULL;
    LL bufSize = 0;

    while (1) {
        inputMutex.lock();
        int idx = inputIndex++;
        inputMutex.unlock();
        if (idx < traceInput.size()) {
            TraceInput& input = traceInput[idx];
            if (input.length > bufSize) {
                buf = (char*)realloc(buf, input.length);
                bufSize = input.length;
            }
            FILE* fp = fopen(TARGET_FILE.c_str(), "rb+");
            memset(buf, rand()%128, input.length);
            pwrite(fileno(fp), buf, input.length, input.offset);
            fclose(fp);
        } else {
            break;
        }
    }
    free(buf);
    done++;
}

int main() 
{
    done = 0;
    Clock::time_point now = Clock::now();
    time_t now_t = Clock::to_time_t(now);
    cout << "UPDATE START " << inputIndex << " at " << ctime(&now_t);

    srand(time(NULL));
    inputIndex = 0;
    readTrace();

    for (int i = 0; i < WORKER_NUM; i++) {
        thread (work).detach();
    }
    
    while (done.load() != WORKER_NUM) {
        now = Clock::now();
        now_t = Clock::to_time_t(now);
        cout << "IO DONE " << inputIndex << " at " << ctime(&now_t);
        this_thread::sleep_for (chrono::seconds(1));
    }

    now = Clock::now();
    now_t = Clock::to_time_t(now);
    cout << "UPDATE END " << inputIndex << " at " << ctime(&now_t);
    return 0;
}
