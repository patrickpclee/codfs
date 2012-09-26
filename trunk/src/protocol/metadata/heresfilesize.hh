

#ifndef __HERE_S_FILE_SIZE_HH__
#define __HERE_S_FILE_SIZE_HH__

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

class HeresFileSizeMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	HeresFileSizeMsg(Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	requestId	Request ID
	 * @param 	sockfd		Source Socket Descriptor
	 * @param	fileId		File ID
	 * @param	filePath	File Path
	 * @param	fileSize	Size of the File
	 * @param	checksum	Checksum of the File
	 * @param 	objectList	List of objects of the file
	 * @param	primaryList	List of primary for storing the object
	 */

	HeresFileSizeMsg(Communicator* communicator,
			uint32_t requestId, uint32_t sockfd, uint32_t fileId, uint64_t fileSize);


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
	uint32_t _fileId;
	uint64_t _fileSize;
};

#endif
