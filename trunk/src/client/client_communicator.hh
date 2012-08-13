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

	struct FileMetaData uploadFile(uint32_t clientId, string path, uint64_t fileSize, uint32_t numOfObjs);	

	/**
	 * 1. Send an init message
	 * 2. Repeatedly send data chunks to OSD
	 * 3. Send an end message
	 * @param clientID Client ID
	 * @param dstOsdSockfd Destination OSD Socket Descriptor
	 * @param objectData ObjectData Structure
	 */

	void putObject (uint32_t clientId, uint32_t dstOsdSockfd, struct ObjectData objectData);

	// TODO: CONNECT TO COMPONENT: PRIMITIVE DESIGN
	void connectToMds();
	void connectToOsd();
private:

	/**
	 * Initiate upload process to OSD (Step 1)
	 * @param clientId Client ID
	 * @param dstOsdSockfd Destination OSD Socket Descriptor
	 * @param objectId Object ID
	 * @param length Size of the object
	 * @param chunkCount Number of chunks that will be sent
	 */

	void putObjectInit(uint32_t clientId, uint32_t dstOsdSockfd, uint64_t objectId,
			uint32_t length, uint32_t chunkCount);

	/**
	 * Send an object chunk to OSD (Step 2)
	 * @param clientId Client ID
	 * @param dstOsdSockfd Destination OSD Socket Descriptor
	 * @param objectId Object ID
	 * @param buf Buffer containing the object
	 * @param offset Offset of the chunk inside the buffer
	 * @param length Length of the chunk
	 */

	void putObjectData(uint32_t clientID, uint32_t dstOsdSockfd, uint64_t objectId,
			char* buf, uint64_t offset, uint32_t length);

	/**
	 * Finalise upload process to OSD (Step 3)
	 * @param clientId Client ID
	 * @param dstOsdSockfd Destination OSD Socket Descriptor
	 * @param objectId Object ID
	 */

	void putObjectEnd(uint32_t clientId, uint32_t dstOsdSockfd, uint64_t objectId);

};
#endif
