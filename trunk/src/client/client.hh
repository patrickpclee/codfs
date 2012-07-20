#ifndef __CLIENT_HH__
#define __CLIENT_HH__

#include <stdint.h>

#include "client_communicator.hh"

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

	/**
	 * @brief	Get the Client Communicator
	 *
	 * @return	Pointer to the Client Communicator Module
	 */
	ClientCommunicator* getClientCommunicator();
//	void downloadFileRequest(char* srcPath, char* dstPath);
//	void downloadFileRequest(uint32_t fileId, char* dstPath);

private:
//	void reportPrimaryFailure(OsdInfo down_osd_info, uint64_t timestamp);

//	void contactPrimaryUpload(DataObject* obj_list, uint32_t fileId);
//	DataObject* contactPrimaryDownload(uint32_t fileId);

//	DataObject* split(char* path, uint32_t num_of_trunk);
//	char* merge(DataObject* obj_list);

	uint32_t _id;
	uint32_t _ip;
	uint16_t _port;
	Cache _cache;

	ClientCommunicator* _clientCommunicator;
};
#endif
