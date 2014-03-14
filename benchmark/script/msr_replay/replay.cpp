#include <cstdio>
#include <cstdlib>
#include <list>
#include <set>
#include <mutex>
#include <thread>
#include <fstream>
#include <string>
#include <ctime>
#include <climits>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <boost/lexical_cast.hpp>

#define SEG_SIZE 16777216		// 16M
#define BLOCK_SIZE 4194304		// 16M / 4 = 4M 
#define THREAD_NUM 1
#define TIME_INT 1
#define FLUSH_INT 5
//#define DEBUG

// progress monitoring
unsigned long long rectotal=0;
unsigned long long recproc=0;
unsigned long long recskip = 0;
struct timespec pstart;
bool reserve = false;

struct trace_rec {
	unsigned long long offset;
	unsigned long long len;
	char type;
	std::list<unsigned long long> blkid;
	bool empty;
};

std::list<std::thread> workers;
// "shared resources" used by getNext()
std::list<struct trace_rec> wholetrace;
std::set<unsigned long long> blkidpool;
std::mutex tracemtx;
int threadtotal = 0;
#ifdef DEBUG_THREAD
unsigned long long trequest[THREAD_NUM];
#endif 
// shared resources used by threads
FILE* output = NULL;
unsigned long long wlargest = 0;
unsigned long long rlargest = 0;
unsigned long long largest = 0;
unsigned long long smallest = ULLONG_MAX;
static unsigned long long curTime = 0;
struct timespec start;
struct timespec end;

// difference between 2 timestamps
// adopted from http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
struct timespec diff(const struct timespec start, const struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

double timeinms (const struct timespec rt) {
	return rt.tv_sec * 1000.0 + rt.tv_nsec / 1000000.0;
}

std::string getTime() {
	struct timeval tv;
	struct tm* ptm;
	long milliseconds;
	char time_string[40];

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
	return std::string(time_string);
}

void printProgress(int signo) {
	static unsigned long long prevproc = 0;
	static int flushcnt = 0;
	if (signo == SIGALRM) {
		curTime += TIME_INT;
		printf("%s [%lld] %lld (%lld)\n",getTime().c_str(), curTime,recproc,recproc-prevproc);
		prevproc = recproc;
	} else {
		printf("Record %lld of %lld\nReplay time: %.3lf ms\n",recproc,rectotal,timeinms(diff(start,end)));
	}
#ifdef DEBUG_THREAD
	for (int i=0; i < THREAD_NUM; ++i) {
		printf("THREAD[%d]: %ld\n",i,trequest[i]);
	}
#endif
	if (++flushcnt >= FLUSH_INT) {
		flushcnt = 0 ;
		fflush(output);
	}
}

int inserttrace(const unsigned long long* offset, const unsigned long long* len, 
		const char* type, std::list<unsigned long long> blkid) {
	struct trace_rec tmp;
	tmp.offset = *offset;
	tmp.len = *len;
	tmp.type = *type;
	tmp.blkid = blkid;
	wholetrace.push_back(tmp);
    return 0;
}

// return a list of blockId associate with the request
std::list<unsigned long long> getblockId(const unsigned long long *offset, const unsigned long long *len) {
	unsigned long long last = *offset + *len - 1;
	std::list<unsigned long long> blkid;
	if (*len == 0)  return blkid;
	for (unsigned long long i=*offset; i <= last;) {
		unsigned long long curblkid = i / BLOCK_SIZE;
		unsigned long long curblkoffset = curblkid * BLOCK_SIZE;
#ifdef DEBUG
		printf("list Push back\n");
#endif
		blkid.push_back(curblkid);
#ifdef DEBUG
		printf("list Push back end, %ld %ld\n",i,curblkoffset + BLOCK_SIZE);
#endif
		i = curblkoffset + BLOCK_SIZE;
	}

	return blkid;
}

int parseline(FILE* input, char* line, unsigned long long* offset, 
		unsigned long long* len, char* type) {
	char field[20];
	//int cnt = 1;
    if (fscanf(input, "%[^,],%llu,%llu\n", field, offset, len) != 3) {
        return 2;
    }
    *type=field[0];

    /*
	if (fgets(line, 4096, input) == 0) { return 2; }
#ifdef DEBUG
	printf("%s\n",line);
#endif
	field = strtok(line,",");
	while (field != NULL) {
		if (cnt == 1) {
			*type = field[0];
		} else if (cnt == 2) {
			*offset = boost::lexical_cast<unsigned long long>(std::string(field));
		} else if (cnt == 3) {
			*len = boost::lexical_cast<unsigned long long>(std::string(field));
		}
		++cnt;
		field = strtok(NULL,",");
	}
	if (cnt < 4) { return -1; }
    */
	return 0;
}

unsigned long long loadtrace(char* filename) {
	unsigned long long newoffset;
	int ret = 0;
	char type = 0;
	unsigned long long offset = 0;
	unsigned long long len = 0;
	char line[4096];
	std::list<unsigned long long> blkid;
	FILE* input = NULL;
	if ((input = fopen(filename,"r")) == NULL) {
		printf("Cannot open trace file %s \n",filename);
		exit(-1);
	}
#ifdef DEBUG_THREAD
	for (int i=0; i < THREAD_NUM; ++i) { trequest[i] = 0; }
#endif
	while (!feof(input)) {
		// get the line of trace from file
		ret = parseline(input,line,&offset,&len,&type);
#ifdef DEBUG
		printf("%ld %ld %c\n",offset,len,type);
#endif
		if (ret == -1) {
			printf("Warning: not enough parameters: %s\n",line);
			continue; 
		} else if (ret == 2) {
			// eof
			break;
		} else {
			// successful read
			//printf("offset: %llu len: %llu type: %c\n",offset,len,type);
		}
#ifdef WRITE_ONLY
		if (type == 'W') {
#endif
			++rectotal;
#ifdef WRITE_ONLY
		}
#endif
		// get the block ids
		blkid = getblockId(&offset,&len);
		// insert the line of trace to in-memory list
		inserttrace(&offset,&len,&type,blkid);
		// find the largest offset accessed
		newoffset = offset+len;
		if (newoffset > largest) {
			largest = newoffset;
		}
		// find the smallest offset accessed
		if (offset < smallest) {
			smallest = offset;
		}
		// find the largest request size of write and read respectively
		if (type == 'W' && wlargest < len) {
			wlargest = len;
		} else if (type == 'R' && rlargest < len) {
            rlargest = len;
        }
	}
	fclose(input);
	return largest;
}

// get next trace record to run
// @param prevrec the previous trace record the thread is holding
// -> the function should clear the blkid in the prevrec
// @return a new rec for the thread to work on
struct trace_rec getNext(struct trace_rec prevrec) {

	// empty rec for end of trace
	static trace_rec empty;
	trace_rec next;
	std::set<unsigned long long> tmpblkid;
	// clear the old blkid
	std::list<unsigned long long> blkid = prevrec.blkid;
	next.empty = false;
	empty.empty = true;
	{
		std::lock_guard<std::mutex> lk (tracemtx);
		//printf("init pool size: %ld - %ld \n",blkidpool.size(), blkid.size());
		for (std::list<unsigned long long>::iterator bid = blkid.begin(); bid != blkid.end(); ++bid) {
			//printf("release: %llu %llu (%llu)\n",prevrec.offset,prevrec.len,*bid);
			blkidpool.erase(*bid);
		}
		//printf("end pool size: %ld\n",blkidpool.size());
	}
	while (true) {
		{
			// lock the wholetrace list
			std::lock_guard<std::mutex> lg (tracemtx);
			// (1), thread can exit
			//std::lock_guard<std:lk (tracemtx);
			if (wholetrace.empty()) {
	#ifdef DEBUG
				printf("END OF TRACE\n");
	#endif
				break;
			}

			// iterator the wholetrace list for a record
			// handle cases:
			// 1. no trace left
			// 2. some trace left, first trace to go
			// 3. some trace left, not the first, but one of the remaining to go
			// 4. some trace left, but none of them to go
			std::list <trace_rec>::iterator i = wholetrace.begin();
			while (i != wholetrace.end()) {
				bool isGood = true;
				tmpblkid.clear();
				// (2),(3),(4)
				for (std::list<unsigned long long>::iterator bid = i->blkid.begin(); bid != i->blkid.end(); ++bid) {
					if (blkidpool.count(*bid) != 0) {
						// skip this trace for the time being
						isGood = false;
						break;
					}
					tmpblkid.insert(*bid);
				}

				if (!isGood) {
					++i;
					continue;
				}

				// (2),(3), the whole list of blkid does not exists in blkidpool
				// set the return trace record
				next.offset = i->offset;
				next.len = i->len;
				next.type = i-> type;
				next.blkid.assign(i->blkid.begin(),i->blkid.end());
				// add the blkid to reserve pool
				//printf("before insert: %lu + %lu (%lu)\n", blkidpool.size(),tmpblkid.size(),i->blkid.size());
				blkidpool.insert(tmpblkid.begin(),tmpblkid.end());
				//printf("after insert: %lu \n", blkidpool.size());
				// remove the trace record from list
				//printf("take: %llu %llu [%llu]\n",next.offset,next.len,*(next.blkid.begin()));
				wholetrace.erase(i++);
				return next;
			}
			// (4), sleep to wait some thread unlock some blkid
			//printf("WAIT FOR BLK ID RELEASE, blkpool: %ld\n",blkidpool.size());
		}
		sleep(1);
	}
	return empty;
}

// thread worker function
void replay(int id) {

	int outputfd = fileno(output);
	char* rbuf = NULL;
	char* wbuf = NULL;
	int roff = 0;
	unsigned long long wlen = wlargest;
	struct trace_rec myrec;
	signal(SIGUSR1,printProgress);
	srand(time(NULL));
	
	rbuf = (char*) malloc(sizeof(char)*(rlargest));
	if (rbuf == NULL) { 
		printf("Not enough space for allocating a read buf.\n");
		exit(-1);
	}

	wbuf = (char*) malloc(sizeof(char)*wlen);
	if (wbuf == NULL) { 
		printf("Not enough space for allocating a write buf.\n");
		exit(-1);
	}
	memset(wbuf,0,wlen);
	while (true) {
		// get a trace record to work on ... (blocking)
		myrec = getNext(myrec);
		if (myrec.empty) {
#ifdef DEBUG_THREAD
			// end of trace, exit peacefully
			printf("NO MORE RECORD, THREAD EXIT\n");
#endif
			return;
		}
		// process the record
        if (myrec.type == 'W') {
			// select a random part of write buffer to write the content
			roff = 0;
			//printf("WRITE %ld %ld \n",myrec.offset,myrec.len);
			if (reserve) {
				// reserve space need offset adjustment
				//usleep(1000);
				pwrite(outputfd,wbuf+roff,myrec.len,myrec.offset-smallest);
			} else {
				pwrite(outputfd,wbuf+roff,myrec.len,myrec.offset);
			}
			++recproc;
		} else if (myrec.type == 'R') {
			//printf("READ %ld %ld \n",myrec.offset,myrec.len);
#ifndef WRITE_ONLY
			pread(outputfd,rbuf,myrec.len,myrec.offset);
			++recproc;
#endif
		} else {
			++recskip;
		}
#ifdef DEBUG_THREAD
		trequest[id]++;
#endif
    }
	//free(rbuf);
}

int newdisk(unsigned long long size, char* outfile) {
	unsigned long long count = 0;
	char cmd [1024];
	// for reserve, create individual files for each client
	if (reserve) {
		// align to segment size
		smallest = smallest/SEG_SIZE * SEG_SIZE;
		size -= smallest;
	}
	if (size % SEG_SIZE != 0) {
		count = 1;
	}
	count += size / SEG_SIZE;
	// new a virtual raw disk file, fill with random data
	// dd zero for debug
	printf("start dd new disk\n");
	if (size == 0) {
		sprintf(cmd,"touch %s", outfile);
	} else {
		sprintf(cmd,"dd if=/dev/zero of=%s bs=%d count=%lld\n",outfile,SEG_SIZE,count);
	}
	// dd random for simulation
	//sprintf(cmd,"dd if=/dev/urandom of=./%s bs=%d count=%ld",outfile,SEG_SIZE,count);
#ifdef DEBUG
	printf("%s\n",cmd);
#endif
	if (system(cmd)) { return -1; }
	return 0;
}
//
int main (int argc, char ** argv) {
	unsigned long long disksize = 0L;

	if (argc < 3) {
		printf("Usage: %s <input file> <output file>\n",argv[0]);
		return 1;
	} else if (argc == 4) {
        //printf("Reserved mode\n");
		reserve = true;
	}
	signal(SIGALRM,printProgress);

	// load the traces
	disksize = loadtrace(argv[1]);
#ifndef CHECK_SIZE
	printf("Upper Disk Size: %lld Lower: %lld, Actual: %lld\n", disksize, smallest, disksize-smallest);
	printf("Total IO: %lld\n", rectotal);
#ifdef PROMPT_NEW_DISK
	//printf("Continue to create a virtual disk file [Y/N]? ");
	//ans = (char)getc(stdin);

	//if (ans == 'N' || ans == 'n') {
	//	printf("Disk allocation aborted\n");
	//	return 0;
	//} 

	if (newdisk(disksize,argv[2]) == -1) {
		printf("Cannot new a disk\n");
		return -1;
	}
#endif
#ifdef PROMPT_CONT
	printf("Continue to replay traces?");
	ans = (char)getc(stdin);
	if (ans == 'N' || ans == 'n') {
		printf("Trace replay aborted\n");
		return 0;
	}
#endif
	output = fopen(argv[2],"r+");
	if (output == NULL) {
		printf("Cannot open file: %s\n",argv[2]);
		return -1;
	} 
	// report statistics
	signal(SIGUSR1,printProgress);
	
	// setup the reporting timer
	struct itimerval newval;
	newval.it_interval.tv_sec = TIME_INT;
	newval.it_interval.tv_usec = 0;
	newval.it_value.tv_sec = TIME_INT;
	newval.it_value.tv_usec = 0;
	if (setitimer(ITIMER_REAL,&newval,NULL)) {
		perror("setitimer()\n");
	}
	clock_gettime(CLOCK_MONOTONIC,&start);
	// initiate threads
	for (int i=0; i < THREAD_NUM; ++i) {
		int a = i;
		workers.push_back(std::thread(replay,a));
	}
#ifdef DEBUG
	printf("Thread %d init.\n",THREAD_NUM);
#endif
	// end of program , join worker threads
	for (std::list<std::thread>::iterator w = workers.begin(); 
			w != workers.end(); ++w) {
		w->join();
	}
	clock_gettime(CLOCK_MONOTONIC,&end);
	// print the end time
	printProgress(0);
#else
	printf("%lld\n",disksize);
#endif
}
