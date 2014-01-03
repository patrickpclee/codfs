/**
 * listdirectoryreply.cc
 */

#include <iostream>

#include "listdirectoryreply.hh"
#include "listdirectoryrequest.hh"

#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#include "../../common/debug.hh"

ListDirectoryReplyMsg::ListDirectoryReplyMsg(Communicator* communicator) :
		Message(communicator) {
}

ListDirectoryReplyMsg::ListDirectoryReplyMsg(Communicator* communicator,
		uint32_t requestId, uint32_t sockfd, const string &path,
		const vector<FileMetaData> &folderData) :
		Message(communicator) {
	_sockfd = sockfd;
	_msgHeader.requestId = requestId;
	_path = path;
	_folderData = folderData;
}

/**
 * @brief	Copy values in private variables to protocol message
 * Serialize protocol message and copy to private variable
 */
void ListDirectoryReplyMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::ListDirectoryReplyPro listDirectoryReplyPro;

	vector<FileMetaData>::iterator it;

	for (it = _folderData.begin(); it < _folderData.end(); ++it) {
		ncvfs::FileInfoPro* fileInfoPro =
				listDirectoryReplyPro.add_fileinfopro();
		fileInfoPro->set_fileid((*it)._id);
		fileInfoPro->set_filesize((*it)._size);
		fileInfoPro->set_filename((*it)._path);
		//TODO: Send File Type
	}

	if (!listDirectoryReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(LIST_DIRECTORY_REPLY);
	setProtocolMsg(serializedString);
	return;
}

void ListDirectoryReplyMsg::doHandle() {
	ListDirectoryRequestMsg* listdirectoryrequest =
			(ListDirectoryRequestMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	listdirectoryrequest->setFolderData(_folderData);
	listdirectoryrequest->setStatus(READY);
}

/**
 * @brief	Parse the Binary to Variables
 */
void ListDirectoryReplyMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::ListDirectoryReplyPro listDirectoryReplyPro;
	listDirectoryReplyPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);
//	listDirectoryReplyPro.ParseFromString(buf + sizeof(struct MsgHeader));
	for (int i = 0; i < listDirectoryReplyPro.fileinfopro_size(); ++i) {
		FileMetaData tempFileMetaData;
		tempFileMetaData._id = listDirectoryReplyPro.fileinfopro(i).fileid();
		tempFileMetaData._size =
				listDirectoryReplyPro.fileinfopro(i).filesize();
		tempFileMetaData._path =
				listDirectoryReplyPro.fileinfopro(i).filename();

		_folderData.push_back(tempFileMetaData);
	}

	return;
}

void ListDirectoryReplyMsg::printProtocol() {
	return;
}

