/**
 * listdirectoryrequest.hh
 */

#ifndef __LISTDIRECTORYREQUESTHH__
#define __LISTDIRECTORYREQUESTHH__

#include <string>
#include <vector>

#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Request to list files in a directory from MDS
 */

class ListDirectoryRequestMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	ListDirectoryRequestMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	clientId	Client ID
	 * @param	mdsSockfd	Socket descriptor of MDS
	 * @param	path	Requested directory path
	 */

	ListDirectoryRequestMsg (Communicator* communicator, uint32_t clientId, uint32_t mdsSockfd, const string &path);

	/**
	 * Copy values in private variables to protocol message
	 * Serialize protocol message and copy to private variable
	 */

	void prepareProtocolMsg ();

	/**
	 * Override
	 * Parse message from raw buffer
	 * @param buf Raw buffer storing header + protocol + payload
	 */

	void parse (char* buf);

	/**
	 * Override
	 * Execute the corresponding Processor
	 */

	void doHandle ();

	/**
	 * Override
	 * DEBUG: print protocol message
	 */

	void printProtocol ();

	/**
	 * @brief	Set the Folder Data
	 *
	 * @param	folderData	Vector of File Meta Data
	 */
	void setFolderData (vector<FileMetaData> folderData);

	vector<FileMetaData> getFolderData ();

private:
	vector<FileMetaData> _folderData;
	uint32_t _clientId;
//	uint32_t _osdId;
	string _directoryPath;
};

#endif
