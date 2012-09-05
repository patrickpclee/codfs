#ifndef __UPLOAD_OBJECT_ACK_HH__
#define __UPLOAD_OBJECT_ACK_HH__

#include <string>
#include <vector>

#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Requset to upload file
 */

class UploadObjectAckMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	UploadObjectAckMsg(Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */
	UploadObjectAckMsg(Communicator* communicator, uint32_t sockfd,
			uint64_t objectId, CodingScheme codingScheme, string codingSetting,
			vector<uint32_t> nodeList, string checksum);

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
	CodingScheme _codingScheme;
	string _codingSetting;
	vector<uint32_t> _nodeList;
	string _checksum;
};

#endif
