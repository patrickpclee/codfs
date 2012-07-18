/**
 * listdirectoryreply.cc
 */

#include <iostream>

#include "listdirectoryreply.hh"

#include "../protocol/message.pb.h"
#include "../common/enums.hh"

ListDirectoryReplyMsg::ListDirectoryReplyMsg ()
{

}

ListDirectoryReplyMsg::ListDirectoryReplyMsg (uint32_t requestId, uint32_t sockfd, string path, vector<FileMetaData> folderData)
{
	setVariables(requestId, sockfd, path, folderData);
}

void ListDirectoryReplyMsg::setVariables (uint32_t requestId, uint32_t sockfd, string path, vector<FileMetaData> folderData)
{
	_sockfd = sockfd;
	_requestId = requestId;
	_path = path;
	_folderData = folderData;

	return ;
}

/**
 * @brief	Copy values in private variables to protocol message
 * Serialize protocol message and copy to private variable
 */
void ListDirectoryReplyMsg::prepareProtocolMsg () {
	string serializedString;

	ncvfs::ListDirectoryReplyPro listDirectoryReplyPro;

	vector<FileMetaData>::iterator it;

	for (it = _folderData.begin(); it < _folderData.end(); ++it)
	{
		ncvfs::FileInfoPro* fileInfoPro = listDirectoryReplyPro.add_fileinfopro();
		fileInfoPro->set_fileid((*it)._id);
		fileInfoPro->set_filesize((*it)._size);
		fileInfoPro->set_filename((*it)._path);
	}

	if (!listDirectoryReplyPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return ;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(LIST_DIRECTORY_REPLY);
	setProtocolMsg(serializedString);

	return ;
}
