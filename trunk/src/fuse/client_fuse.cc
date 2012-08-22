#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <errno.h>

#include "client.hh"
#include "client_communicator.hh"

#include "../common/metadata.hh"
#include "../common/garbagecollector.hh"

#include "../config/config.hh"

Client* client;

ConfigLayer* configLayer;

ClientCommunicator* communicator;

mutex fileInfoCacheMutex;
map <uint32_t, struct FileMetaData> _fileInfoCache;

thread garbageCollectionThread;
thread receiveThread;
thread sendThread;

void startGarbageCollectionThread() {
	GarbageCollector::getInstance().start();
}

void startSendThread() {
	client->getCommunicator()->sendMessage();
}

void startReceiveThread(Communicator* communicator) {
	// wait for message
	communicator->waitForMessage();

}

void* ncvfs_init(struct fuse_conn_info *conn)
{
	communicator->createServerSocket();

	// 1. Garbage Collection Thread
	//thread garbageCollectionThread(startGarbageCollectionThread);
	garbageCollectionThread = thread(startGarbageCollectionThread);

	// 2. Receive Thread
	receiveThread = thread(startReceiveThread, communicator);

	// 3. Send Thread
	sendThread = thread(startSendThread);

	communicator->setId(client->getClientId());
	communicator->setComponentType(CLIENT);
	communicator->connectAllComponents();
	return NULL;
}

static int ncvfs_getattr(const char *path, struct stat *stbuf)
{
	struct FileMetaData fileMetaData = communicator->getFileInfo(51000, 423);
	//if(strcmp(path, "/") != 0)
	//	return -ENOENT;

	stbuf->st_mode = S_IFREG | 0644;
	stbuf->st_nlink = 1;
	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
	//stbuf->st_size = (1ULL << 32); /* 4G */
	stbuf->st_size = fileMetaData._size;
	stbuf->st_blocks = 0;
	stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);

	return 0;
}

static int ncvfs_open(const char *path, struct fuse_file_info *fi)
{
	//(void) fi;

	//if(strcmp(path, "/") != 0)
	//	return -ENOENT;

	{
		lock_guard<mutex> lk(fileInfoCacheMutex);
		struct FileMetaData fileMetaData = communicator->getFileInfo(51000, 423);
		fi->fh = fileMetaData._id;
		_fileInfoCache[fileMetaData._id] = fileMetaData;
	}
	return 0;
}

static int ncvfs_read(const char *path, char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	(void) buf;
	(void) offset;
	(void) fi;

//	if(strcmp(path, "/") != 0)
//		return -ENOENT;

	struct FileMetaData fileMetaData;
	{
		fileMetaData = _fileInfoCache[fi->fh];
		//fileMetaData = communicator->getFileInfo(51000, fi->fh);
	}

	if((offset + size) > fileMetaData._size)
		return 0;
	
	ClientStorageModule* storageModule = client->getStorageModule();

	uint64_t objectSize = storageModule->getObjectSize(); 
	uint32_t startObjectNum = offset / objectSize;
	uint32_t endObjectNum = (offset + size) / objectSize;
	if(((offset + size) % objectSize) == 0)
		--endObjectNum;
	uint64_t byteWritten = 0;
	for(uint32_t i = startObjectNum; i <= endObjectNum; ++i){
		uint64_t objectId = fileMetaData._objectList[i];
		uint32_t componentId = fileMetaData._primaryList[i];
		uint32_t sockfd = communicator->getSockfdFromId(componentId);

		struct ObjectCache objectCache;
		objectCache = communicator->getObject(51000, sockfd, objectId); 
		uint64_t copySize = min(objectSize,size - byteWritten);
		copySize = min(copySize, (i+1) * objectSize - (offset + byteWritten));
		uint64_t objectOffset = (offset + byteWritten) - i * objectSize;
		memcpy(buf + byteWritten, objectCache.buf + objectOffset, copySize);
		byteWritten += copySize;
		//storageModule->closeObject(objectId);
	}
	//if (offset >= (1ULL << 32))
	//	return 0;

	return byteWritten;
}

static int ncvfs_write(const char *path, const char *buf, size_t size,
		      off_t offset, struct fuse_file_info *fi)
{
	(void) buf;
	(void) offset;
	(void) fi;

//	if(strcmp(path, "/") != 0)
//		return -ENOENT;

	return size;
}

void ncvfs_destroy(void* userdata)
{
	//garbageCollectionThread.join();
	//receiveThread.join();
	//sendThread.join();
}

struct ncvfs_fuse_operations: fuse_operations
{
	ncvfs_fuse_operations ()
	{
		init	= ncvfs_init;
		getattr = ncvfs_getattr;
		open	= ncvfs_open;
		read	= ncvfs_read;
		write	= ncvfs_write;
		destroy = ncvfs_destroy;
	}
};

static struct ncvfs_fuse_operations ncvfs_oper;

int main(int argc, char *argv[])
{
	configLayer = new ConfigLayer("clientconfig.xml");
	client = new Client();
	communicator = client->getCommunicator();
	return fuse_main(argc, argv, &ncvfs_oper, NULL);
}
