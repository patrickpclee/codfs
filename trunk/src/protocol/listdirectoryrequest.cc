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
	const MsgType msgType = LIST_DIRECTORY_REQUEST;

	ncvfs::ListDirectoryRequest listDirectoryRequest;
	listDirectoryRequest.set_directorypath(_directoryPath);
	listDirectoryRequest.set_osdid(_osdId);

	if (!listDirectoryRequest.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	const uint32_t msgSize = serializedString.length();

	prepareMsgHeader(msgType, msgSize);

	cout << "Message Prepared. Type = " << msgType << " Size = " << msgSize << endl;
}
