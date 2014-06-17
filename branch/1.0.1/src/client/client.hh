#ifndef __CLIENT_HH__
#define __CLIENT_HH__

#include <stdint.h>
#include <boost/thread/thread.hpp>

#include "client_communicator.hh"
#include "client_storagemodule.hh"

#include "../cache/cache.hh"
#include "../common/metadata.hh"
#include "../datastructure/concurrentmap.hh"
#include "../../lib/threadpool/threadpool.hpp"

class Client {
public:

	Client(uint32_t clientId);

	/**
	 * @brief	Request to List a Folder
	 *
	 * @param	path	Path to the Folder
	 *
	 * @return	Vector of FileMetaData
	 */
	vector<FileMetaData> listFolderRequest(char* path);

	/**
	 * Upload a file to OSD
	 * @param filepath Location of the file to upload
	 * @param codingScheme Coding Scheme specified
	 * @return 0 if success, -1 if failure
	 */
	uint32_t uploadFileRequest(string path, CodingScheme codingScheme,
			string codingSetting);

	/**
	 * @brief	Request to Delete a File
	 *
	 * @param	path	Path of the File;
	 * @param	fileId	ID of the File;
	 */
	void deleteFileRequest(string path, uint32_t fileId);

	/**
	 * @brief	Request to Rename a File
	 *
	 * @param	fileId	File ID
	 * @param	newPath	New File Path
	 */
	void renameFileRequest(uint32_t fileId, const string& newPath);

	/**
	 * @brief	Request to Rename a File
	 *
	 * @param	path	File Path
	 * @param	newPath	New File Path
	 */
	void renameFileRequest(const string& path, const string& newPath);

	/**
	 * @brief	Requset to Truncate a File
	 *
	 * @param	fileId	ID of the File
	 */
	void truncateFileRequest(uint32_t fileId);

	/**
	 * @brief	Get the Client Communicator
	 *
	 * @return	Pointer to the Client Communicator Module
	 */
	ClientCommunicator* getCommunicator();

	/**
	 * @brief	Get the Client StorageModule
	 *
	 * @return	Pointer to the Client Storage Module
	 */
	ClientStorageModule* getStorageModule();

	/**
	 * @brief	Download a file from OSDs
	 * @param	fileId	File ID
	 * @param	dstPath	Location to save the file
	 */
	void downloadFileRequest(uint32_t fileId, string dstPath);

	/**
	 * @brief	putSegmentInitRequestMsg Handler: update chunkCount in pendingChunkMap
	 * @param	requestId	Request ID
	 * @param   sockfd 		Socket file descriptor
	 * @param	segmentId	Segment ID
	 * @param	segLength	Segment Length
	 * @param	bufLength	Buffer Length
	 * @param 	chunkCount	Number of Chunks
	 */
	void putSegmentInitProcessor(uint32_t requestId, uint32_t sockfd,
			uint64_t segmentId, uint32_t segLength, uint32_t bufLength,
            uint32_t chunkCount, bool isSmallSegment = false);

	/**
	 * @brief	SegmentDataMsg Handler: receive Segment Data
	 * @param	requestId	Request ID
	 * @param   sockfd 		Socket file descriptor
	 * @param	segmentId	Segment ID
	 * @param	offset		Offset in the file
	 * @param	length		Data Length
	 * @param 	buf			The Buffer contains the data
	 */
	uint32_t SegmentDataProcessor(uint32_t requestId, uint32_t sockfd,
			uint64_t segmentId, uint64_t offset, uint32_t length, char* buf);

	/**
	 * @brief	putSegmentEndRequestMsg Handler: counting chunkCount in pendingChunkMap
	 * @param	requestId	Request ID
	 * @param   sockfd 		Socket file descriptor
	 * @param	segmentId	Segment ID
	 */
	void putSegmentEndProcessor(uint32_t requestId, uint32_t sockfd,
			uint64_t segmentId, bool isSmallSegment = false);

	/**
	 * @brief	get the client ID
	 *
	 * @return	uint32_t 	client ID
	 */
	uint32_t getClientId();

	struct SegmentData getSegment(uint32_t clientId, uint32_t dstSockfd, uint64_t segmentId);
	void getSegment(uint32_t clientId, uint32_t dstSockfd, uint64_t segmentId,
			uint64_t offset, FILE* filePtr, string dstPath);

private:

	uint32_t _clientId;
	uint32_t _ip;
	uint16_t _port;

	ClientCommunicator* _clientCommunicator;
	ClientStorageModule* _storageModule;

	ConcurrentMap<uint64_t, int> _pendingSegmentChunk;

	// thread pool for upload
	uint32_t _numClientThreads;
	boost::threadpool::pool _tp;

};
#endif
