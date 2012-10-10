
#include <iostream>
#include "iwantfilesize.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"
#include "../../common/debug.hh"

#ifdef COMPILE_FOR_MDS
#include "../../mds/mds.hh"
extern Mds* mds;
#endif

/**
 * Default Constructor
 */

IWantFileSizeMsg::IWantFileSizeMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
IWantFileSizeMsg::IWantFileSizeMsg(Communicator* communicator,
		uint32_t mdsSockfd, uint32_t clientId, uint32_t fileId) :
		Message(communicator) {
	_sockfd = mdsSockfd;
	_fileId = fileId;
}

/**
 * Constructor - Save parameters in private variables
 */

void IWantFileSizeMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::IWantFileSizeMsgPro iWantFileSizeMsgPro;

	iWantFileSizeMsgPro.set_fileid(_fileId);

	setProtocolSize(serializedString.length());
	setProtocolType(I_WANT_FILE_SIZE_MSG);
	setProtocolMsg(serializedString);

}

void IWantFileSizeMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::IWantFileSizeMsgPro iWantFileSizeMsgPro;
	iWantFileSizeMsgPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_fileId = iWantFileSizeMsgPro.fileid();

}

void IWantFileSizeMsg::doHandle() {
#ifdef COMPILE_FOR_MDS
	mds->FileSizeProcessor(_msgHeader.requestId, _sockfd, _fileId);
#endif
}

void IWantFileSizeMsg::printProtocol() {

}

void IWantFileSizeMsg::setSize (uint64_t size) {
	_size = size;
}

uint32_t IWantFileSizeMsg::getSize() {
	return _size;
}
