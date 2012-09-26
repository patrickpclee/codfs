

#include <iostream>
#include "iwantfilesize.hh"
#include "heresfilesize.hh"
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

HeresFileSizeMsg::HeresFileSizeMsg(Communicator* communicator) :
		Message(communicator) {
}

/**
 * Constructor - Save parameters in private variables
 */
HeresFileSizeMsg::HeresFileSizeMsg(Communicator* communicator,
		uint32_t requestId, uint32_t sockfd, uint32_t fileId, uint64_t fileSize) :
		Message(communicator) {

	_msgHeader.requestId = requestId;
	_sockfd = sockfd;
	_fileId = fileId;
	_fileSize = fileSize;


}

void HeresFileSizeMsg::prepareProtocolMsg()  {
	string serializedString;

	ncvfs::HeresFileSizeMsgPro heresFileSizeMsgPro;

	heresFileSizeMsgPro.set_fileid(_fileId);
	heresFileSizeMsgPro.set_filesize(_fileSize);

	if (!heresFileSizeMsgPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType (HERES_FILE_SIZE_MSG);
	setProtocolMsg(serializedString);

}

void HeresFileSizeMsg::parse(char* buf) {
	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::HeresFileSizeMsgPro heresFileSizeMsgPro;
	heresFileSizeMsgPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_fileId = heresFileSizeMsgPro.fileid();
	_fileSize = heresFileSizeMsgPro.filesize();

}

void HeresFileSizeMsg::doHandle() {
	IWantFileSizeMsg* iWantFileSizeMsg =
			(IWantFileSizeMsg*) _communicator->popWaitReplyMessage(
					_msgHeader.requestId);
	iWantFileSizeMsg->setSize(_fileSize);
	iWantFileSizeMsg->setStatus(READY);
}

void HeresFileSizeMsg::printProtocol() {

}
