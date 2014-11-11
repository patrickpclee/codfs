/**
 * downloadfilereply.hh
 */

#ifndef __DOWNLOAD_FILE_REPLY_HH__
#define __DOWNLOAD_FILE_REPLY_HH__

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

class DownloadFileReplyMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	DownloadFileReplyMsg(Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	requestId	Request ID
	 * @param 	sockfd		Source Socket Descriptor
	 * @param	fileId		File ID
	 * @param	filePath	File Path
	 * @param	fileSize	Size of the File
	 * @param	fileType	File Type
	 * @param 	segmentList	List of segments of the file
	 * @param	primaryList	List of primary for storing the segment
	 */

	DownloadFileReplyMsg(Communicator* communicator, 
			uint32_t requestId, uint32_t sockfd, uint32_t fileId, const string &filePath,
			uint64_t fileSize, const FileType& fileType,
			const vector<uint64_t> &segmentList, const vector<uint32_t> &primaryList);


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
//	uint32_t _clientId;
	uint32_t _fileId;
	string _filePath;
	uint64_t _fileSize;
	FileType _fileType;
	vector<uint64_t> _segmentList;
	vector<uint32_t> _primaryList;
};

#endif
