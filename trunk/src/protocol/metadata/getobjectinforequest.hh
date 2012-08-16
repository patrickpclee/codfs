#ifndef __GET_OBJECT_INFO_REQUEST_HH__
#define __GET_OBJECT_INFO_REQUEST_HH__

#include "../message.hh"
#include "../../common/enums.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an object upload
 */

class GetObjectInfoRequestMsg: public Message {
public:

	GetObjectInfoRequestMsg(Communicator* communicator);

	GetObjectInfoRequestMsg(Communicator* communicator, uint32_t dstSockfd,
			uint64_t objectId);

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

	vector<uint32_t> getNodeList();
	void setNodeList(vector<uint32_t> nodeList);
	CodingScheme getCodingScheme();
	void setCodingScheme(CodingScheme codingScheme);
	string getCodingSetting();
	void setCodingSetting(string codingSetting);

private:
	uint64_t _objectId;

	vector<uint32_t> _nodeList;
	CodingScheme _codingScheme;
	string _codingSetting;
};

#endif
