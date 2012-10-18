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
	 * @brief	Delete a File
	 *
	 * @param	clientId	Client ID
	 * @param	path	File Path
	 * @param	fileId	File ID
	 */
	void deleteFile(uint32_t clientId, string path, uint32_t fileId);

	/**
	 * @brief	Get Download Info
	 * @param clientId Client ID
	 * @param fileId File ID
	 * @return FileMetaData structure
	 */

	struct FileMetaData downloadFile(uint32_t clientId, uint32_t fileId);

	/**
	 * @brief	Get Download Info
	 *
	 * @param	clientId	Client ID
	 * @param	filePath	File Path
	 *
	 * @return	File Meta Data
	 */
	struct FileMetaData downloadFile(uint32_t clientId, string filePath);

	/**
	 * @brief	Get File Info
	 *
	 * @param	clientId	Client ID
	 * @param	fileId	ID of the file
	 *
	 * @return	File Meta Data
	 */
	struct FileMetaData getFileInfo(uint32_t clientId, uint32_t fileId);

	/**
	 * @brief	Get File Info
	 *
	 * @param	clientId	Client ID
	 * @param	filePath	File Path
	 *
	 * @return	File Meta Data
	 */
	struct FileMetaData getFileInfo(uint32_t clientId, string filePath);

	/**
	 * @brief	Save ObjectList of a File
	 *
	 * @param	clientId	Client ID
	 * @param	fileId	ID of the File
	 * @param	objectList	Object List of the File
	 */

	void saveObjectList (uint32_t clientId, uint32_t fileId, vector<uint64_t> objectList);

	/**
	 * @brief	Get New Object List
	 *
	 * @param	clientId	Client ID
	 * @param	numOfObjs	Number of Objects
	 */
	vector<struct ObjectMetaData> getNewObjectList (uint32_t clientId, uint32_t numOfObjs);

	/**
	 * @brief	Save Object Size of a File
	 *
	 * @param	clientId	Client ID
	 * @param	fileId	ID of the File
	 * @param	fileSize	Size of the file
	 */
	void saveFileSize (uint32_t clientId, uint32_t fileId, uint64_t fileSize);

	/**
	 * @brief	Send a request to OSD for the object
	 * @param dstSockfd OSD Socket Descriptor
	 * @param objectId Object ID
	 */

	void requestObject (uint32_t dstSockfd, uint64_t objectId);

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

	/**
	 * Send an initiate message of a object data transfer.
	 * @param requestId 	Request ID
	 * @param connectionId 	connection ID
	 * @param objectId 		object ID
	 */
	void replyPutObjectInit(uint32_t requestId, uint32_t connectionId, uint64_t objectId); // new version

	/**
	 * Send an end message of an object data transfer.
	 * @param requestId 	Request ID
	 * @param connectionId 	connection ID
	 * @param objectId 		object ID
	 */
	void replyPutObjectEnd(uint32_t requestId, uint32_t connectionId, uint64_t objectId); //new version

	/**
	 * Send a request to monitor to get Osd List and connect 
	 */
	void getOsdListAndConnect();

private:

};
#endif
