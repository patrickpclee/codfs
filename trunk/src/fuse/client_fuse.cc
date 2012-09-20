#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <errno.h>
#include <unordered_map>

#include "client.hh"
#include "client_communicator.hh"
//#include "filedatamodule.hh"
#include "filedatacache.hh"

#include "../common/metadata.hh"
#include "../common/garbagecollector.hh"
#include "../common/debug.hh"

#include "../coding/raid1coding.hh"

#include "../config/config.hh"

Client* client;

ConfigLayer* configLayer;

ClientCommunicator* _clientCommunicator;

CodingScheme codingScheme = RAID1_CODING;
string codingSetting = Raid1Coding::generateSetting(1);

uint32_t _clientId = 51000;

mutex fileInfoCacheMutex;
mutex _objectProcessingMutex;

unordered_map<uint32_t, FileDataCache*> _fileDataCache;
unordered_map<uint64_t, unique_lock<mutex> > _objectProcessing;
unordered_map<uint32_t, struct FileMetaData> _fileInfoCache;
unordered_map<string, uint32_t> _fileIdCache;

thread garbageCollectionThread;
thread receiveThread;
thread sendThread;

struct FileMetaData getAndCacheFileInfo(string filePath) {
	struct FileMetaData fileMetaData;
	uint32_t fileId;
	lock_guard<mutex> lk(fileInfoCacheMutex);
	if (_fileIdCache.count(filePath) == 0) {
		fileMetaData = _clientCommunicator->getFileInfo(_clientId, filePath);
		_fileIdCache[filePath] = fileMetaData._id;
		_fileInfoCache[fileMetaData._id] = fileMetaData;
		fileId = fileMetaData._id;
	} else {
		fileId = _fileIdCache[filePath];
		debug("File Info Cache Found %s [%" PRIu32 "]\n",
				filePath.c_str(), fileId);
	}
	return _fileInfoCache[fileId];
}

struct FileMetaData getAndCacheFileInfo(uint32_t fileId) {
	struct FileMetaData fileMetaData;
	lock_guard<mutex> lk(fileInfoCacheMutex);
	if (_fileInfoCache.count(fileId) == 0) {
		fileMetaData = _clientCommunicator->getFileInfo(_clientId, fileId);
		_fileIdCache[fileMetaData._path] = fileId;
		_fileInfoCache[fileId] = fileMetaData;
	}
	debug("File Info Cache Found [%" PRIu32 "]\n", fileId);
	return _fileInfoCache[fileId];

}

void startGarbageCollectionThread() {
	GarbageCollector::getInstance().start();
}

void startSendThread() {
	client->getCommunicator()->sendMessage();
}

void startReceiveThread(Communicator* _clientCommunicator) {
	// wait for message
	_clientCommunicator->waitForMessage();

}

void* ncvfs_init(struct fuse_conn_info *conn) {
	_clientCommunicator->createServerSocket();

	// 1. Garbage Collection Thread
	garbageCollectionThread = thread(startGarbageCollectionThread);

	// 2. Receive Thread
	receiveThread = thread(startReceiveThread, _clientCommunicator);

	// 3. Send Thread
	sendThread = thread(startSendThread);

	_clientCommunicator->setId(client->getClientId());
	_clientCommunicator->setComponentType(CLIENT);

	//_clientCommunicator->connectAllComponents();
	_clientCommunicator->connectToMds();
	_clientCommunicator->connectToMonitor();
	_clientCommunicator->getOsdListAndConnect();
	return NULL;
}

static int ncvfs_getattr(const char *path, struct stat *stbuf) {

	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		return 0;
	}
	struct FileMetaData fileMetaData = getAndCacheFileInfo(path);
	if (fileMetaData._id == 0) {
		_fileIdCache.erase(path);
		_fileInfoCache.erase(fileMetaData._id);
		return -ENOENT;
	}

	stbuf->st_mode = S_IFREG | 0644;
	stbuf->st_nlink = 1;
	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
	stbuf->st_size = fileMetaData._size;
	stbuf->st_blocks = 0;
	stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);

	return 0;
}

static int ncvfs_open(const char *path, struct fuse_file_info *fi) {
	struct FileMetaData fileMetaData = getAndCacheFileInfo(path);
	fi->fh = fileMetaData._id;
	return 0;
}

static int ncvfs_create(const char * path, mode_t mode,
		struct fuse_file_info *fi) {
	uint32_t objectCount = configLayer->getConfigInt(
			"Fuse>PreallocateObjectNumber");
	uint32_t objectSize = configLayer->getConfigInt("Storage>ObjectSize")
			* 1024;
	struct FileMetaData fileMetaData = _clientCommunicator->uploadFile(
			_clientId, path, 0, objectCount, codingScheme, codingSetting);
	_fileIdCache[path] = fileMetaData._id;
	_fileInfoCache[fileMetaData._id] = fileMetaData;
	_fileDataCache[fileMetaData._id] = new FileDataCache(fileMetaData,
			objectSize);
	fi->fh = fileMetaData._id;
	return 0;
}

static int ncvfs_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	struct FileMetaData fileMetaData = getAndCacheFileInfo(fi->fh);

	if ((offset + size) > fileMetaData._size)
		return 0;

	ClientStorageModule* storageModule = client->getStorageModule();

	uint64_t objectSize = storageModule->getObjectSize();
	uint32_t startObjectNum = offset / objectSize;
	uint32_t endObjectNum = (offset + size) / objectSize;
	if (((offset + size) % objectSize) == 0)
		--endObjectNum;
	uint64_t byteWritten = 0;
	for (uint32_t i = startObjectNum; i <= endObjectNum; ++i) {
		uint64_t objectId = fileMetaData._objectList[i];
		uint32_t componentId = fileMetaData._primaryList[i];
		uint32_t sockfd = _clientCommunicator->getSockfdFromId(componentId);

		struct ObjectTransferCache objectCache;
		{
			lock_guard<mutex> lk(_objectProcessingMutex);
			objectCache = client->getObject(_clientId, sockfd, objectId);
		}
		uint64_t copySize = min(objectSize, size - byteWritten);
		copySize = min(copySize, (i + 1) * objectSize - (offset + byteWritten));
		uint64_t objectOffset = (offset + byteWritten) - i * objectSize;
		memcpy(buf + byteWritten, objectCache.buf + objectOffset, copySize);
		byteWritten += copySize;
		//storageModule->closeObject(objectId);
	}

	return byteWritten;
}

static int ncvfs_write(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi) {
	(void) buf;
	(void) offset;
	(void) fi;

//	if(strcmp(path, "/") != 0)
//		return -ENOENT;

	_fileDataCache[fi->fh]->write(buf, size, offset);

	return size;
}

static int ncvfs_release(const char* path, struct fuse_file_info *fi) {
	debug("Release %s [%" PRIu32 "]\n", path, (uint32_t)fi->fh);
	delete _fileDataCache[fi->fh];
	_fileDataCache.erase(fi->fh);
	_fileInfoCache.erase(fi->fh);
	_fileIdCache.erase(path);
	return 0;
}

void ncvfs_destroy(void* userdata) {
	//garbageCollectionThread.join();
	//receiveThread.join();
	//sendThread.join();
	return;
}

int ncvfs_chmod(const char *path, mode_t mode) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_chown(const char *path, uid_t uid, gid_t gid) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_utime(const char *path, struct utimbuf *ubuf) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_readlink(const char *path, char *link, size_t size) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_mknod(const char *path, mode_t mode, dev_t dev) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_mkdir(const char *path, mode_t mode) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_unlink(const char *path) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_rmdir(const char *path) {
	debug_cyan ("%s\n", "not implemented");
	return 0;

}

int ncvfs_symlink(const char *path, const char *link) {
	debug_cyan ("%s\n", "not implemented");
	return 0;

}

int ncvfs_rename(const char *path, const char *newpath) {
	debug_cyan ("%s\n", "not implemented");
	return 0;

}

int ncvfs_link(const char *path, const char *newpath) {
	debug_cyan ("%s\n", "not implemented");
	return 0;

}

int ncvfs_truncate(const char *path, off_t newsize) {
	debug_cyan ("%s\n", "not implemented");
	return 0;

}

int ncvfs_statfs(const char *path, struct statvfs *statv) {
	debug_cyan ("%s\n", "not implemented");
	return 0;

}

int ncvfs_flush(const char *path, struct fuse_file_info *fi) {
	debug_cyan ("%s\n", "not implemented");
	return 0;

}

int ncvfs_fsync(const char *path, int datasync, struct fuse_file_info *fi) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_setxattr(const char *path, const char *name, const char *value,
		size_t size, int flags) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_getxattr(const char *path, const char *name, char *value,
		size_t size) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_listxattr(const char *path, char *list, size_t size) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_removexattr(const char *path, const char *name) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_opendir(const char *path, struct fuse_file_info *fi) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_releasedir(const char *path, struct fuse_file_info *fi) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_access(const char *path, int mask) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

int ncvfs_fgetattr(const char *path, struct stat *statbuf,
		struct fuse_file_info *fi) {
	debug_cyan ("%s\n", "not implemented");
	return 0;
}

struct ncvfs_fuse_operations: fuse_operations {
	ncvfs_fuse_operations() {
		getattr = ncvfs_getattr;
		readlink = ncvfs_readlink;
		getdir = NULL;
		mknod = ncvfs_mknod;
		mkdir = ncvfs_mkdir;
		unlink = ncvfs_unlink;
		rmdir = ncvfs_rmdir;
		symlink = ncvfs_symlink;
		rename = ncvfs_rename;
		link = ncvfs_link;
		chmod = ncvfs_chmod;
		chown = ncvfs_chown;
		truncate = ncvfs_truncate;
		utime = ncvfs_utime;
		open = ncvfs_open;
		read = ncvfs_read;
		write = ncvfs_write;
		statfs = ncvfs_statfs;
		flush = ncvfs_flush;
		release = ncvfs_release;
		fsync = ncvfs_fsync;
		setxattr = ncvfs_setxattr;
		getxattr = ncvfs_getxattr;
		listxattr = ncvfs_listxattr;
		removexattr = ncvfs_removexattr;
		opendir = ncvfs_opendir;
		readdir = ncvfs_readdir;
		releasedir = ncvfs_releasedir;
		fsyncdir = ncvfs_fsyncdir;
		init = ncvfs_init;
		destroy = ncvfs_destroy;
		access = ncvfs_access;
		create = ncvfs_create;
		ftruncate = ncvfs_ftruncate;
		fgetattr = ncvfs_fgetattr;
	}
};

static struct ncvfs_fuse_operations ncvfs_oper;

int main(int argc, char *argv[]) {
	configLayer = new ConfigLayer("clientconfig.xml");
	client = new Client(_clientId);
	//fileDataModule = new FileDataModule();
	_clientCommunicator = client->getCommunicator();
	return fuse_main(argc, argv, &ncvfs_oper, NULL);
}
