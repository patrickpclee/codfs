#ifndef __UPLOAD_SEGMENT_ACK_HH__
#define __UPLOAD_SEGMENT_ACK_HH__

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

class UploadSegmentAckMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	UploadSegmentAckMsg(Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */
	UploadSegmentAckMsg(Communicator* communicator, uint32_t sockfd,
			uint64_t segmentId, uint32_t segmentSize, CodingScheme codingScheme, const string &codingSetting,
			const vector<uint32_t> &nodeList);

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
	uint64_t _segmentId;
	CodingScheme _codingScheme;
	string _codingSetting;
	vector<uint32_t> _nodeList;
	uint32_t _segmentSize;
};

#endif
