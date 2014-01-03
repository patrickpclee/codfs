/**
 * uploadfilereply.hh
 */

#ifndef __UPLOAD_FILE_REPLY_HH__
#define __UPLOAD_FILE_REPLY_HH__

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

class UploadFileReplyMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	UploadFileReplyMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	mdsSockfd	Socket descriptor of MDS
	 */
	UploadFileReplyMsg (Communicator* communicator, uint32_t requestId, uint32_t sockfd, uint32_t fileId, const vector<uint64_t> &segmentList, const vector<uint32_t> &primaryList);

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
	uint32_t _fileId;
	vector<uint64_t> _segmentList;
	vector<uint32_t> _primaryList;
};

#endif
