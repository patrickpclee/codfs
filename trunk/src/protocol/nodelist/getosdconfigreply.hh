/**
 * getosdconfigreply.hh
 */

#ifndef __GET_OSD_CONFIG_REPLY_HH__
#define __GET_OSD_CONFIG_REPLY_HH__

#include <vector>
#include <string>
#include "../message.hh"

#include "../../common/enums.hh"

using namespace std;

/**
 * Extends the Message class
 * Request to list files in a directory from MDS
 */

class GetOsdConfigReplyMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	GetOsdConfigReplyMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	numOfObjs	number of Objects
	 * @param	mdsSockfd	Socket descriptor
	 */

	GetOsdConfigReplyMsg (Communicator* communicator, uint32_t requestId, uint32_t osdSockfd, uint32_t osdId, uint16_t port,
			uint32_t segmentCapacity, uint32_t objectCache, string segmentFolder, string objectCacheFolder);

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


private:
	uint32_t _osdId;
	uint32_t _servePort;
	uint32_t _segmentCapacity;
	uint32_t _objectCacheCapacity;
	string _segmentFolder;
	string _objectCacheFolder;
};

#endif
