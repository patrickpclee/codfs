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
	 * @param	clientId	ID of the client
	 * @param	dstPath		Target Path for the file uploaded
	 * @param	numOfObjs	number of objects to be uploaded
	 *
	 * @return	File ID		File ID
	 */
	uint32_t uploadFileProcessor (uint32_t requestId, uint32_t connectionId, uint32_t clientId, string dstPath, uint32_t numOfObjs);

	/**
	 * @brief	Handle Upload Object Acknowledgement from Primary
	 *
	 * @param	osdId		ID of the OSD which the Acknowledgement originated
	 * @param	fileId		ID of the File which the object associated with
	 * @param	objectId	ID of the object uploaded
	 * @param	objectNodeList	List of the OSD
	 */
	void uploadObjectAckProcessor (uint32_t requestId, uint32_t connectionId, uint32_t osdId, uint32_t fileId, uint64_t objectId, vector<uint32_t> objectNodeList);

	/**
	 * @brief	Handle Download File Request from Client (uint32_t requestId, uint32_t connectionId, Request with Path)
	 *
	 * @param	clientId	ID of the client Requesting
	 * @param	dstPath		Path of the file
	 */
	void downloadFileProcessor (uint32_t requestId, uint32_t connectionId, uint32_t clientId, string dstPath);

	/**
	 * @brief	Handle Download File Request from Client (uint32_t requestId, uint32_t connectionId, Request with File ID)
	 *
	 * @param	clientId	ID of the client Requesting
	 * @param	fileId		ID of the file
	 */
	void downloadFileProcessor (uint32_t requestId, uint32_t connectionId, uint32_t clientId, uint32_t fileId);

	/**
	 * @brief	Handle the Secondary Node List Request from OSDs
	 *
	 * @param	osdId	ID of the OSD Requesting
	 * @param	objectID	ID of the Object
	 */
	void secondaryNodeListProcessor (uint32_t requestId, uint32_t connectionId, uint32_t clientId, uint64_t objectId);

	uint32_t listFolderProcessor (uint32_t requestId, uint32_t connectionId, string path);

	/**
	 * @brief	Handle Primary Node Failure Report from Client
	 *
	 * @param	clientId	ID of the Client Reporting
	 * @param	osdId		ID of the Failed OSD
	 * @param	objectId	ID of the Failed Object
	 * @param	reason		Reason of the Failure (uint32_t requestId, uint32_t connectionId, Default to Node Failure)
	 */
	void primaryFailureProcessor (uint32_t requestId, uint32_t connectionId, uint32_t clientId, uint32_t osdId, uint64_t objectId, FailureReason reason=UNREACHABLE);

	/**
	 * @brief	Handle Secondary Node Failure Report from OSD
	 *
	 * @param	osdId		ID of the Failed OSD
	 * @param	objectId	ID of the Failed Object
	 * @param	reason		Reason of the Failure (uint32_t requestId, uint32_t connectionId, Default to Node Failure)
	 */
	void secondaryFailureProcessor(uint32_t requestId, uint32_t connectionId, uint32_t osdId, uint64_t objectId, FailureReason reason=UNREACHABLE);

	/**
	 * @brief	Handle OSD Recovery Initialized by Monitor
	 *
	 * @param	monitorId	ID of the Monitor
	 * @param	osdId		ID of the failed OSD
	 */
	void recoveryProcessor(uint32_t requestId, uint32_t connectionId, uint32_t monitorId, uint32_t osdId);

	/**
	 * @brief	Handle Object Node List Update from OSD
	 *
	 * @param	objectId	ID of the Object
	 * @param	objectNodeList	Node List of the Object
	 */
	void nodeListUpdateProcessor (uint32_t requestId, uint32_t connectionId, uint64_t objectId, vector<uint32_t> objectNodeList);
private:
	vector<uint64_t> newObjectList (uint32_t requestId, uint32_t connectionId, uint32_t numOfObjs);

	/**
	 * @brief	Process the Download Request
	 *
	 * @param	clientId	ID of the client
	 * @param	fileId		ID of the File
	 * @param	path		Path of the File
	 */
	void downloadFileProcess (uint32_t requestId, uint32_t connectionId, uint32_t clientId, uint32_t fileId, string path);

	/// Handle Communication with other components
	MdsCommunicator* _mdsCommunicator;

	/// Handle Metadata Operations
	MetaDataModule* _metaDataModule;

	/// Handle Namespace Operations
	NameSpaceModule* _nameSpaceModule;
};
#endif
