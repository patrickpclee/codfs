#ifndef __CLIENT_HH__
#define __CLIENT_HH__
class Client {
Public:
	uint32_t uploadFileRequest (char[] srcPath, char[] dstPath);
	FileMetaData[] listFolderRequest (char[] path);
	downloadFileRequest (char[] srcPath, char[] dstPath);
	downloadFileRequest (uint32_t fileId, char[] dstPath);
Private:
	reportPrimaryFailure (OsdInfo down_osd_info, uint64_t timestamp);

	void contactPrimaryUpload(DataObject[] obj_list, uint32_t fileId);
	DataObject[] contactPrimaryDownload(uint32_t fileId);

	DataObject[] split(char[] path, uint32_t num_of_trunk);
	char[] merge(DataObject[] obj_list);

	uint32_t _id;
	uint32_t _ip;
	uint16_t _port;
	Cache _cache;

	ClientCommunicator _communicator;
}
#endif
