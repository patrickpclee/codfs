#ifndef __CLIENT_HH__
#define __CLIENT_HH__

#include <stdint.h>

#include "client_communicator.hh"
#include "client_storagemodule.hh"

#include "../cache/cache.hh"
#include "../common/metadata.hh"
#include "../datastructure/concurrentmap.hh"

class Client {
public:

	Client();

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

	uint32_t uploadFileRequest(string path, CodingScheme codingScheme, string codingSetting);
//	uint32_t uploadFileRequest(char* srcPath, char* dstPath);
//	uint32_t sendFileRequest(string filepath, CodingScheme codingScheme, string codingSetting);

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
//	void downloadFileRequest(char* srcPath, char* dstPath);

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

	/**
	 * @brief	get the chunkCount in pendingChunkMap
	 * @param	objectId	Object ID
	 *
	 * @return	int chunkCount
	 */
	int getPendingChunkCount(uint64_t objectId);

	/**
	 * @brief	set the chunkCount in pendingChunkMap
	 * @param	objectId	Object ID
	 * @param	chunkCount	Number of chunks
	 */
	void setPendingChunkCount(uint64_t objectId, int chunkCount);

	/**
	 * @brief	get the client ID
	 *
	 * @return	uint32_t 	client ID
	 */
	uint32_t getClientId ();
private:

	/**
	 * Upload an object to OSD
	 * @param dstOsdID Destination of the OSD
	 * @param objectData ObjectData structure
	 */

//	void uploadObjectRequest(uint32_t dstOsdID, struct ObjectData objectData);

//	void reportPrimaryFailure(OsdInfo down_osd_info, uint64_t timestamp);

//	void contactPrimaryUpload(DataObject* obj_list, uint32_t fileId);
//	DataObject* contactPrimaryDownload(uint32_t fileId);

//	DataObject* split(char* path, uint32_t num_of_trunk);
//	char* merge(DataObject* obj_list);

	uint32_t _clientId;
	uint32_t _ip;
	uint16_t _port;

	ClientCommunicator* _clientCommunicator;
	ClientStorageModule* _storageModule;

	ConcurrentMap<uint64_t, int> _pendingObjectChunk;
};
#endif
