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

#include <forward_list>		// std::forward_list

#include "client.hh"
#include "client_communicator.hh"

#include "filemetadatacache.hh"
#include "filedatacache.hh"

#include "../common/metadata.hh"
#include "../common/garbagecollector.hh"
#include "../common/convertor.hh"	//md5ToHex()
#include "../common/debug.hh"
#include "../common/define.hh"
#include "../config/config.hh"

char* _cwdpath;
string _cwd;
string _fuseFolder = "fusedir";

Client* client;
ConfigLayer* configLayer;
uint32_t _clientId;
ClientCommunicator* _clientCommunicator;
FileMetaDataCache* _fileMetaDataCache;
FileDataCache* _fileDataCache;

mutex _segmentMetaMutex;

uint32_t _segmentSize;
uint32_t _prefetchCount;

std::forward_list<struct SegmentMetaData> _segmentMetaDataList;
uint32_t _segmentMetaDataAllocateSize = 50;

thread garbageCollectionThread;
thread receiveThread;
thread sendThread;


/*
 * FileId Mutex for modify file meta data
 */
std::mutex _fileRWMutexMapMutex;
unordered_map <uint32_t, RWMutex*> _fileRWMutexMap;
static RWMutex* obtainFileRWMutex(uint32_t fileId) {
    // obtain rwmutex for this segment
    _fileRWMutexMapMutex.lock();
    RWMutex* rwmutex;
    if (_fileRWMutexMap.count(fileId) == 0) {
        rwmutex = new RWMutex();
        _fileRWMutexMap[fileId] = rwmutex; 
    } else {
        rwmutex = _fileRWMutexMap[fileId];
    }
    _fileRWMutexMapMutex.unlock();
    return rwmutex;
}

static void removeNameSpace(const char* path) {
	string fpath = _fuseFolder + string(path);
	unlink(fpath.c_str());
	return ;
}

/**
 * Get fileId given the filepath
 * fileId is saved simply as an integer inside the shadow file in _fuseFolder
 * @param path Filepath
 * @return fileId
 */

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
		debug("No File ID for %s", path);
		return 0;
		//perror("fscanf()");
		//exit(-1);
	}
	debug("File ID Cached %" PRIu32 "\n", fileId);
	return fileId;
}

static struct FileMetaData getAndCacheFileMetaData(uint32_t id) {

	struct FileMetaData fileMetaData;
	try {
		fileMetaData = _fileMetaDataCache->getMetaData(id);
		for (uint32_t primary : fileMetaData._primaryList) {
		    // if at least one primary is disconnected, request latest metadata from MDS
            if (_clientCommunicator->getSockfdFromId(primary) == (uint32_t)-1){
                fileMetaData = _clientCommunicator->getFileInfo(_clientId, id);
                _fileMetaDataCache->saveMetaData(fileMetaData);
                break;
            }
		}
	} catch (const std::out_of_range& oor) {
		debug("Meta Data of File %" PRIu32 " Not Cached\n",id);
		fileMetaData = _clientCommunicator->getFileInfo(_clientId, id);
		if(fileMetaData._fileType == NOTFOUND)
			return fileMetaData;
		_fileMetaDataCache->saveMetaData(fileMetaData);
	}
	return fileMetaData;
}

static struct SegmentMetaData allocateSegmentMetaData() {
	_segmentMetaMutex.lock();
	if(_segmentMetaDataList.empty()) {
		vector<struct SegmentMetaData> segmentMetaDataList = _clientCommunicator->getNewSegmentList(_clientId, _segmentMetaDataAllocateSize);
		_segmentMetaDataList.insert_after(_segmentMetaDataList.before_begin(), segmentMetaDataList.begin(), segmentMetaDataList.end());
	}
	struct SegmentMetaData _segmentMetaData = _segmentMetaDataList.front();
	_segmentMetaDataList.pop_front();
	_segmentMetaMutex.unlock();
	return _segmentMetaData;
}

void startGarbageCollectionThread() {
	GarbageCollector::getInstance().start();
}

/*
   static int ncvfs_error(const char *str) {
   int ret = -errno;
   printf("    ERROR %s: %s\n", str, strerror(errno));
   return ret;
   }
 */

static void* ncvfs_init(struct fuse_conn_info *conn) {
	_cwd = string(_cwdpath) + "/";
	configLayer = new ConfigLayer((_cwd + "common.xml").c_str(),(_cwd + "clientconfig.xml").c_str());

    _fileRWMutexMap.clear();
    _segmentSize = stringToByte(configLayer->getConfigString("Fuse>segmentSize"));
    _prefetchCount = configLayer->getConfigInt("Fuse>prefetchCount");
	client = new Client(_clientId);
	_fileMetaDataCache = new FileMetaDataCache();
	_clientCommunicator = client->getCommunicator();
	_clientCommunicator->createServerSocket();
	_fuseFolder = _cwd + _fuseFolder;

	_fileDataCache = new FileDataCache();

	// 1. Garbage Collection Thread
	garbageCollectionThread = thread(startGarbageCollectionThread);

	// 2. Receive Thread
	receiveThread = thread(&Communicator::waitForMessage, _clientCommunicator);

	_clientCommunicator->setId(_clientId);
	_clientCommunicator->setComponentType(CLIENT);

	//_clientCommunicator->connectAllComponents();
	_clientCommunicator->connectToMds();
	_clientCommunicator->connectToMonitor();
	_clientCommunicator->getOsdListAndConnect();
	return NULL;
}

static int ncvfs_getattr(const char *path, struct stat *stbuf) {
	// Root Directory
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		return 0;
	}

	string fpath = _fuseFolder + string(path);
	int retstat = lstat(fpath.c_str(), stbuf);

	if(retstat < 0) {
		perror("lstat()");
		return -ENOENT;
	}

	if(S_ISDIR(stbuf->st_mode)) {
		debug("%s is a Directory\n", path);
		return retstat;
	}

	uint32_t fileId = checkNameSpace(path);
	if (fileId == 0) {
		debug("File %s Does Not Exist\n", path);
		return -ENOENT;
		// File ID Cache
	} else {
		struct FileMetaData fileMetaData;
		fileMetaData = getAndCacheFileMetaData(fileId);

		//In Case the Record on MDS is Deleted
		if(fileMetaData._fileType == NOTFOUND) {
			return -ENOENT;
		}

		stbuf->st_size = fileMetaData._size;
	}

	return 0;
}

static int ncvfs_open(const char *path, struct fuse_file_info *fi) {
	uint32_t fileId = checkNameSpace(path);
	if (fileId == 0) {
		debug("File %s Does Not Exist\n", path);
		return -ENOENT;
	} else {
		struct FileMetaData fileMetaData;
		fileMetaData = getAndCacheFileMetaData(fileId);

		//In Case the Record on MDS is Deleted
		if(fileMetaData._fileType == NOTFOUND) {
			return -ENOENT;
		}

		fi->fh = fileMetaData._id;
		debug("Open File %s with ID %" PRIu64 "\n",path,fi->fh);
	}
	return 0;
}

static int ncvfs_create(const char * path, mode_t mode, struct fuse_file_info *fi) {
	string fpath = _fuseFolder + string(path);

	uint32_t segmentCount = configLayer->getConfigInt("Fuse>PreallocateSegmentNumber");
	struct FileMetaData fileMetaData = _clientCommunicator->uploadFile(_clientId, path, 0, segmentCount);
	//struct FileMetaData fileMetaData = _clientCommunicator->uploadFile(_clientId, path, 0, 0);
	fileMetaData._fileType = NORMAL;
	_fileMetaDataCache->saveMetaData(fileMetaData);
	fi->fh = fileMetaData._id;

	FILE* fp = fopen(fpath.c_str(),"w");
	if (fp == NULL) {
		perror("fopen()");
		exit(-1);
	}
	fprintf(fp,"%" PRIu32, fileMetaData._id);
	fclose(fp);
	return 0;
}

static int ncvfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	uint32_t fileId = fi->fh;
	struct FileMetaData fileMetaData = getAndCacheFileMetaData(fileId);
	uint64_t sizeRead = 0;
	uint32_t lastSegmentCount = 0;
	char* bufptr = buf;
	if(offset >= fileMetaData._size)
		return 0;
	while (sizeRead < size) {
		// TODO: Check Read Size
		uint32_t segmentCount = (offset + sizeRead) / _segmentSize;		// position of segment in the file
		uint64_t segmentId = fileMetaData._segmentList[segmentCount];
		uint32_t primary = fileMetaData._primaryList[segmentCount];
		uint32_t segmentOffset = offset + sizeRead - ((uint64_t)segmentCount * _segmentSize);	// offset within the segment
		debug("Read at %" PRIu64 " for %" PRIu64 ", Seg Cnt %" PRIu32 " off %" PRIu32 "\n", offset, size, segmentCount, segmentOffset);
		uint32_t readSize = _segmentSize - segmentOffset;
		if (size - sizeRead < readSize)
			readSize = size - sizeRead;
		if (fileMetaData._size - offset - sizeRead < readSize)
			readSize = fileMetaData._size - offset - sizeRead;
		debug("Seg Size %" PRIu32 ", Seg Off %" PRIu32 ", Size %" PRIu64 ", Size Read %" PRIu64 ", File Size %" PRIu64 ", Offset %" PRIu64 "\n", _segmentSize, segmentOffset, size, sizeRead, fileMetaData._size, offset);

		// return immediately if data is cached, otherwise retrieve data from OSDs
		uint32_t retstat = _fileDataCache->readDataCache(segmentId, primary, bufptr, readSize, segmentOffset);
		bufptr += retstat;
		sizeRead += retstat;
		lastSegmentCount = segmentCount;
		if (fileMetaData._size <= offset + sizeRead)
			break;
	}

	// prefetch the next _prefetchCount segments
	for (uint32_t i = 0; i < _prefetchCount; ++i) {
		uint32_t segmentCount = lastSegmentCount + i;
		if(segmentCount < fileMetaData._segmentList.size())
			_fileDataCache->prefetchSegment(fileMetaData._segmentList[segmentCount], fileMetaData._primaryList[segmentCount]);
	}
	return (int)sizeRead;
}

static int ncvfs_write(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi) {
	uint32_t fileId = fi->fh;
	struct FileMetaData fileMetaData = getAndCacheFileMetaData(fileId);
	uint64_t sizeWritten = 0;
	const char* bufptr = buf;
	while (sizeWritten < size) {
		uint32_t segmentCount = (offset + sizeWritten) / _segmentSize;
		while (segmentCount >= fileMetaData._segmentList.size()) {
            writeLock wtLock(*obtainFileRWMutex(fileId));
            fileMetaData = getAndCacheFileMetaData(fileId);
            if (segmentCount >= fileMetaData._segmentList.size()) {
                fileMetaData = getAndCacheFileMetaData(fileId);
			    struct SegmentMetaData segmentMetaData = allocateSegmentMetaData();
			    fileMetaData._segmentList.push_back(segmentMetaData._id);
			    fileMetaData._primaryList.push_back(segmentMetaData._primary);
                _fileMetaDataCache->saveMetaData(fileMetaData);
            } // else someone else has already allocate for this offset
		}
		uint64_t segmentId = fileMetaData._segmentList[segmentCount];
		uint32_t primary = fileMetaData._primaryList[segmentCount];
		uint32_t segmentOffset = offset + sizeWritten - (segmentCount * _segmentSize);
		uint32_t writeSize = _segmentSize - segmentOffset;
		if (size - sizeWritten < writeSize)
			writeSize = _segmentSize - segmentOffset;
		uint32_t retstat = _fileDataCache->writeDataCache(segmentId, primary, bufptr, writeSize, segmentOffset, fileMetaData._fileType);
		bufptr += retstat;
		sizeWritten += retstat;
	}
	if((offset + sizeWritten) > fileMetaData._size)
		fileMetaData._size = offset + sizeWritten;
	_fileMetaDataCache->saveMetaData(fileMetaData);
	return (int)sizeWritten;
}

static void ncvfs_destroy(void* userdata) {
}

static int ncvfs_chmod(const char *path, mode_t mode) {
	string fpath = _fuseFolder + string(path);
	return chmod(fpath.c_str(),mode);
	//return 0;
}

static int ncvfs_chown(const char *path, uid_t uid, gid_t gid) {
	string fpath = _fuseFolder + string(path);
	return chown(fpath.c_str(),uid,gid);
	//return 0;
}

static int ncvfs_utime(const char *path, struct utimbuf *ubuf) {
	string fpath = _fuseFolder + string(path);
	return utime(fpath.c_str(), ubuf);
	//return 0;
}

/*
   static int ncvfs_mknod(const char *path, mode_t mode, dev_t dev) {
   return 0;
   }
 */

static int ncvfs_mkdir(const char *path, mode_t mode) {
	string fpath = _fuseFolder + string(path);
	return mkdir(fpath.c_str(), mode);
}

static int ncvfs_unlink(const char *path) {
	removeNameSpace(path);
	uint32_t fileId = _fileMetaDataCache->path2Id(string(path));
	if(fileId > 0)
		_fileMetaDataCache->removeMetaData(fileId);
	return 0;
}

static int ncvfs_rmdir(const char *path) {
	string fpath = _fuseFolder + string(path);
	return rmdir(fpath.c_str());
}

static int ncvfs_rename(const char *path, const char *newpath) {
	string fpath = _fuseFolder + string(path);
	string new_fpath = _fuseFolder + string(newpath);
    string new_path = string(newpath);

	/// TODO: Check Return Value
	_fileMetaDataCache->renameMetaData(string(path), new_path);

    uint32_t id = _fileMetaDataCache->path2Id(new_path);

    client->renameFileRequest(id, new_path);

	return rename(fpath.c_str(), new_fpath.c_str());
}

static int ncvfs_truncate(const char *path, off_t newsize) {
	/// TODO: Support truncate to size other than 0
	if(newsize > 0) {
		debug_error("%s\n","Only Truncate to 0 is supported");
		return 0;
	}

	uint32_t fileId = checkNameSpace(path);
	if (fileId == 0) {
		debug("File %s Does Not Exist\n", path);
		return -ENOENT;
	}

	/// TODO: Exception
	struct FileMetaData fileMetaData = _fileMetaDataCache->getMetaData(fileId);

	fileMetaData._size = newsize;
	/// TODO: Discard File Data Cache
	fileMetaData._segmentList.clear();
	fileMetaData._primaryList.clear();
	_fileMetaDataCache->saveMetaData(fileMetaData);

	client->truncateFileRequest(fileId);
	return 0;
}

static int ncvfs_flush(const char *path, struct fuse_file_info *fi) {
	debug("Flush %s [%" PRIu32 "]\n", path, (uint32_t)fi->fh);

	uint32_t fileId = (uint32_t)fi->fh;

	/*
	string fpath = _fuseFolder + string(path);
	FILE* fp = fopen(fpath.c_str(),"w");
	fprintf(fp,"%" PRIu32, fileId);
	fclose(fp);
	*/

	struct FileMetaData fileMetaData = _fileMetaDataCache->getMetaData(fileId);

	for(uint32_t i = 0; i < fileMetaData._segmentList.size(); ++i) {
		_fileDataCache->closeDataCache(fileMetaData._segmentList[i], true);
	}

	_clientCommunicator->saveFileSize(_clientId, fileId, fileMetaData._size);
	_clientCommunicator->saveSegmentList(_clientId, fileId, fileMetaData._segmentList);
	return 0;
}

/*
   static int ncvfs_fsync(const char *path, int datasync, struct fuse_file_info *fi) {
   return 0;
   }
 */

static int ncvfs_opendir(const char *path, struct fuse_file_info *fi) {
	string fpath = _fuseFolder + string(path);
	DIR* dp = opendir(fpath.c_str());
	if(dp == NULL)
		return -errno;
	else fi->fh = (intptr_t)dp;
	return 0;
}

int ncvfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi) {

	DIR* dp;
	struct dirent *de;
	dp = (DIR *) (uintptr_t) fi->fh;
	de = readdir(dp);
	if (de == 0)
		return -errno;
	do {
		if (filler(buf, de->d_name, NULL, 0) != 0)
			return -ENOMEM;
	} while ((de = readdir(dp)) != NULL);

	return 0;
}

static int ncvfs_release(const char* path, struct fuse_file_info *fi) {
	debug("Release %s [%" PRIu32 "]\n", path, (uint32_t)fi->fh);

	ncvfs_flush(path,fi);

	/*
	uint32_t fileId = (uint32_t)fi->fh;
	_fileMetaDataCache->removeMetaData(fileId);
	*/
	return 0;
}

static int ncvfs_releasedir(const char *path, struct fuse_file_info *fi) {
	return closedir((DIR *) (uintptr_t) fi->fh);
}

/*
   static int ncvfs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi) {
   return 0;
   }
 */

static int ncvfs_access(const char *path, int mask) {
	string fpath = _fuseFolder + string(path);
	return access(fpath.c_str(),mask);
}

/*
   static int ncvfs_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi) {
   return 0;
   }

static int ncvfs_fgetattr(const char *path, struct stat *statbuf,
		struct fuse_file_info *fi) {
	return ncvfs_getattr(path, statbuf);
	//	return 0;
}
*/

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
//		mknod = ncvfs_mknod; // not required
		mkdir = ncvfs_mkdir;
		unlink = ncvfs_unlink;
		rmdir = ncvfs_rmdir;
//		symlink = ncvfs_symlink; // not required and not implemented
		rename = ncvfs_rename;
//		link = ncvfs_link; // not required
		chmod = ncvfs_chmod;
		chown = ncvfs_chown; // not required
		truncate = ncvfs_truncate;
//		ftruncate = ncvfs_ftruncate; // Would call truncate instead
		utime = ncvfs_utime; // not required
		open = ncvfs_open;
		read = ncvfs_read;
		write = ncvfs_write;
//		statfs = ncvfs_statfs; // not required
		release = ncvfs_release;
		releasedir = ncvfs_releasedir;
//		fsync = ncvfs_fsync; // not strictly required
//		fsyncdir = ncvfs_fsyncdir; // not strictly required
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
    _clientId = atoi(argv[argc-1]); // last argument is client id
	_cwdpath = (char*)calloc(sizeof(PATH_MAX) * PATH_MAX,1);
	if(getcwd(_cwdpath, PATH_MAX) == NULL){
		perror("getcwd()");
		exit(-1);
	}
	debug("Current Directory = %s\n",_cwdpath);
	return fuse_main(argc-1, argv, &ncvfs_oper, _cwdpath);
}
