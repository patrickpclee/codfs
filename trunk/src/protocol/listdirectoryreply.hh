/**
 * listdirectoryreply.hh
 */

#ifndef __LIST_DIRECTORY_REPLY_HH__
#define __LIST_DIRECTORY_REPLY_HH__

#include <string>
#include <vector>

#include "message.hh"

#include "../common/enums.hh"
#include "../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Reply to List Folder
 */

class ListDirectoryReplyMsg : public Message {
public:

	/**
	 * @brief	Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */
	ListDirectoryReplyMsg (Communicator* communicator);

	/**
	 * @brief	Constructor - Save Parameters in Private Variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	requestId	Request ID
	 * @param	connectionId	connection ID
	 * @param	path	Path to the Folder
	 * @param	folderData	Folder Data
	 */
	ListDirectoryReplyMsg (Communicator* communicator, uint32_t requestId, uint32_t sockfd, string path, vector<FileMetaData> folderData);

	/**
	 * @brief	Save Parameters in Private Variables
	 *
	 * @param	requestId	Request ID
	 * @param	connectionId	connection ID
	 * @param	path	Path to the Folder
	 * @param	folderData	Folder Data
	 */
	void setVariables (uint32_t requestId, uint32_t sockfd, string path, vector<FileMetaData> folderData);

	/**
	 * @brief	Copy values in private variables to protocol message
	 * Serialize protocol message and copy to private variable
	 */
	void prepareProtocolMsg ();

	void printProtocol ();

	/**
	 * @brief	Parse the Binary to Variables
	 *
	 * @param	buf	Message Binary
	 */
	void parse (char* buf);

	void handle ();
private:
	string _path;
	vector<FileMetaData> _folderData;
};

#endif
