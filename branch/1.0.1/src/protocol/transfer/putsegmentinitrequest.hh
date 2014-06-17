#ifndef __PUTSEGMENTINITREQUEST_HH__
#define __PUTSEGMENTINITREQUEST_HH__

#include "../../common/enums.hh"
#include "../message.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class PutSegmentInitRequestMsg: public Message {
public:

	PutSegmentInitRequestMsg(Communicator* communicator);

	PutSegmentInitRequestMsg(Communicator* communicator, uint32_t osdSockfd,
			uint64_t segmentId, uint32_t segmentSize, uint32_t bufferSize, 
            uint32_t chunkCount, CodingScheme codingScheme, const string &codingSetting,
			string updateKey = "");

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

	void setDataMsgType (DataMsgType dataMsgType);
	DataMsgType getDataMsgType ();

private:
	uint64_t _segmentId;
	uint32_t _bufferSize;
	uint32_t _segmentSize;
	uint32_t _chunkCount;
	CodingScheme _codingScheme;
	string _codingSetting;
	DataMsgType _dataMsgType;
	string _updateKey;
};

#endif
