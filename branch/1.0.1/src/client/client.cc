#include <iostream>
#include <cstdio>
#include <thread>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include "client.hh"
#include "client_storagemodule.hh"
#include "../config/config.hh"
#include "../common/debug.hh"
#include "../common/segmentdata.hh"
#include "../common/blockdata.hh"
#include "../common/convertor.hh"
#include "../common/define.hh"

using namespace std;

extern Client* client;
extern ConfigLayer* configLayer;

Client::Client(uint32_t clientId) {
	_clientCommunicator = new ClientCommunicator();
	_storageModule = new ClientStorageModule();

	_clientId = clientId;

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

void startUploadThread(uint32_t clientId, uint32_t sockfd,
		struct SegmentData segmentData, CodingScheme codingScheme,
		string codingSetting) {
	client->getCommunicator()->sendSegment(clientId, sockfd, segmentData,
			codingScheme, codingSetting);
	MemoryPool::getInstance().poolFree(segmentData.buf);
}

void startDownloadThread(uint32_t clientId, uint32_t sockfd, uint64_t segmentId,
		uint64_t offset, FILE* filePtr, string dstPath) {
	client->getSegment(clientId, sockfd, segmentId, offset, filePtr, dstPath);
	debug("Segment ID = %" PRIu64 " finished download\n", segmentId);
}

struct SegmentData Client::getSegment(uint32_t clientId,
		uint32_t dstSockfd, uint64_t segmentId) {
	struct SegmentData segmentCache = { };

	// If currently modifying cache, wait!
	while (_pendingSegmentChunk.count(segmentId)) {
		usleep(USLEEP_DURATION);
	}

	// get segment from cache directly if possible
	if (_storageModule->locateSegmentCache(segmentId)) {
		segmentCache = _storageModule->getSegmentCache(segmentId);
		return segmentCache;
	}

	// unknown number of chunks at this point
    if (_pendingSegmentChunk.init(segmentId, -1)) {
        // if it is the first to download, request segment
	    _clientCommunicator->requestSegment(dstSockfd, segmentId);
    }

	// wait until the segment is fully downloaded
	while (_pendingSegmentChunk.count(segmentId)) {
		usleep(USLEEP_DURATION);
	}

	// write segment from cache to file
	segmentCache = _storageModule->getSegmentCache(segmentId);
	return segmentCache;
}

void Client::getSegment(uint32_t clientId, uint32_t dstSockfd, uint64_t segmentId,
		uint64_t offset, FILE* filePtr, string dstPath) {

	struct SegmentData segmentCache = { };

	segmentCache = getSegment(clientId, dstSockfd, segmentId);

	_storageModule->writeFile(filePtr, dstPath, segmentCache.buf, offset,
			segmentCache.info.segLength);

	debug(
			"Write Segment ID: %" PRIu64 " Offset: %" PRIu64 " Length: %" PRIu32 " to %s\n",
			segmentId, offset, segmentCache.info.segLength, dstPath.c_str());

	_storageModule->closeSegment(segmentId);

}

uint32_t Client::uploadFileRequest(string path, CodingScheme codingScheme,
		string codingSetting) {

	// start timer
	typedef chrono::high_resolution_clock Clock;
	typedef chrono::milliseconds milliseconds;
	Clock::time_point t0 = Clock::now();

	const uint32_t segmentCount = _storageModule->getSegmentCount(path);
	const uint64_t fileSize = _storageModule->getFilesize(path);

	debug("Segment Count of %s: %" PRIu32 "\n", path.c_str(), segmentCount);

	struct FileMetaData fileMetaData = _clientCommunicator->uploadFile(
			_clientId, path, fileSize, segmentCount);

	debug("File ID %" PRIu32 "\n", fileMetaData._id);

	for (uint32_t i = 0; i < fileMetaData._primaryList.size(); ++i) {
		debug("%" PRIu64 "[%" PRIu32 "]\n",
				fileMetaData._segmentList[i], fileMetaData._primaryList[i]);
	}

	for (uint32_t i = 0; i < segmentCount; ++i) {
		struct SegmentData segmentData = _storageModule->readSegmentFromFile(path,
				i);
		uint32_t primary = fileMetaData._primaryList[i];
		debug("Send to Primary [%" PRIu32 "]\n", primary);
		uint32_t dstOsdSockfd = _clientCommunicator->getSockfdFromId(primary);
		segmentData.info.segmentId = fileMetaData._segmentList[i];

		_tp.schedule(
				boost::bind(startUploadThread, _clientId, dstOsdSockfd,
						segmentData, codingScheme, codingSetting));
	}

	// wait for every thread to finish
	_tp.wait();

	// Time and Rate calculation (in seconds)
	Clock::time_point t1 = Clock::now();
	milliseconds ms = chrono::duration_cast < milliseconds > (t1 - t0);
	double duration = ms.count() / 1000.0;

	// allow time for messages to go out
	usleep(USLEEP_DURATION);

	cout << "Upload " << path << " Done [" << fileMetaData._id << "]" << endl;

	cout << fixed;
	cout << setprecision(2);
	cout << formatSize(fileSize) << " transferred in " << duration
			<< " secs, Rate = " << formatSize(fileSize / duration) << "/s"
			<< endl;

	return fileMetaData._id;
}

void Client::deleteFileRequest(string path, uint32_t fileId) {
	_clientCommunicator->deleteFile(_clientId, path, fileId);
}

void Client::renameFileRequest(uint32_t fileId, const string& newPath) {
	_clientCommunicator->renameFile(_clientId, fileId, "", newPath);
}

void Client::renameFileRequest(const string& path, const string& newPath) {
	_clientCommunicator->renameFile(_clientId, 0, path, newPath);
}

void Client::truncateFileRequest(uint32_t fileId) {
	_clientCommunicator->saveFileSize(_clientId, fileId, 0);
	_clientCommunicator->saveSegmentList(_clientId, fileId, {});
}

void Client::downloadFileRequest(uint32_t fileId, string dstPath) {

	const uint64_t segmentSize = _storageModule->getSegmentSize();

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

	vector <uint64_t> segmentList = fileMetaData._segmentList;

	uint32_t i = 0;
	for (uint64_t segmentId : segmentList) {
		uint32_t dstComponentId = fileMetaData._primaryList[i];
		uint32_t dstSockfd = _clientCommunicator->getSockfdFromId(
				dstComponentId);

		const uint64_t offset = segmentSize * i;
		_tp.schedule(
				boost::bind(startDownloadThread, _clientId, dstSockfd, segmentId,
						offset, filePtr, dstPath));

		i++;
	}

	_tp.wait();

	_storageModule->closeFile(filePtr);

	// Time and Rate calculation (in seconds)
	Clock::time_point t1 = Clock::now();
	milliseconds ms = chrono::duration_cast < milliseconds > (t1 - t0);
	double duration = ms.count() / 1000.0;

	// allow time for messages to go out
	usleep(USLEEP_DURATION);

	cout << fixed;
	cout << setprecision(2);
	cout << formatSize(fileMetaData._size) << " transferred in " << duration
			<< " secs, Rate = " << formatSize(fileMetaData._size / duration)
			<< "/s" << endl;
}

void Client::putSegmentInitProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t segmentId, uint32_t segLength, uint32_t bufLength, uint32_t chunkCount,
		bool isSmallSegment) {

	// initialize chunkCount value
	_pendingSegmentChunk.set(segmentId, chunkCount);
	debug("Init Chunkcount = %" PRIu32 "\n", chunkCount);

	// create segment and cache
	if (!_storageModule->locateSegmentCache(segmentId))
    {
		_storageModule->createSegmentCache(segmentId, segLength, bufLength);
        memset(_storageModule->getSegmentCache(segmentId).buf, 'a', segLength);
    }

	if (!isSmallSegment) {
	    _clientCommunicator->replyPutSegmentInit(requestId, sockfd, segmentId);
	}

}

void Client::putSegmentEndProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t segmentId, bool isSmallSegment) {

	// TODO: check integrity of segment received
	bool chunkRemaining = false;

	while (1) {

		chunkRemaining = _pendingSegmentChunk.count(segmentId);

		if (!chunkRemaining) {
			// if all chunks have arrived, send ack
			_clientCommunicator->replyPutSegmentEnd(requestId, sockfd, segmentId, isSmallSegment);
			break;
		} else {
			usleep(USLEEP_DURATION); // sleep 0.01s
		}

	}
}

uint32_t Client::SegmentDataProcessor(uint32_t requestId, uint32_t sockfd,
		uint64_t segmentId, uint64_t offset, uint32_t length, char* buf) {

	uint32_t byteWritten;
	byteWritten = _storageModule->writeSegmentCache(segmentId, buf, offset,
			length);
	_pendingSegmentChunk.decrement(segmentId);

	if (_pendingSegmentChunk.get(segmentId) == 0) {
		_pendingSegmentChunk.erase(segmentId);
	}
	return byteWritten;
}

uint32_t Client::getClientId() {
	return _clientId;
}
