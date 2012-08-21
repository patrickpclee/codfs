#ifndef __CLIENT_HH__
#define __CLIENT_HH__

#include <stdint.h>

#include "client_communicator.hh"
#include "client_storagemodule.hh"

#include "../cache/cache.hh"
#include "../common/metadata.hh"

class Client {
public:
//	uint32_t uploadFileRequest(char* srcPath, char* dstPath);

	Client();

	/**
	 * @brief	Request to List a Folder
	 *
	 * @param	path	Path to the Folder
	 *
	 * @return	Vector of FileMetaData
	 */
	vector<FileMetaData> listFolderRequest(char* path);

	uint32_t uploadFileRequest(string path, CodingScheme codingScheme,
			string codingSetting);
	/**
	 * Upload a file to OSD
	 * @param filepath Location of the file to upload
	 * @param codingScheme Coding Scheme specified
	 * @return 0 if success, -1 if failure
	 */

//	uint32_t sendFileRequest(string filepath, CodingScheme codingScheme, string codingSetting);
	/**
	 * @brief	Get the Client Communicator
	 *
	 * @return	Pointer to the Client Communicator Module
	 */
	ClientCommunicator* getCommunicator();

//	void downloadFileRequest(char* srcPath, char* dstPath);

	/**
	 * @brief	Download a file from OSDs
	 * @param	fileId	File ID
	 * @param	dstPath	Location to save the file
	 */
	void downloadFileRequest(uint32_t fileId, string dstPath);

	/**
	 * @brief	putObjectInitRequestMsg Handler: update chunkCount in pendingChunkMap
	 * @param	requestId	Request ID
	 * @param   sockfd 		Socket file descriptor
	 * @param	objectId	Object ID
	 * @param	length		Data Length
	 * @param 	chunkCount	Number of Chunks
	 */
	void putObjectInitProcessor(uint32_t requestId, uint32_t sockfd, uint64_t objectId, uint32_t length, uint32_t chunkCount);

	/**
	 * @brief	ObjectDataMsg Handler: receive Object Data
	 * @param	requestId	Request ID
	 * @param   sockfd 		Socket file descriptor
	 * @param	objectId	Object ID
	 * @param	offset		Offset in the file
	 * @param	length		Data Length
	 * @param 	buf			The Buffer contains the data
	 */
	uint32_t ObjectDataProcessor(uint32_t requestId, uint32_t sockfd, uint64_t objectId, uint64_t offset, uint32_t length, char* buf);

	/**
	 * @brief	putObjectEndRequestMsg Handler: counting chunkCount in pendingChunkMap
	 * @param	requestId	Request ID
	 * @param   sockfd 		Socket file descriptor
	 * @param	objectId	Object ID
	 */
	void putObjectEndProcessor(uint32_t requestId, uint32_t sockfd, uint64_t objectId);

	void updatePendingObjectChunkMap(uint64_t objectId, uint32_t chunkCount);
	void removePendingObjectFromMap(uint64_t objectId);

	uint32_t getPendingChunkCount(uint64_t objectId);
	void setPendingChunkCount(uint64_t objectId, int32_t chunkCount);

	uint32_t getClientId ();
private:

	/**
	 * Upload an object to OSD
	 * @param dstOsdID Destination of the OSD
	 * @param objectData ObjectData structure
	 */

	void uploadObjectRequest(uint32_t dstOsdID, struct ObjectData objectData);

//	void reportPrimaryFailure(OsdInfo down_osd_info, uint64_t timestamp);

//	void contactPrimaryUpload(DataObject* obj_list, uint32_t fileId);
//	DataObject* contactPrimaryDownload(uint32_t fileId);

//	DataObject* split(char* path, uint32_t num_of_trunk);
//	char* merge(DataObject* obj_list);

	uint32_t _clientId;
	uint32_t _ip;
	uint16_t _port;
//	Cache _cache;

	ClientCommunicator* _clientCommunicator;
	ClientStorageModule* _storageModule;

	map<uint64_t, int32_t> _pendingObjectChunk;
};
#endif
