#ifndef __REPORT_DELETED_CACHE_HH__
#define __REPORT_DELETED_CACHE_HH__

#include "../message.hh"
#include "../../common/enums.hh"

using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class ReportDeletedCacheMsg: public Message {
public:

	ReportDeletedCacheMsg(Communicator* communicator);

	ReportDeletedCacheMsg(Communicator* communicator, uint32_t dstSockfd,
			list<uint64_t> segmentId, uint32_t osdId);

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
	list<uint64_t> _segmentIdList;
	uint32_t _osdId;
};

#endif
