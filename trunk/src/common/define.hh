#ifndef __DEFINE_HH__
#define __DEFINE_HH__

#include <vector>
#include <stdint.h>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

// coding
typedef std::pair<uint32_t, uint32_t> offset_length_t;
typedef std::pair<uint32_t, std::vector<offset_length_t> > symbol_list_t;
typedef std::vector<symbol_list_t> block_list_t ;
typedef std::pair<uint32_t, uint32_t> block_symbol_t;
typedef boost::shared_mutex RWMutex;
typedef boost::shared_lock<RWMutex> readLock;
typedef boost::unique_lock<RWMutex> writeLock;


#define USLEEP_DURATION 10000

// osd/storagemodule.cc
#define MAX_OPEN_FILES 100

// benchmark/benchmark.cc
#define RANDOM_SHUFFLE_SEGMENT_ORDER

// common/debug.hh
#ifndef DEBUG
#define DEBUG 1
#endif

// communicator/message.cc
#define NUM_THREADS_PER_MSG 10

// config/config.hh
#define DEFAULT_CONFIG_PATH	"config.xml"
#define DEFAUTT_COMMON_CONFIG "common.xml"
#define XML_ROOT_NODE "NcvfsConfig"

// datastructure/lowlockqueue.hh
#define CACHE_LINE_SIZE 64

// fuse/client_fuse.cc
#define FUSE_USE_VERSION 26
#define FUSE_READ_AHEAD
#define FUSE_PRECACHE_AHEAD

// osd/osd.cc
#define INF (1<<29)
#define DISK_PATH "/"
#define RECOVERY_THREADS 10
#define MAX_NUM_PROCESSING_SEGMENT 10

#define UPDATE_SCHEME 2 // FO,FL,PL,PLR
#if UPDATE_SCHEME == 3
#define RESERVE_SPACE_SIZE 5242880
#else
#define RESERVE_SPACE_SIZE 0
#endif

// osd/storagemodule.cc
#define HOTNESS_ALG TOP_HOTNESS_ALG
#define IO_THREADS 2
#define IO_POLL_INTERVAL 10000
#define USE_FSYNC

// monitor/selectionmodule.cc
//#define RR_DISTRIBUTE
#define RANDOM_CHOOSE_SECONDARY

// Receive Optimization
#define RECV_BUF_PER_SOCKET 10485760

// Trigger Recovery or not
//#define TRIGGER_RECOVERY
#define RECOVERY_DST "destinations.txt"

#define LATENCY_TEST
#endif
