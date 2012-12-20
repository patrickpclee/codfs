/**
 * uploadfilerequest.hh
 */

#ifndef __UPLOAD_FILE_REQUEST_HH__
#define __UPLOAD_FILE_REQUEST_HH__

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

class UploadFileRequestMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	UploadFileRequestMsg(Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	mdsSockfd	Socket descriptor of MDS
	 * @param	clientId	Client ID
	 * @param	path	Requested directory path
	 * @param	fileSize	Size of the File
	 * @param	numOfObjs	Number of Segments
	 */

	UploadFileRequestMsg(Communicator* communicator, uint32_t mdsSockfd,
			uint32_t clientId, const string &path, uint64_t fileSize,
			uint32_t numOfObjs);

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

	/**
	 * @brief	Set the Segment ID List
	 *
	 * @param	segmentList	Vector of Segment ID
	 */
	void setSegmentList(vector<uint64_t> segmentList);
	void setPrimaryList(vector<uint32_t> primaryList);
	void setFileId(uint32_t fileId);

	vector<uint64_t> getSegmentList();
	vector<uint32_t> getPrimaryList();
	uint32_t getFileId();

private:
	uint32_t _clientId;
	string _path;
	uint64_t _fileSize;
	uint32_t _numOfObjs;

	vector<uint64_t> _segmentList;
	vector<uint32_t> _primaryList;
	uint32_t _fileId;
};

#endif
