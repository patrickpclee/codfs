#include <iostream>
#include <cstdio>
#include <thread>
#include <iomanip>
#include <chrono>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <openssl/md5.h>
#include "client.hh"
#include "client_storagemodule.hh"
#include "../config/config.hh"
#include "../common/debug.hh"
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"
#include "../common/convertor.hh"
#include "../../lib/logger.hh"

using namespace std;

#define PARALLEL_TRANSFER

extern Client* client;
extern ConfigLayer* configLayer;

Client::Client() {
	_clientCommunicator = new ClientCommunicator();
	_storageModule = new ClientStorageModule();

	_clientId = configLayer->getConfigInt("Clientid");

	_numClientThreads = configLayer->getConfigInt(
			"Communication>NumClientThreads");
	_tp.size_controller().resize(_numClientThreads);
}

/**
 * @brief	Get the Client Communicator
 *
 * @return	Pointer to the Client Communicator Module
 */
ClientCommunicator* Client::getCommunicator() {
	return _clientCommunicator;
}

ClientStorageModule* Client::getStorageModule() {
	return _storageModule;
}

#ifdef PARALLEL_TRANSFER

void startUploadThread(uint32_t clientId, uint32_t sockfd,
		struct ObjectData objectData, CodingScheme codingScheme,
		string codingSetting, string checksum) {
	client->getCommunicator()->sendObject(clientId, sockfd, objectData,
			codingScheme, codingSetting, checksum);
	MemoryPool::getInstance().poolFree(objectData.buf);
}

void startDownloadThread(uint32_t clientId, uint32_t sockfd, uint64_t objectId,
		uint64_t offset, FILE* filePtr, string dstPath) {
	client->getObject(clientId, sockfd, objectId, offset, filePtr, dstPath);
	debug("Object ID = %" PRIu64 " finished download\n", objectId);
}

#endif

struct ObjectTransferCache Client::getObject(uint32_t clientId,
		uint32_t dstSockfd, uint64_t objectId) {
	struct ObjectTransferCache objectCache = { };

	// get object from cache directly if possible
	if (_storageModule->locateObjectCache(objectId)) {
		objectCache = _storageModule->getObjectCache(objectId);
		return objectCache;
	}

	// unknown number of chunks at this point
	_pendingObjectChunk.set(objectId, -1);

	_clientCommunicator->requestObject(dstSockfd, objectId);

	// wait until the object is fully downloaded
	while (_pendingObjectChunk.count(objectId)) {
		usleep(10000);
	}

	// write object from cache to file
	objectCache = _storageModule->getObjectCache(objectId);

	return objectCache;
}

void Client::getObject(uint32_t clientId, uint32_t dstSockfd, uint64_t objectId,
		uint64_t offset, FILE* filePtr, string dstPath) {

	struct ObjectTransferCache objectCache = { };

	objectCache = getObject(clientId, dstSockfd, objectId);

	_storageModule->writeFile(filePtr, dstPath, objectCache.buf, offset,
			objectCache.length);

	debug(
			"Write Object ID: %" PRIu64 " Offset: %" PRIu64 " Length: %" PRIu64 " to %s\n",
			objectId, offset, objectCache.length, dstPath.c_str());

	_storageModule->closeObject(objectId);

}

uint32_t Client::uploadFileRequest(string path, CodingScheme codingScheme,
		string codingSetting) {

	// start timer
	typedef chrono::high_resolution_clock Clock;
	typedef chrono::milliseconds milliseconds;
	Clock::time_point t0 = Clock::now();

	const uint32_t objectCount = _storageModule->getObjectCount(path);
	const uint64_t fileSize = _storageModule->getFilesize(path);

	debug("Object Count of %s: %" PRIu32 "\n", path.c_str(), objectCount);

	struct FileMetaData fileMetaData = _clientCommunicator->uploadFile(
			_clientId, path, fileSize, objectCount, codingScheme,
			codingSetting);

	debug("File ID %" PRIu32 "\n", fileMetaData._id);

	for (uint32_t i = 0; i < fileMetaData._primaryList.size(); ++i) {
		debug("%" PRIu64 "[%" PRIu32 "]\n",
				fileMetaData._objectList[i], fileMetaData._primaryList[i]);
	}

	for (uint32_t i = 0; i < objectCount; ++i) {
		struct ObjectData objectData = _storageModule->readObjectFromFile(path,
				i);
		uint32_t primary = fileMetaData._primaryList[i];
		debug("Send to Primary [%" PRIu32 "]\n", primary);
		uint32_t dstOsdSockfd = _clientCommunicator->getSockfdFromId(primary);
		objectData.info.objectId = fileMetaData._objectList[i];

		// get checksum
		unsigned char checksum[MD5_DIGEST_LENGTH];
		MD5((unsigned char*) objectData.buf, objectData.info.objectSize,
				checksum);

#ifdef PARALLEL_TRANSFER
		_tp.schedule(
				boost::bind(startUploadThread, _clientId, dstOsdSockfd,
						objectData, codingScheme, codingSetting,
						md5ToHex(checksum)));
#else
		_clientCommunicator->sendObject(_clientId, dstOsdSockfd, objectData,
				codingScheme, codingSetting, md5ToHex(checksum));
		MemoryPool::getInstance().poolFree(objectData.buf);
#endif
	}

#ifdef PARALLEL_TRANSFER
	// wait for every thread to finish
	_tp.wait();
#endif

	// Time and Rate calculation (in seconds)
	Clock::time_point t1 = Clock::now();
	milliseconds ms = chrono::duration_cast < milliseconds > (t1 - t0);
	double duration = ms.count() / 1024.0;

	// allow time for messages to go out
	usleep(10000);

	cout << "Upload " << path << " Done [" << fileMetaData._id << "]" << endl;

	cout << fixed;
	cout << setprecision(2);
	cout << formatSize(fileSize) << " transferred in " << duration
			<< " secs, Rate = " << formatSize(fileSize / duration) << "/s"
			<< endl;

	FILE_LOG(logINFO) << formatSize(fileSize) << " transferred in " << duration
			<< " secs, Rate = " << formatSize(fileSize / duration) << "/s";

	/*
	 int rtnval = system("./mid.sh");
	 exit(42);
	 */

	return fileMetaData._id;
}

void Client::downloadFileRequest(uint32_t fileId, string dstPath) {

	const uint64_t objectSize = _storageModule->getObjectSize();

	// start timer
	typedef chrono::high_resolution_clock Clock;
	typedef chrono::milliseconds milliseconds;
	Clock::time_point t0 = Clock::now();

	// 1. Get file infomation from MDS
	struct FileMetaData fileMetaData = _clientCommunicator->downloadFile(
			_clientId, fileId);
	debug("File ID %" PRIu32 ", Size = %" PRIu64 "\n",
			fileMetaData._id, fileMetaData._size);

	// open file
	FILE* filePtr = _storageModule->createAndOpenFile(dstPath);

	// 2. Download file from OSD

	uint32_t i = 0;
	for (uint64_t objectId : fileMetaData._objectList) {
		uint32_t dstComponentId = fileMetaData._primaryList[i];
		uint32_t dstSockfd = _clientCommunicator->getSockfdFromId(
				dstComponentId);
		const uint64_t offset = objectSize * i;
#ifdef PARALLEL_TRANSFER
		_tp.schedule(
				boost::bind(startDownloadThread, _clientId, dstSockfd, objectId,
						offset, filePtr, dstPath));
#else
		_clientCommunicator->getObjectAndWriteFile(_clientId, dstSockfd,
				objectId, offset, filePtr, dstPath);
#endif

		i++;
	}

#ifdef PARALLEL_TRANSFER
	_tp.wait();
#endif

	_storageModule->closeFile(filePtr);

	// Time and Rate calculation (in seconds)
	Clock::time_point t1 = Clock::now();
	milliseconds ms = chrono::duration_cast < milliseconds > (t1 - t0);
	double duration = ms.count() / 1024.0;

	// allow time for messages to go out
	usleep(10000);

	cout << fixed;
	cout << setprecision(2);
	cout << formatSize(fileMetaData._size) << " transferred in " << duration
			<< " secs, Rate = " << formatSize(fileMetaData._size / duration)
			<< "/s" << endl;

	FILE_LOG(logINFO) << formatSize(fileMetaData._size) << " transferred in "
			<< duration << " secs, Rate = "
			<< formatSize(fileMetaData._size / duration) << "/s";

	/*
	 int rtnval = system("./mid.sh");
	 exit(42);
	 */

}

void Client::putObjectInitProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint32_t length, uint32_t chunkCount,
		string checksum) {

	// initialize chunkCount value
	_pendingObjectChunk.set(objectId, chunkCount);
	debug("Init Chunkcount = %" PRIu32 "\n", chunkCount);

	// create object and cache
	if (!_storageModule->locateObjectCache(objectId))
		_storageModule->createObjectCache(objectId, length);
	_clientCommunicator->replyPutObjectInit(requestId, sockfd, objectId);

	// save md5 to map
	_checksumMap.set(objectId, checksum);

}

void Client::putObjectEndProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId) {

	// TODO: check integrity of object received
	bool chunkRemaining = false;

	while (1) {

		chunkRemaining = _pendingObjectChunk.count(objectId);

		if (!chunkRemaining) {
			// if all chunks have arrived, send ack
			_clientCommunicator->replyPutObjectEnd(requestId, sockfd, objectId);
			break;
		} else {
			usleep(10000); // sleep 0.1s
		}

	}
}

uint32_t Client::ObjectDataProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t objectId, uint64_t offset, uint32_t length, char* buf) {

	uint32_t byteWritten;
	byteWritten = _storageModule->writeObjectCache(objectId, buf, offset,
			length);
	_pendingObjectChunk.decrement(objectId);

	if (_pendingObjectChunk.get(objectId) == 0) {
		_pendingObjectChunk.erase(objectId);
	}
	return byteWritten;
}

uint32_t Client::getClientId() {
	return _clientId;
}
