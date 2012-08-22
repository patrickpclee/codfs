#ifndef __CLIENT_COMMUNICATOR_HH__
#define __CLIENT_COMMUNICATOR_HH__

#include <vector>

#include "../communicator/communicator.hh"
#include "../common/metadata.hh"

class ClientCommunicator: public Communicator {
public:
	void display();
	void osdIdListRequest();
	void objectMessageHandler();

	/**
	 * @brief	Send List Folder Request to MDS (Blocking)
	 *
	 * @param	clientId	Client ID
	 * @param	path	Path to the Folder
	 *
	 * @return	Folder Data
	 */
	vector<FileMetaData> listFolderData(uint32_t clientId, string path);

	/**
	 * Upload a file to OSD
	 * @param clientId Client ID
	 * @param path Destination Path
	 * @param fileSize Size of file
	 * @param numOfObjs Number of objects
	 * @param codingScheme Coding Scheme
	 * @param codingSetting Coding Setting
	 * @return FileMetaData structure
	 */

	struct FileMetaData uploadFile(uint32_t clientId, string path,
			uint64_t fileSize, uint32_t numOfObjs, CodingScheme codingScheme,
			string codingSetting);

	/**
	 * Download a file from OSD
	 * @param clientId Client ID
	 * @param fileId File ID
	 * @return FileMetaData structure
	 */

	struct FileMetaData downloadFile(uint32_t clientId, uint32_t fileId);

	/**
	 * @brief	Save ObjectList of a File
	 *
	 * @param	clientId	Client ID
	 * @param	fileId	ID of the File
	 * @param	objectList	Object List of the File
	 */
	void saveObjectList (uint32_t clientId, uint32_t fileId, vector<uint64_t> objectList);

	/**
	 * Get an object from the primary OSD and Write to File
	 * @param clientId Client ID
	 * @param dstSockfd Destination Socket Descriptor
	 * @param objectId Object ID
	 */

	void getObjectAndWriteFile(uint32_t clientId, uint32_t dstSockfd, uint64_t objectId, uint64_t offset, FILE* filePtr, string dstPath);

	/**
	 * Get an object from the primary OSD
	 * @param clientId Client ID
	 * @param dstSockfd Destination Socket Descriptor
	 * @param objectId Object ID
	 * @return ObjectData structure
	 */

	//struct objectCache getObject(uint32_t clientId, uint32_t dstSockfd, uint64_t objectId, uint64_t offset, FILE* filePtr, string dstPath);

	/**
	 * 1. Send an init message
	 * 2. Repeatedly send data chunks to OSD
	 * 3. Send an end message
	 * @param clientID Client ID
	 * @param dstOsdSockfd Destination OSD Socket Descriptor
	 * @param objectData ObjectData Structure
	 * @param codingScheme Coding Scheme specified
	 * @param codingSetting Coding Scheme setting
	 */

	void putObject(uint32_t clientId, uint32_t dstOsdSockfd,
			struct ObjectData objectData, CodingScheme codingScheme,
			string codingSetting);

	void replyPutObjectInit(uint32_t requestId, uint32_t connectionId, uint64_t objectId); // new version
	void replyPutObjectEnd(uint32_t requestId, uint32_t connectionId, uint64_t objectId); //new version

	// TODO: CONNECT TO COMPONENT: PRIMITIVE DESIGN
	void connectToMds();
	void connectToOsd();
private:

	void getObjectData(uint32_t clientID, uint32_t dstOsdSockfd,
			uint64_t objectId, char* buf, uint64_t offset, uint32_t length);

};
#endif
