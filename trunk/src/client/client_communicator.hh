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
	 * Upload an object to OSD (blocking)
	 * @param clientId Client ID
	 * @param objectId Object ID
	 * @param path Path to local file
	 * @param offset Offset of the object in file
	 * @param length Size of the object
	 */

	void uploadObject(uint32_t clientId, uint64_t objectId, string path,
			uint64_t offset, uint32_t length);

	void connectToMds();
	void connectToOsd();
private:
};
#endif
