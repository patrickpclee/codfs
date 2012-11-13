#ifndef __MDS_COMMUNICATOR_HH__
#define __MDS_COMMUNICATOR_HH__

#include "../common/metadata.hh"
#include "../communicator/communicator.hh"
#include "../common/objectlocation.hh"

#include <stdint.h>
#include <vector>
using namespace std;

class MdsCommunicator: public Communicator {
public:

	MdsCommunicator();

	void display();

	// Reply to Request

	/**
	 * @brief	Reply Object and Primary List to Client
	 *
	 * @param	requestId	Request ID
	 * @param	connectionId	Connection ID
	 * @param	fileId	File ID
	 * @param	objectList	Object List
	 * @param	primaryList	Primary List
	 */
	void replyObjectandPrimaryList(uint32_t requestId, uint32_t connectionId,
			uint32_t fileId, vector<uint64_t> objectList,
			vector<uint32_t> primaryList);

	/**
	 * @brief	Reply Download Information to Client
	 *
	 * @param	requestId	Request ID
	 * @param	connectionId	Connection ID
	 * @param	fileId	File ID
	 * @param	filePath	File Path
	 * @param	fileSize	Size of the File
	 * @param	fileType	File Type
	 * @param	checksum	Checksum of the File
	 * @param	objectList	Object List
	 * @param	primaryList	Primary List
	 */
	void replyDownloadInfo(uint32_t requestId, uint32_t connectionId,
			uint32_t fileId, string filePath, uint64_t fileSize,
			const FileType& fileType, string checksum,
			vector<uint64_t> objectList, vector<uint32_t> primaryList);

	/**
	 * @brief	Reply Delete File
	 * 
	 * @param	fileId	File Id
	 */
	void replyDeleteFile(uint32_t requestId, uint32_t connectionId,
			uint32_t fileId);

	/**
	 * @brief	Reply Object ID List
	 *
	 * @param	requestId	Request ID
	 * @param	connectionId	Connection ID
	 * @param	objectList	Object ID List
	 * @parma	primaryList	Primary List
	 */
	void replyObjectIdList(uint32_t requestId, uint32_t connectionId,
			vector<uint64_t> objectList, vector<uint32_t> primaryList);

	/**
	 * @brief	Reply Object Information to Osd
	 *
	 * @param	requestId	Request ID
	 * @param	connectionId	Connection ID
	 * @param	objectId	ID of the Object
	 * @param 	objectSize	Object Size
	 * @param	nodeList	Node List
	 * @param	codingScheme	Coding Scheme for the file
	 * @param 	codingSetting	Coding Scheme Setting
	 */
	void replyObjectInfo(uint32_t requestId, uint32_t connectionId,
			uint64_t objectId, uint32_t objectSize, vector<uint32_t> nodeList,
			CodingScheme codingScheme, string codingSetting);

	/**
	 * @brief	Reply Save Object List Request
	 *
	 * @param	requestId	Request ID
	 * @param	connectionId	Connection ID
	 * @param	fileId	File ID
	 */
	void replySaveObjectList(uint32_t rquestId, uint32_t connectionId,
			uint32_t fileId);

	/**
	 * @brief	Reply With Folder Data
	 *
	 * @param	requestId	Request ID
	 * @param	connectionId	Connection ID
	 * @param	path	Path to the folder
	 * @param	folderData	Folder Data
	 */
	void replyFolderData(uint32_t requestId, uint32_t connectionId, string path,
			vector<FileMetaData> folderData);

	/**
	 * @brief	Reply Current Primary of an object
	 *
	 * @param	requestId	Request ID
	 * @param	connectionId	Connection ID
	 * @param	objectId	ID of the Object
	 * @oarm	osdId		ID of the Primary Osd
	 */
	void replyPrimary(uint32_t requestId, uint32_t connectionId,
			uint64_t objectId, uint32_t osdId);

	/**
	 * @brief	Reply the Recovery Information (Object List and Associated Node List
	 * 
	 * @param	requestId	Request ID
	 * @param	connectionId	Connection ID
	 * @param	osdId		ID of the Osd to be Recovered
	 * @param	objectLocationList	List of the Location of Object
	 */

	void replyRecoveryTrigger(uint32_t requestId, uint32_t connectionId,
			vector<ObjectLocation> objectLocationList);

	// Request to Other Nodes

	/**
	 * @brief	Report Failure to Monitor
	 * 
	 * @param	osdId	ID of the Failed Osd
	 * @param	reason	Reason of the Failure
	 */
	void reportFailure(uint32_t osdId, FailureReason reason);

	/**
	 * @brief	Ask Monitor for Primary List
	 *
	 * @param	numOfObjs	Number of Objects
	 */
	vector<uint32_t> askPrimaryList(uint32_t numOfObjs);

	vector<uint32_t> getPrimaryList(uint32_t sockfd, uint32_t numOfObjs);

	/**
	 * Request the status of OSD from MONITOR
	 * @param osdIdList List of OSD ID to request
	 * @return boolean array storing OSD health
	 */

	vector<bool> getOsdStatusRequest(vector<uint32_t> osdIdList);

private:
};
#endif
