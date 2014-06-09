#ifndef __GET_SEGMENT_INFO_REQUEST_HH__
#define __GET_SEGMENT_INFO_REQUEST_HH__

#include "../message.hh"
#include "../../common/enums.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class GetSegmentInfoRequestMsg: public Message {
public:

	GetSegmentInfoRequestMsg(Communicator* communicator);

	GetSegmentInfoRequestMsg(Communicator* communicator, uint32_t dstSockfd,
			uint64_t segmentId, uint32_t osdId, bool needReply = true, bool isRecovery = false);

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

	uint64_t getSegmentSize();
	void setSegmentSize(uint32_t segmentSize);
	vector<uint32_t> getNodeList();
	void setNodeList(vector<uint32_t> nodeList);
	CodingScheme getCodingScheme();
	void setCodingScheme(CodingScheme codingScheme);
	string getCodingSetting();
	void setCodingSetting(string codingSetting);
	string getChecksum();


private:
	uint64_t _segmentId;
	uint32_t _osdId;
	bool _needReply;

	// reply
	uint32_t _segmentSize;
	vector<uint32_t> _nodeList;
	CodingScheme _codingScheme;
	string _codingSetting;
	bool _isRecovery;
};

#endif
