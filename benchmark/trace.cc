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

typedef chrono::high_resolution_clock Clock;
typedef chrono::milliseconds milliseconds;

typedef struct {
    LL offset;
    LL length;
    bool isWrite;
} TraceInput;

// Settings

const int SEGMENT_SIZE = 10 * 1024 * 1024;
const int WORKER_NUM = 10;
const char* TRACE_FILE = "/tmp/a.csv";
const char* TARGET_FILE = "/home/qding/Workspace/ncvfs/trunk/mountdir/testfile";


LL globalSize[2];
double globalTime[2];
int doneWorker;
mutex globalMutex;

vector <TraceInput> traceInput;
mutex inputMutex;
int inputIndex;


void readTrace() {
    FILE* fp = fopen(TRACE_FILE, "r");
    char buf[1000];
    while (fgets(buf, 1000, fp) != NULL) {
        char *p = strtok(buf, ",");
        int idx = 0;
        TraceInput input;
        while (p) {
            if (idx == 3) {
                if (p[0] == 'R')  input.isWrite = 0;
                else input.isWrite = 1;
            }
            if (idx == 4) {
                input.offset = atoi(p);
            }
            if (idx == 5) {
                input.length = atoi(p);
            }
            idx++;
            p = strtok(NULL, ",");
        }
        traceInput.push_back(input);
    }
    fclose(fp);
}

void work() {
    char* buf = NULL;
    int bufSize = 0;
    LL workSize[2];
    double workTime[2];
    workSize[0] = workSize[1] = 0;
    workTime[0] = workSize[1] = 0;

    FILE* fp = fopen(TARGET_FILE, "r+");

    while (1) {
        inputMutex.lock();
        int idx = inputIndex++;
        inputMutex.unlock();
        if (idx < traceInput.size()) {
            TraceInput& input = traceInput[idx];
            if (idx % 500 == 0) {
                cout << "Working on " << idx << endl;
            }

            if (input.length > bufSize) {
                buf = (char*)realloc(buf, input.length);
                bufSize = input.length;
            }

            if (input.isWrite) {
                memset(buf, rand()%128, input.length);
            }

            Clock::time_point t0 = Clock::now();
            // do read write now
            if (input.isWrite) {
                //pwrite(fileno(fp), buf, input.length % SEGMENT_SIZE, 0);
                fwrite(buf, input.length, 1, fp);
            } else {
                //pread(fileno(fp), buf, input.length % SEGMENT_SIZE, 0);
                fread(buf, input.length, 1, fp);
            }
            Clock::time_point t1 = Clock::now();
            milliseconds ms = chrono::duration_cast < milliseconds > (t1 - t0);

            workSize[input.isWrite] += input.length;
            workTime[input.isWrite] += ms.count() / 1000.0;
        } else {
            break;
        }
    }
    fclose(fp);
    globalMutex.lock();
    globalSize[0] += workSize[0];
    globalSize[1] += workSize[1];
    globalTime[0] += workTime[0];
    globalTime[1] += workTime[1];
    ++doneWorker;
    globalMutex.unlock();
}

int main() 
{
    srand(time(NULL));
    doneWorker = 0;
    inputIndex = 0;
    memset(globalSize, 0, sizeof(globalSize));
    memset(globalTime, 0, sizeof(globalTime));
    readTrace();

    for (int i = 0; i < WORKER_NUM; i++) {
        thread (work).detach();
    }
    
    while (doneWorker != WORKER_NUM) {
        this_thread::sleep_for (chrono::seconds(2));
    }

    cout << "Total Read  : " << globalSize[0] / 1024.0 / 1024 << "MB, Time " << globalTime[0] << "s" << endl;
    cout << "Total Write : " << globalSize[1] / 1024.0 / 1024 << "MB, Time " << globalTime[1] << "s" << endl;
    
    return 0;
}
