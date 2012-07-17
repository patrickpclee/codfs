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
	uint32_t uploadFileHandler (uint32_t clientId, string dstPath, uint32_t numOfObjs);

	/**
	 * @brief	Handle Upload Object Acknowledgement from Primary
	 *
	 * @param	osdId		ID of the OSD which the Acknowledgement originated
	 * @param	fileId		ID of the File which the object associated with
	 * @param	objectId	ID of the object uploaded
	 * @param	osdIdList	List of the OSD
	 */
	void uploadObjectAckHandler (uint32_t osdId, uint32_t fileId, uint64_t objectId, vector<uint32_t> osdIdList);

	/**
	 * @brief	Handle Download File Request from Client (Request with Path)
	 *
	 * @param	clientId	ID of the client Requesting
	 * @param	dstPath		Path of the file
	 */
	void downloadFileHandler (uint32_t clientId, string dstPath);

	/**
	 * @brief	Handle Download File Request from Client (Request with File ID)
	 *
	 * @param	clientId	ID of the client Requesting
	 * @param	fileId		ID of the file
	 */
	void downloadFileHandler (uint32_t clientId, uint32_t fileId);

	/**
	 * @brief	Handle the Secondary Node List Request from OSDs
	 *
	 * @param	osdId	ID of the OSD Requesting
	 * @param	objectID	ID of the Object
	 */
	void secondaryNodeListHandler (uint32_t clientId, uint64_t objectId);

	uint32_t listFolderHandler (string path);

	/**
	 * @brief	Handle Primary Node Failure Report from Client
	 *
	 * @param	clientId	ID of the Client Reporting
	 * @param	osdId		ID of the Failed OSD
	 * @param	objectId	ID of the Failed Object
	 * @param	reason		Reason of the Failure (Default to Node Failure)
	 */
	void primaryFailureHandler (uint32_t clientId, uint32_t osdId, uint64_t objectId, FailureReason reason=UNREACHABLE);
	uint32_t secondaryFailureHandler (uint32_t osdId);

	uint32_t osdObjectListHandler (uint32_t osdId);

	uint32_t nodeListUpdateHandler (uint64_t objectId, uint32_t osdIdList[]);
private:
	vector<uint64_t> newObjectList (uint32_t numOfObjs);
	// Ask Monitor for Primary Node List
	vector<uint32_t> askPrimaryList (uint32_t numOfObjs);

	/**
	 * @brief	Process the Download Request
	 *
	 * @param	clientId	ID of the client
	 * @param	fileId		ID of the File
	 * @param	path		Path of the File
	 */
	void downloadFileProcess (uint32_t clientId, uint32_t fileId, string path);

	MdsCommunicator* _mdsCommunicator;
	MetaDataModule* _metaDataModule;
	NameSpaceModule* _nameSpaceModule;
};
#endif
