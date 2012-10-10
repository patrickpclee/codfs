/**
 * mds.hh
 */

#ifndef __MDS_HH__
#define __MDS_HH__

#include "metadatamodule.hh"
#include "namespacemodule.hh"
#include "mds_communicator.hh"

#include <stdint.h>
#include <string.h>
#include <vector>

using namespace std;

class Mds {
public:
	/**
	 * @brief	MDS Constructor
	 */
	Mds();

	~Mds();

	/**
	 * @brief	Handle File Upload Request From Client
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the client
	 * @param	dstPath		Target Path for the file uploaded
	 * @param	fileSize	Size of the File
	 * @param	numOfObjs	number of objects to be uploaded
	 * @param	codingScheme	Coding Scheme for the file
	 * @param 	codingSetting	Coding Scheme Setting
	 *
	 * @return	File ID
	 */
	uint32_t uploadFileProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, const string &dstPath, uint64_t fileSize,
			uint32_t numOfObjs, CodingScheme codingScheme, const string &codingSetting);

//	void FileSizeProcessor(uint32_t requestId, uint32_t connectionId, uint32_t fileId);


	/**
	 * @brief	Handle Upload Object Acknowledgement from Primary
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	objectId	ID of the object uploaded
	 * @param 	objectSize	Size of object
	 * @param	codingScheme	Coding Scheme
	 * @param 	codingSetting	Coding Scheme Setting
	 * @param	objectNodeList	List of the Osd
	 * @param	checksum	Checksum
	 */
	void uploadObjectAckProcessor(uint32_t requestId, uint32_t connectionId,
			uint64_t objectId, uint32_t objectSize, CodingScheme codingScheme, const string &codingSetting,
			const vector<uint32_t> &objectNodeList, const string &checksum);

	/**
	 * @brief	Handle Download File Request from Client (Request with Path)
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the client Requesting
	 * @param	dstPath		Path of the file
	 */
	void downloadFileProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, const string &dstPath);

	/**
	 * @brief	Handle Download File Request from Client (Request with File ID)
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the Client
	 * @param	fileId		ID of the file
	 */
	void downloadFileProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, uint32_t fileId);

	/**
	 * @brief	Handle Get Object ID Lsit
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the Client
	 * @param	numOfObjs	Number of Objects
	 */
	void getObjectIdListProcessor(uint32_t requestId, uint32_t connectionId, uint32_t clientId, uint32_t numOfObjs);

	/**
	 * @brief	Handle Get File Info Request
	 *
	 * @param	requestId	Request ID
	 * @param	connectonId	Connection ID
	 * @param	clientId	ID of the Client
	 * @param	path	Path of the File
	 */
	void getFileInfoProcessor(uint32_t requestId, uint32_t connectionId, uint32_t clientId, const string &path);

	/**
	 * @brief	Handle the Object Info Request from Osd
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	objectID	ID of the Object
	 */
	void getObjectInfoProcessor(uint32_t requestId, uint32_t connectionId, uint64_t objectId);

	/**
	 * @brief	Handle List Folder Request from Client
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the Client
	 * @param	path	Path to the Folder
	 */
	void listFolderProcessor(uint32_t requestId, uint32_t clientId,
			uint32_t connectionId, const string &path);

	/**
	 * @brief	Handle Primary Node Failure Report from Client
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	osdId		ID of the Failed Osd
	 * @param	objectId	ID of the Failed Object
	 * @param	reason		Reason of the Failure (Default to Node Failure)
	 */
	void primaryFailureProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t osdId, uint64_t objectId, FailureReason reason =
					UNREACHABLE);

	/**
	 * @brief	Handle Secondary Node Failure Report from Osd
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	osdId		ID of the Failed Osd
	 * @param	objectId	ID of the Failed Object
	 * @param	reason		Reason of the Failure (Default to Node Failure)
	 */
	void secondaryFailureProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t osdId, uint64_t objectId, FailureReason reason =
					UNREACHABLE);

	/**
	 * @brief	Handle Osd Recovery Initialized by Monitor
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	osdId		ID of the failed Osd
	 */
	void recoveryProcessor(uint32_t requestId, uint32_t connectionId,
			uint32_t osdId);

	/**
	 * @brief	Handle Object Node List Update from Osd
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	objectId	ID of the Object
	 * @param	objectNodeList	Node List of the Object
	 */
	void nodeListUpdateProcessor(uint32_t requestId, uint32_t connectionId,
			uint64_t objectId, const vector<uint32_t> &objectNodeList);

	/**
	 * @brief	Handle Object List Save Request
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the client Requesting
	 * @param	fileId	ID of the File
	 * @param	objectList	Object List of the File
	 */
	void saveObjectListProcessor(uint32_t requestId, uint32_t connectionId, uint32_t clientId, uint32_t fileId, const vector<uint64_t> &objectList);

	/**
	 * @brief	Handle Set File Size Request
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the client Requesting
	 * @param	fileId	ID of the File
	 * @param	fileSize	Size of the File
	 */
	void setFileSizeProcessor(uint32_t requestId, uint32_t connectionId, uint32_t clientId, uint32_t fileId, uint64_t fileSize);

	/**
	 * @brief	Get the MDS Communicator
	 *
	 * @return	Pointer to the MDS Communicator Module
	 */
	MdsCommunicator* getCommunicator();

	/**
	 * @brief	Run the MDS
	 */
	void run();

	/**
	 * @brief	Test Case
	 */
	void test();

private:

	/**
	 * @brief	Process the Download Request
	 *
	 * @param	requestId	Request ID
	 * @param	conenctionId	Connection ID
	 * @param	clientId	ID of the client
	 * @param	fileId		ID of the File
	 * @param	path		Path of the File
	 */
	void downloadFileProcess(uint32_t requestId, uint32_t connectionId,
			uint32_t clientId, uint32_t fileId, const string &path);

	/// Handle Communication with other components
	MdsCommunicator* _mdsCommunicator;

	/// Handle Metadata Operations
	MetaDataModule* _metaDataModule;

	/// Handle Namespace Operations
	NameSpaceModule* _nameSpaceModule;

	/// Running Indicator
	bool running;
};
#endif
