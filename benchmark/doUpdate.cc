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

#define FILENAME "/home/ncsgroup/shb118/ncvfs/trunk/mountdir/benchmarkfile"

typedef struct {
    LL offset;
    LL length;
    bool isWrite;
    LL fileId;
} TraceInput;

// Settings

const int WORKER_NUM = 10;
string TARGET_FILE;
vector <TraceInput> traceInput;
mutex inputMutex;
int inputIndex;
atomic<int> done;

void readTrace() {
    char filename[1000];
    char action[1000];
    LL offset, length;
    LL fileId;
    while (scanf("%[^,],%lld,%lld,%lld\n", action, &offset, &length, &fileId) != EOF) {
        TraceInput input;
        input.offset = offset;
        input.length = length;
        if (action[0] == 'W')  {
            input.isWrite = true;
        } else {
            input.isWrite = false;
        }
        input.fileId = fileId;
        traceInput.push_back(input);
    }
    TARGET_FILE = FILENAME;
}

void work() {
    char* buf = NULL;
    LL bufSize = 0;

    FILE* fp = fopen(TARGET_FILE.c_str(), "rb+");
    LL prevFileId = 0;
    while (1) {
        inputMutex.lock();
        int idx = inputIndex++;
        inputMutex.unlock();
        if (idx < traceInput.size()) {
            TraceInput& input = traceInput[idx];
            if (!input.isWrite) {
                continue;
            }
            if (input.length > bufSize) {
                buf = (char*)realloc(buf, input.length);
                bufSize = input.length;
            }
            memset(buf, rand()%128, input.length);
            pwrite(fileno(fp), buf, input.length, input.offset);

            if (prevFileId != input.fileId) {
                fflush(fp);
            }
            prevFileId = input.fileId;
        } else {
            break;
        }
    }
    free(buf);
    fclose(fp);
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
