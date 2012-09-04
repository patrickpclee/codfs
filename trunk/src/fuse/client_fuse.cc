#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <errno.h>
#include <unordered_map>

#include "client.hh"
#include "client_communicator.hh"
#include "filedatamodule.hh"
#include "filedatacache.hh"

#include "../common/metadata.hh"
#include "../common/garbagecollector.hh"

#include "../config/config.hh"

Client* client;

ConfigLayer* configLayer;

ClientCommunicator* communicator;

FileDataModule* fileDataModule;

uint32_t clientId = 51000;

mutex fileInfoCacheMutex;
mutex _objectProcessingMutex;
unordered_map <uint64_t, unique_lock<mutex> > _objectProcessing;
map <uint32_t, struct FileMetaData> _fileInfoCache;
unordered_map <string, uint32_t> _fileIdCache;

thread garbageCollectionThread;
thread receiveThread;
thread sendThread;

struct FileMetaData getAndCacheFileInfo (string filePath)
{
	struct FileMetaData fileMetaData;
	uint32_t fileId;
	lock_guard<mutex> lk(fileInfoCacheMutex);
	if(_fileIdCache.count(filePath) == 0) {
		fileMetaData = communicator->getFileInfo(clientId, filePath);
		_fileIdCache[filePath] = fileMetaData._id;
		_fileInfoCache[fileMetaData._id] = fileMetaData;
		fileId = fileMetaData._id;
	}
	else {
		fileId = _fileIdCache[filePath];
	}
	return _fileInfoCache[fileId];
}

struct FileMetaData getAndCacheFileInfo (uint32_t fileId)
{
	struct FileMetaData fileMetaData;
	lock_guard<mutex> lk(fileInfoCacheMutex);
	if(_fileInfoCache.count(fileId) == 0) {
		fileMetaData = communicator->getFileInfo(clientId, fileId);
		_fileIdCache[fileMetaData._path] = fileId;
		_fileInfoCache[fileId] = fileMetaData;
	}
	return _fileInfoCache[fileId];

}

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

	if(strcmp(path,"/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		return 0;
	}
	struct FileMetaData fileMetaData = getAndCacheFileInfo(path);

	stbuf->st_mode = S_IFREG | 0644;
	stbuf->st_nlink = 1;
	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
	stbuf->st_size = fileMetaData._size;
	stbuf->st_blocks = 0;
	stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);

	return 0;
}

static int ncvfs_open(const char *path, struct fuse_file_info *fi)
{
	struct FileMetaData fileMetaData = getAndCacheFileInfo(path);
	fi->fh = fileMetaData._id;
	return 0;
}

static int ncvfs_create(const char *, mode_t, struct fuse_file_info *)
{
	uint32_t objectCount = configLayer->getConfigInt("Fuse>PreallocateObjectNumber");
	//fileDataModule->createFileDataCache(		
}

static int ncvfs_read(const char *path, char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	struct FileMetaData fileMetaData = getAndCacheFileInfo(fi->fh);

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

		struct ObjectTransferCache objectCache;
		{
			lock_guard<mutex> lk(_objectProcessingMutex);
			objectCache = communicator->getObject(clientId, sockfd, objectId); 
		}
		uint64_t copySize = min(objectSize,size - byteWritten);
		copySize = min(copySize, (i+1) * objectSize - (offset + byteWritten));
		uint64_t objectOffset = (offset + byteWritten) - i * objectSize;
		memcpy(buf + byteWritten, objectCache.buf + objectOffset, copySize);
		byteWritten += copySize;
		//storageModule->closeObject(objectId);
	}

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
		create	= ncvfs_create;
		destroy = ncvfs_destroy;
	}
};

static struct ncvfs_fuse_operations ncvfs_oper;

int main(int argc, char *argv[])
{
	configLayer = new ConfigLayer("clientconfig.xml");
	client = new Client();
	fileDataModule = new FileDataModule();
	communicator = client->getCommunicator();
	return fuse_main(argc, argv, &ncvfs_oper, NULL);
}
