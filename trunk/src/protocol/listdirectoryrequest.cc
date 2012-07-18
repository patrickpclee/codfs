/**
 * listdirectoryrequest.cc
 */

#include <iostream>
#include "listdirectoryrequest.hh"
#include "../protocol/message.pb.h"
#include "../common/enums.hh"

/**
 * Constructor - Save parameters in private variables
 * @param osdId My OSD ID
 * @param directoryPath Requested directory path
 * @param mdsSockfd Socket descriptor of MDS
 */

ListDirectoryRequestMsg::ListDirectoryRequestMsg(uint32_t osdId,
		string directoryPath, uint32_t mdsSockfd) {
	_osdId = osdId;
	_directoryPath = directoryPath;
	_sockfd = mdsSockfd;
}

/**
 * Copy values in private variables to protocol message
 * Serialize protocol message and copy to private variable
 */

void ListDirectoryRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::ListDirectoryRequestPro listDirectoryRequestPro;
	listDirectoryRequestPro.set_directorypath(_directoryPath);
	listDirectoryRequestPro.set_osdid(_osdId);

	if (!listDirectoryRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(LIST_DIRECTORY_REQUEST);
	setProtocolMsg(serializedString);

}

/**
 * DEBUG: override method: print protocol message
 */

void ListDirectoryRequestMsg::printProtocol() {
	cout << "[LIST_DIRECTORY_REQUEST] osdID = " << _osdId << " Path = "
			<< _directoryPath << endl;
}
