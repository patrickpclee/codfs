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

	uint32_t uploadFileRequest (string path); 
	/**
	 * Upload a file to OSD
	 * @param filepath Location of the file to upload
	 * @return 0 if success, -1 if failure
	 */

	uint32_t sendFileRequest(string filepath);

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
	Cache _cache;

	ClientCommunicator* _clientCommunicator;
	ClientStorageModule* _storageModule;

	// config
	uint64_t _objectSize;
};
#endif
