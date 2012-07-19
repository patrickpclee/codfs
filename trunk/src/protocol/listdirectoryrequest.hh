/**
 * listdirectoryrequest.hh
 */

#ifndef __LISTDIRECTORYREQUESTHH__
#define __LISTDIRECTORYREQUESTHH__

#include <string>
#include "../common/enums.hh"
#include "message.hh"

using namespace std;

/**
 * Extends the Message class
 * Request to list files in a directory from MDS
 */

class ListDirectoryRequestMsg: public Message {
public:

	/**
	 * Default Constructor
	 */

	ListDirectoryRequestMsg();

	/**
	 * Constructor - Save parameters in private variables
	 * @param osdId My OSD ID
	 * @param directoryPath Requested directory path
	 * @param mdsSockfd Socket descriptor of MDS
	 */

	ListDirectoryRequestMsg(uint32_t osdId, string directoryPath,
			uint32_t mdsSockfd);

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

	void handle();

	/**
	 * Override
	 * DEBUG: print protocol message
	 */

	void printProtocol();

private:
	uint32_t _osdId;
	string _directoryPath;
};

#endif
