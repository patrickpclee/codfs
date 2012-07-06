#include <iostream>
#include "listdirectoryrequest.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"

ListDirectoryRequestMessage::ListDirectoryRequestMessage(uint32_t osdId,
		string directoryPath) {
	_osdId = osdId;
	_directoryPath = directoryPath;
}

void ListDirectoryRequestMessage::prepareProtocolMsg() {
	string serializedString;

	ncvfs::ListDirectoryRequest listDirectoryRequest;
	listDirectoryRequest.set_directorypath(_directoryPath);
	listDirectoryRequest.set_osdid(_osdId);

	if (!listDirectoryRequest.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(LIST_DIRECTORY_REQUEST);

}

void ListDirectoryRequestMessage::printProtocol() {
	cout << "[LIST_DIRECTORY_REQUEST] osdID = " << _osdId << " Path = "
			<< _directoryPath << endl;
}
