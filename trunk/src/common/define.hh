#ifndef __DEFINE_HH__
#define __DEFINE_HH__

#include <vector>
#include <stdint.h>

// coding
typedef std::pair<uint32_t, uint32_t> offset_length_t;
typedef std::pair<uint32_t, std::vector<offset_length_t> > symbol_list_t;
typedef std::vector<symbol_list_t> block_list_t ;

#define USLEEP_DURATION 10000

// osd.cc, client.cc, filedatacache.cc, benchmark.cc
//#define USE_CHECKSUM

// osd/storagemodule.cc
#define MAX_OPEN_FILES 100

// benchmark/benchmark.cc
// client/client.cc
#define PARALLEL_TRANSFER

// common/debug.hh
#ifndef DEBUG
#define DEBUG 1
#endif

// common/memorypool.hh
//#define USE_APR_MEMORY_POOL
//#define USE_NEDMALLOC

// communicator/communicator.cc
#define USE_THREAD_POOL
#define SERIALIZE_DATA_QUEUE

// communicator/communicator.hh
#define USE_LOWLOCK_QUEUE
#define USE_MULTIPLE_QUEUE

// config/config.hh
#define DEFAULT_CONFIG_PATH	"config.xml"
#define DEFAUTT_COMMON_CONFIG "common.xml"
#define XML_ROOT_NODE "NcvfsConfig"

// datastructure/lowlockqueue.hh
#define CACHE_LINE_SIZE 64

// fuse/client_fuse.cc
#define FUSE_USE_VERSION 26
#define FUSE_READ_AHEAD

// osd/osd.cc
#define INF (1<<29)
#define DISK_PATH "/"
#define RECOVERY_THREADS 10
#define MAX_NUM_PROCESSING_SEGMENT 4

// osd/storagemodule.cc
#define USE_SEGMENT_CACHE
#define USE_IO_THREADS
#define IO_THREADS 2
#define IO_POLL_INTERVAL 10000

// protocol/message.hh
#define USE_MESSAGE_MEMORY_POOL

// storage/mongodb.hh
//#define COLLECTION "ncvfs"

// monitor/selectionmodule.cc
//#define RR_DISTRIBUTE

// Receive Optimization
#define RECV_BUF_PER_SOCKET 10485760
#define USE_PARSING_THREADS
#define PARSING_THREADS 20


// Trigger Recovery or not
//#define TRIGGER_RECOVERY
#endif
