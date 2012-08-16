#ifndef __GET_OBJECT_INFO_REPLY_HH__
#define __GET_OBJECT_INFO_REPLY_HH__

#include "../message.hh"
#include "../../common/enums.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an object upload
 */

class GetObjectInfoReplyMsg: public Message {
public:

	GetObjectInfoReplyMsg(Communicator* communicator);

	GetObjectInfoReplyMsg(Communicator* communicator, uint32_t requestId,
			uint32_t dstSockfd, uint64_t objectId, vector<uint32_t> nodeList,
			CodingScheme codingScheme, string codingSetting);

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
	uint64_t _objectId;

	vector<uint32_t> _nodeList;
	CodingScheme _codingScheme;
	string _codingSetting;
};

#endif
