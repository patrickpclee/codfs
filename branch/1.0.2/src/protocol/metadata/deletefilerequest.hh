#ifndef __DELETE_FILE_REQUEST_HH__
#define __DELETE_FILE_REQUEST_HH__

#include <string>

#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Requset to upload file
 */

class DeleteFileRequestMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	DeleteFileRequestMsg(Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	mdsSockfd	Socket descriptor of MDS
	 * @param	clientId	Client ID
	 * @param	fileId	File ID
	 * @param	path	File Path
	 */

	DeleteFileRequestMsg(Communicator* communicator, uint32_t mdsSockfd,
			uint32_t clientId, uint32_t fileId, const string &path);

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
};

#endif
