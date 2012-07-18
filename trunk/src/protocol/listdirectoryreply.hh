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

class ListDirectoryReplyMessage : public Message {
public:
	/**
	 * @brief	Constructor - Save Parameters in Private Variables
	 *
	 * @param	requestId	Request ID
	 * @param	connectionId	connection ID
	 * @param	path	Path to the Folder
	 * @param	folderData	Folder Data
	 */
	ListDirectoryReplyMessage(uint32_t requestId, uint32_t connectionId, string path, vector<FileMetaData> folderData);

	void prepareProtocolMsg();
	void printProtocol();
private:
	uint32_t _requestId;
	uint32_t _connectiondId;
	string _path;
	vector<FileMetaData> _folderData;
};

#endif
