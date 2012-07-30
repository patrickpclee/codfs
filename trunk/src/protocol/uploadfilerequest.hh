/**
 * uploadfilerequest.hh
 */

#ifndef __UPLOAD_FILE_REQUEST_HH__
#define __UPLOAD_FILE_REQUEST_HH__

#include <string>
#include <vector>
#include <future>

#include "message.hh"

#include "../common/enums.hh"
#include "../common/metadata.hh"

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

	UploadFileRequestMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	clientId	Client ID
	 * @param	mdsSockfd	Socket descriptor of MDS
	 * @param	path	Requested directory path
	 */

	UploadFileRequestMsg (Communicator* communicator, uint32_t clientId, uint32_t mdsSockfd, string path);

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

	void handle ();

	/**
	 * Override
	 * DEBUG: print protocol message
	 */

	void printProtocol ();

	/**
	 * @brief	Get the Future of the Folder Data
	 *
	 * @return	Future of the Folder Data
	 */
	//future< vector<FileMetaData> > getFolderDataFuture();
	
	future< vector<uint64_t> > getObjectIdListFuture ();
	future< vector<uint32_t> > getPrimaryListFuture ();
	future< uint32_t > getFileIdFuture ();


	/**
	 * @brief	Set the Object ID List (Fulfill Promise)
	 *
	 * @param	objectList	Vector of Object ID
	 */
	void setObjectIdListValue (vector<uint64_t> objectIdList);

	void setPrimaryListValue (vector<uint32_t> primaryList);
	void setfileIdValue (uint32_t fileId);

private:
	uint32_t _clientId;
	string _path;

	promise< vector<uint64_t> > _objectIdList;
	promise< vector<uint32_t> > _primaryList;
	promise< uint32_t > _fileId;
};

#endif
