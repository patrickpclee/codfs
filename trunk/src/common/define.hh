#ifndef __DEFINE_HH__
#define __DEFINE_HH__

// osd/storagemodule.cc
#define MAX_OPEN_FILES 100

// benchmark/benchmark.cc
// client/client.cc
#define PARALLEL_TRANSFER

// common/debug.hh
#define DEBUG 1

// common/memorypool.hh
//#define USE_APR_MEMORY_POOL
//#define USE_NEDMALLOC

// communicator/communicator.cc
#define USE_THREAD_POOL

// communicator/communicator.hh
#define USE_LOWLOCK_QUEUE

// config/config.hh
#define DEFAULT_CONFIG_PATH	"config.xml"
#define DEFAUTT_COMMON_CONFIG "common.xml"
#define XML_ROOT_NODE "NcvfsConfig"

// datastructure/lowlockqueue.hh
#define CACHE_LINE_SIZE 64

// fuse/client_fuse.cc
#define FUSE_USE_VERSION 26

// osd/osd.cc
#define INF (1<<29)
#define DISK_PATH "/"

// osd/storagemodule.cc
//#define USE_OBJECT_DISK_CACHE

// protocol/message.hh
#define USE_MESSAGE_MEMORY_POOL

// storage/mongodb.hh
//#define COLLECTION "ncvfs"

#endif
