/**
 * downloadfilerequest.hh
 */

#ifndef __I_WANT_FILE_SIZE_HH__
#define __I_WANT_FILE_SIZE_HH__

#include <string>
#include <vector>
#include <future>

#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Requset to upload file
 */

class IWantFileSizeMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	IWantFileSizeMsg(Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	clientId	Client ID
	 * @param	fileId		File ID
	 */

	IWantFileSizeMsg(Communicator* communicator, uint32_t mdsSockfd, uint32_t clientId, uint32_t fileId);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	clientId	Client ID
	 * @param	filePath	File Path
	 */

	IWantFileSizeMsg(Communicator* communicator, uint32_t mdsSockfd,
			uint32_t clientId, const string &filePath);

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

	void setSize (uint64_t size);
	uint32_t getSize();

private:
	uint32_t _fileId;
	uint64_t _size;
};

#endif
