#include "putsegmentinitrequest.hh"
#include "../../common/debug.hh"
#include "../../protocol/message.pb.h"
#include "../../common/enums.hh"
#include "../../common/memorypool.hh"

#ifdef COMPILE_FOR_OSD
#include "../../osd/osd.hh"
extern Osd* osd;
#endif

#ifdef COMPILE_FOR_CLIENT
#include "../../client/client.hh"
extern Client* client;
#endif

PutSegmentInitRequestMsg::PutSegmentInitRequestMsg(Communicator* communicator) :
		Message(communicator) {

}

PutSegmentInitRequestMsg::PutSegmentInitRequestMsg(Communicator* communicator,
		uint32_t osdSockfd, uint64_t segmentId, uint32_t segmentSize,
		uint32_t chunkCount, CodingScheme codingScheme, const string &codingSetting,
		const string &checksum) :
		Message(communicator) {

	_sockfd = osdSockfd;
	_segmentId = segmentId;
	_segmentSize = segmentSize;
	_chunkCount = chunkCount;
	_codingScheme = codingScheme;
	_codingSetting = codingSetting;
	_checksum = checksum;

}

void PutSegmentInitRequestMsg::prepareProtocolMsg() {
	string serializedString;

	ncvfs::PutSegmentInitRequestPro putSegmentInitRequestPro;
	putSegmentInitRequestPro.set_segmentid(_segmentId);
	putSegmentInitRequestPro.set_segmentsize(_segmentSize);
	putSegmentInitRequestPro.set_chunkcount(_chunkCount);
	putSegmentInitRequestPro.set_codingscheme(
			(ncvfs::PutSegmentInitRequestPro_CodingScheme) _codingScheme);
	putSegmentInitRequestPro.set_codingsetting(_codingSetting);
	putSegmentInitRequestPro.set_checksum(_checksum);

	if (!putSegmentInitRequestPro.SerializeToString(&serializedString)) {
		cerr << "Failed to write string." << endl;
		return;
	}

	setProtocolSize(serializedString.length());
	setProtocolType(PUT_SEGMENT_INIT_REQUEST);
	setProtocolMsg(serializedString);

}

void PutSegmentInitRequestMsg::parse(char* buf) {

	memcpy(&_msgHeader, buf, sizeof(struct MsgHeader));

	ncvfs::PutSegmentInitRequestPro putSegmentInitRequestPro;
	putSegmentInitRequestPro.ParseFromArray(buf + sizeof(struct MsgHeader),
			_msgHeader.protocolMsgSize);

	_segmentId = putSegmentInitRequestPro.segmentid();
	_segmentSize = putSegmentInitRequestPro.segmentsize();
	_chunkCount = putSegmentInitRequestPro.chunkcount();
	_codingScheme = (CodingScheme) putSegmentInitRequestPro.codingscheme();
	_codingSetting = putSegmentInitRequestPro.codingsetting();
	_checksum = putSegmentInitRequestPro.checksum();

}

void PutSegmentInitRequestMsg::doHandle() {
#ifdef COMPILE_FOR_OSD
	osd->putSegmentInitProcessor (_msgHeader.requestId, _sockfd, _segmentId,
			_segmentSize, _chunkCount, _codingScheme, _codingSetting, _checksum);
#endif

#ifdef COMPILE_FOR_CLIENT
	client->putSegmentInitProcessor (_msgHeader.requestId, _sockfd, _segmentId,
			_segmentSize, _chunkCount, _checksum);
#endif
}

void PutSegmentInitRequestMsg::printProtocol() {
	debug(
			"[PUT_SEGMENT_INIT_REQUEST] Segment ID = %" PRIu64 ", Length = %" PRIu64 ", Count = %" PRIu32 " CodingScheme = %" PRIu32 " CodingSetting = %s Checksum = %s\n",
			_segmentId, _segmentSize, _chunkCount, _codingScheme, _codingSetting.c_str(), _checksum.c_str());
}
