#include "../common/define.hh"

#include <fuse.h>
#include <errno.h>
#include <stdexcept>      // std::out_of_range
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/stat.h>

#include "client.hh"
#include "client_communicator.hh"

#include "fuselogger.hh"
#include "filemetadatacache.hh"

#include "../common/metadata.hh"
#include "../common/garbagecollector.hh"
#include "../common/debug.hh"

#include "../coding/raid1coding.hh"
#include "../coding/raid5coding.hh"
#include "../coding/embrcoding.hh"

#include "../config/config.hh"

char* _cwdpath;
string _cwd;
string _fuseFolder = "fusedir";

Client* client;
ConfigLayer* configLayer;
uint32_t _clientId = 51000;
ClientCommunicator* _clientCommunicator;
FuseLogger* _fuseLogger;
FileMetaDataCache* _fileMetaDataCache;

thread garbageCollectionThread;
thread receiveThread;
thread sendThread;

static uint32_t checkNameSpace(const char* path) {
	string fpath = _fuseFolder + string(path);
	FILE* fp = fopen(fpath.c_str(),"r");
	if(fp == NULL) {
		debug("File: %s Does not Exist\n",path);
		return 0;
	}

	uint32_t fileId;
	int ret = fscanf(fp,"%" PRIu32, &fileId);

	fclose(fp);
	if (ret < 0) {
		perror("fscanf()");
		exit(-1);
	}
	debug("File ID Cached %" PRIu32 "\n", fileId);
	return fileId;
}

static struct FileMetaData getAndCacheFileMetaData(uint32_t id) {
	struct FileMetaData fileMetaData;
	try {
		fileMetaData = _fileMetaDataCache->getMetaData(id);
	} catch (const std::out_of_range& oor) {
		debug("Meta Data of File %" PRIu32 " Not Cached\n",id);
		fileMetaData = _clientCommunicator->getFileInfo(_clientId, id);
		if(fileMetaData._fileType == NOTFOUND)
			return fileMetaData;
		_fileMetaDataCache->saveMetaData(fileMetaData);
	}
	return fileMetaData;
}

void startGarbageCollectionThread() {
	GarbageCollector::getInstance().start();
}

static int ncvfs_error(const char *str) {
	int ret = -errno;
	printf("    ERROR %s: %s\n", str, strerror(errno));
	return ret;
}

static void* ncvfs_init(struct fuse_conn_info *conn) {
	_cwd = string(_cwdpath) + "/";
	configLayer = new ConfigLayer((_cwd + "common.xml").c_str(),(_cwd + "clientconfig.xml").c_str());
	client = new Client(_clientId);
	_fuseLogger = new FuseLogger(_cwd + "fuse.log");	
	_fileMetaDataCache = new FileMetaDataCache();
	_clientCommunicator = client->getCommunicator();
	_clientCommunicator->createServerSocket();

	// 1. Garbage Collection Thread
	garbageCollectionThread = thread(startGarbageCollectionThread);

	// 2. Receive Thread
	receiveThread = thread(&Communicator::waitForMessage, _clientCommunicator);

	// 3. Send Thread
#ifdef USE_MULTIPLE_QUEUE
#else
	sendThread = thread(&Communicator::sendMessage, _clientCommunicator);
#endif

	_clientCommunicator->setId(_clientId);
	_clientCommunicator->setComponentType(CLIENT);

	//_clientCommunicator->connectAllComponents();
	_clientCommunicator->connectToMds();
	_clientCommunicator->connectToMonitor();
	_clientCommunicator->getOsdListAndConnect();
}

static int ncvfs_getattr(const char *path, struct stat *stbuf) {
	// Root Directory
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		return 0;
	}

	uint32_t fileId = checkNameSpace(path);
	if (fileId == 0) {
		debug("File %s Does Not Exist\n", path);
		return -ENOENT;
	// File ID Cache
	} else {
		struct FileMetaData fileMetaData;
		fileMetaData = getAndCacheFileMetaData(fileId);

		if(fileMetaData._fileType == NOTFOUND) {
			return -ENOENT;
		}

		string fpath = _fuseFolder + string(path);
		int retstat = lstat(fpath.c_str(), stbuf);
		if(retstat < 0) {
			perror("lstat()");
			return -ENOENT;
		}
		stbuf->st_size = fileMetaData._size;
	}

	return 0;
}

static int ncvfs_open(const char *path, struct fuse_file_info *fi) {
}

static int ncvfs_create(const char * path, mode_t mode, struct fuse_file_info *fi) {
	string fpath = _fuseFolder + string(path);
	int ret = creat(fpath.c_str(), mode);
	if (ret < 0){
		perror("create()");
		return ret;
	}

	uint32_t segmentCount = configLayer->getConfigInt("Fuse>PreallocateSegmentNumber");
	struct FileMetaData fileMetaData = _clientCommunicator->uploadFile(_clientId, path, 0, segmentCount);
	fileMetaData._fileType = NORMAL;
	_fileMetaDataCache->saveMetaData(fileMetaData);
	fi->fh = fileMetaData._id;

	FILE* fp = fopen(fpath.c_str(),"w");
	fprintf(fp,"%" PRIu32, fileMetaData._id);
	fclose(fp);
	return 0;
}

static int ncvfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
}

static int ncvfs_write(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi) {
}

static void ncvfs_destroy(void* userdata) {
}

static int ncvfs_chmod(const char *path, mode_t mode) {
}

static int ncvfs_chown(const char *path, uid_t uid, gid_t gid) {
}

static int ncvfs_utime(const char *path, struct utimbuf *ubuf) {
	return 0;
}

static int ncvfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi) {
}

static int ncvfs_mknod(const char *path, mode_t mode, dev_t dev) {
}

static int ncvfs_mkdir(const char *path, mode_t mode) {
}

static int ncvfs_unlink(const char *path) {
}

static int ncvfs_rmdir(const char *path) {
}

static int ncvfs_rename(const char *path, const char *newpath) {
}

static int ncvfs_truncate(const char *path, off_t newsize) {
}

static int ncvfs_flush(const char *path, struct fuse_file_info *fi) {
	return 0;
}

static int ncvfs_fsync(const char *path, int datasync, struct fuse_file_info *fi) {
}

static int ncvfs_opendir(const char *path, struct fuse_file_info *fi) {
}

static int ncvfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset) {
}

static int ncvfs_release(const char* path, struct fuse_file_info *fi) {
	debug("Release %s [%" PRIu32 "]\n", path, (uint32_t)fi->fh);

	string fpath = _fuseFolder + string(path);
	uint32_t fileId = (uint32_t)fi->fh;

	FILE* fp = fopen(fpath.c_str(),"w");
	fprintf(fp,"%" PRIu32, fileId);
	fclose(fp);

	_fileMetaDataCache->removeMetaData(fileId);
	return 0;
}

static int ncvfs_releasedir(const char *path, struct fuse_file_info *fi) {
}

static int ncvfs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi) {
}

static int ncvfs_access(const char *path, int mask) {
}

static int ncvfs_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi) {
}

static int ncvfs_fgetattr(const char *path, struct stat *statbuf,
		struct fuse_file_info *fi) {
}

struct ncvfs_fuse_operations: fuse_operations {
	ncvfs_fuse_operations() {
		init = ncvfs_init;
		destroy = ncvfs_destroy;
		getattr = ncvfs_getattr;
//		fgetattr = ncvfs_fgetattr;
		access = ncvfs_access;
//		readlink = ncvfs_readlink; // not required
		opendir = ncvfs_opendir;
		readdir = ncvfs_readdir;
		mknod = ncvfs_mknod; // not required
		mkdir = ncvfs_mkdir;
		unlink = ncvfs_unlink;
		rmdir = ncvfs_rmdir;
//		symlink = ncvfs_symlink; // not required and not implemented
		rename = ncvfs_rename;
//		link = ncvfs_link; // not required
		chmod = ncvfs_chmod;
		chown = ncvfs_chown; // not required
		truncate = ncvfs_truncate;
		ftruncate = ncvfs_ftruncate;
		utime = ncvfs_utime; // not required
		open = ncvfs_open;
		read = ncvfs_read;
		write = ncvfs_write;
//		statfs = ncvfs_statfs; // not required
		release = ncvfs_release;
		releasedir = ncvfs_releasedir;
		fsync = ncvfs_fsync; // not strictly required
		fsyncdir = ncvfs_fsyncdir; // not strictly required
		flush = ncvfs_flush; // not required
//		setxattr = ncvfs_setxattr; // not required
//		getxattr = ncvfs_getxattr; // not required
//		listxattr = ncvfs_listxattr; // not required
//		removexattr = ncvfs_removexattr; // not required
		create = ncvfs_create;

//		flag_nullpath_ok = 0;				// accept NULL path and use fi->fh
	}
};

static struct ncvfs_fuse_operations ncvfs_oper;

int main(int argc, char *argv[]) {
	_cwdpath = (char*)calloc(sizeof(PATH_MAX) * PATH_MAX,1);
	if(getcwd(_cwdpath, PATH_MAX) == NULL){
		perror("getcwd()");
		exit(-1);
	}
	debug("Current Directory = %s\n",_cwdpath);
	return fuse_main(argc, argv, &ncvfs_oper, _cwdpath);
}
