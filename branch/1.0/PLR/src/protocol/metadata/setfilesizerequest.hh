#ifndef __SET_FILE_SIZE_REQUEST_HH__
#define __SET_FILE_SIZE_REQUEST_HH__

#include "../message.hh"
#include "../../common/enums.hh"

using namespace std;

/**
 * Extends the Message class
 */

class SetFileSizeRequestMsg: public Message {
public:

	SetFileSizeRequestMsg(Communicator* communicator);

	SetFileSizeRequestMsg(Communicator* communicator, uint32_t sockfd, uint32_t clientId, uint32_t fileId, uint64_t fileSize);

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
	uint64_t _fileSize;
};

#endif
