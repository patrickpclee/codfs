/**
 * uploadfilerequest.cc
 */

#include <iostream>
#include "uploadfilerequest.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../mds/mds.hh"

#ifdef COMPILE_FOR_MDS
extern Mds* mds;
#endif

/**
 * Default Constructor
 */

UploadFileRequestMsg::UploadFileRequestMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
UploadFileRequestMsg::UploadFileRequestMsg(Communicator* communicator,
		uint32_t clientId, uint32_t mdsSockfd, string path) :
		Message(communicator) {
	_clientId = clientId;
	_path = path;
	_sockfd = mdsSockfd;
}

void UploadFileRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::UploadFileRequestPro uploadFileRequestPro;
	//uploadFileRequestPro.set_directorypath(_directoryPath);
	//uploadFileRequestPro.set_osdid(_clientId);

	if (!uploadFileRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(LIST_DIRECTORY_REQUEST);
	setProtocolMsg(serializedString);

}

void UploadFileRequestMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::UploadFileRequestPro uploadFileRequestPro;
	uploadFileRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	//_clientId = uploadFileRequestPro.osdid();
	//_directoryPath = uploadFileRequestPro.directorypath();

}

void UploadFileRequestMsg::handle() {
#ifdef COMPILE_FOR_MDS
	//mds->listFolderProcessor(_msgHeader.requestId,_sockfd,_clientId,_directoryPath);
#endif
}

void UploadFileRequestMsg::printProtocol() {
	cout << "[UPLOAD_FILE_REQUEST] client ID = " << _clientId << " Path = "
			<< _path << endl;
}

/**
 * @brief	Get the Future of the Folder Data
 *
 * @return	Future of the Folder Data
 */
//future<vector<FileMetaData> > UploadFileRequestMsg::getFolderDataFuture() {
//	return _folderData.get_future();
//}

future< vector<uint64_t> > UploadFileRequestMsg::getObjectIdListFuture () {
	return _objectIdList.get_future();
}

future< vector<uint32_t> > UploadFileRequestMsg::getPrimaryListFuture () {
	return _primaryList.get_future();
}
future< uint32_t > UploadFileRequestMsg::getFileIdFuture ()
{
	return _fileId.get_future();
}

/**
 * @brief	Set the Folder Data (Fulfill Promise)
 */
//void UploadFileRequestMsg::setFolderDataValue(
//		vector<FileMetaData> folderData) {
//	_folderData.set_value(folderData);
//
//	return;
//}
void UploadFileRequestMsg::setObjectIdListValue (vector<uint64_t> objectIdList) {
	_objectIdList.set_value(objectIdList);

	return ;
}

void UploadFileRequestMsg::setPrimaryListValue (vector<uint32_t> primaryList)
{
	_primaryList.set_value(primaryList);

	return ;
}

void UploadFileRequestMsg::setfileIdValue (uint32_t fileId)
{
	_fileId.set_value(fileId);

	return ;
}
