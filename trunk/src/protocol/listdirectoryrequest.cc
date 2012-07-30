/**
 * listdirectoryrequest.cc
 */

#include <iostream>
#include "listdirectoryrequest.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"
#include "../mds/mds.hh"

#ifdef COMPILE_FOR_MDS
extern Mds* mds;
#endif

/**
 * Default Constructor
 */

ListDirectoryRequestMsg::ListDirectoryRequestMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
ListDirectoryRequestMsg::ListDirectoryRequestMsg(Communicator* communicator,
		uint32_t clientId, uint32_t mdsSockfd, string path) :
		Message(communicator) {
	_clientId = clientId;
	_directoryPath = path;
	_sockfd = mdsSockfd;
}

void ListDirectoryRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::ListDirectoryRequestPro listDirectoryRequestPro;
	listDirectoryRequestPro.set_directorypath(_directoryPath);
	listDirectoryRequestPro.set_osdid(_clientId);

	if (!listDirectoryRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(LIST_DIRECTORY_REQUEST);
	setProtocolMsg(serializedString);

}

void ListDirectoryRequestMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::ListDirectoryRequestPro listDirectoryRequestPro;
	listDirectoryRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_clientId = listDirectoryRequestPro.osdid();
	_directoryPath = listDirectoryRequestPro.directorypath();

}

void ListDirectoryRequestMsg::handle() {
#ifdef COMPILE_FOR_MDS
	mds->listFolderProcessor(_msgHeader.requestId,_sockfd,_clientId,_directoryPath);
#endif
}

void ListDirectoryRequestMsg::printProtocol() {
	cout << "[LIST_DIRECTORY_REQUEST] osdID = " << _clientId << " Path = "
			<< _directoryPath << endl;
}

/**
 * @brief	Get the Future of the Folder Data
 *
 * @return	Future of the Folder Data
 */
future<vector<FileMetaData> > ListDirectoryRequestMsg::getFolderDataFuture() {
	return _folderData.get_future();
}

/**
 * @brief	Set the Folder Data (Fulfill Promise)
 */
void ListDirectoryRequestMsg::setFolderDataValue(
		vector<FileMetaData> folderData) {
	_folderData.set_value(folderData);

	return;
}
