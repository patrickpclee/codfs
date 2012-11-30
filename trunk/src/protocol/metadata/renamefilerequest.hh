#ifndef __RENAME_FILE_REQUEST_HH__
#define __RENAME_FILE_REQUEST_HH__

#include "../message.hh"
#include "../../common/enums.hh"

using namespace std;

/**
 * Extends the Message class
 */

class RenameFileRequestMsg: public Message {
public:

	RenameFileRequestMsg(Communicator* communicator);

	RenameFileRequestMsg(Communicator* communicator, uint32_t sockfd, uint32_t clientId, uint32_t fileId, const string& path, const string& newPath);

	/**
	 * Copy values in private variables to protocol message
	 * Serialize protocol message and copy to private variable
	 */

	void prepareProtocolMsg();

	/**
	 * Override
	 * Parse message from raw buffer
	 * @param buf Raw buffer storing header + protocol + payload
	 */

	void parse(char* buf);

	/**
	 * Override
	 * Execute the corresponding Processor
	 */

	void doHandle();

	/**
	 * Override
	 * DEBUG: print protocol message
	 */

	void printProtocol();

private:
	uint32_t _clientId;
	uint32_t _fileId;
	string _path;
	string _newPath;
};

#endif
