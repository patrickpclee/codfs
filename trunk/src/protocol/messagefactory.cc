/*
 * messagefactory.cc
 */

#include "message.hh"
#include "messagefactory.hh"
#include "../common/debug.hh"
#include "../common/enums.hh"
#include "../common/enumtostring.hh"

#include "metadata/listdirectoryrequest.hh"
#include "metadata/listdirectoryreply.hh"
#include "metadata/uploadfilerequest.hh"
#include "metadata/uploadfilereply.hh"
#include "metadata/uploadobjectack.hh"
#include "metadata/getobjectidlistrequest.hh"
#include "metadata/getobjectidlistreply.hh"
#include "metadata/downloadfilerequest.hh"
#include "metadata/downloadfilereply.hh"
#include "metadata/getobjectinforequest.hh"
#include "metadata/getobjectinforeply.hh"
#include "metadata/saveobjectlistrequest.hh"
#include "metadata/setfilesizerequest.hh"

#include "transfer/putobjectinitrequest.hh"
#include "transfer/putobjectinitreply.hh"
#include "transfer/segmenttransferendrequest.hh"
#include "transfer/segmenttransferendreply.hh"
#include "transfer/putsegmentinitrequest.hh"
#include "transfer/putsegmentinitreply.hh"
#include "transfer/objecttransferendreply.hh"
#include "transfer/objecttransferendrequest.hh"
#include "transfer/objectdatamsg.hh"
#include "transfer/segmentdatamsg.hh"
//#include "transfer/getobjectreadymsg.hh"
//#include "transfer/getsegmentreadymsg.hh"
#include "transfer/getsegmentinitrequest.hh"
#include "transfer/getobjectrequest.hh"
//#include "transfer/getobjectreplymsg.hh"

#include "handshake/handshakerequest.hh"
#include "handshake/handshakereply.hh"

#include "status/osdstartupmsg.hh"
#include "status/osdshutdownmsg.hh"
#include "status/osdstatupdaterequestmsg.hh"
#include "status/osdstatupdatereplymsg.hh"

#include "nodelist/getprimarylistrequest.hh"
#include "nodelist/getprimarylistreply.hh"
#include "nodelist/getsecondarylistrequest.hh"
#include "nodelist/getsecondarylistreply.hh"

#include "nodelist/getosdconfigrequest.hh"
#include "nodelist/getosdconfigreply.hh"

MessageFactory::MessageFactory() {

}

MessageFactory::~MessageFactory() {

}

Message* MessageFactory::createMessage(Communicator* communicator,
		MsgType messageType) {
	switch (messageType) {
	//HAND SHAKE
	case (HANDSHAKE_REQUEST):
		return new HandshakeRequestMsg(communicator);
		break;
	case (HANDSHAKE_REPLY):
		return new HandshakeReplyMsg(communicator);
		break;

		//METADATA
	case (LIST_DIRECTORY_REQUEST):
		return new ListDirectoryRequestMsg(communicator);
		break;
	case (LIST_DIRECTORY_REPLY):
		return new ListDirectoryReplyMsg(communicator);
		break;
	case (UPLOAD_FILE_REQUEST):
		return new UploadFileRequestMsg(communicator);
		break;
	case (UPLOAD_FILE_REPLY):
		return new UploadFileReplyMsg(communicator);
		break;
	case (UPLOAD_OBJECT_ACK):
		return new UploadObjectAckMsg(communicator);
		break;
	case (GET_OBJECT_ID_LIST_REQUEST):
		return new GetObjectIdListRequestMsg(communicator);
		break;
	case (GET_OBJECT_ID_LIST_REPLY):
		return new GetObjectIdListReplyMsg(communicator);
		break;
	case (DOWNLOAD_FILE_REQUEST):
		return new DownloadFileRequestMsg(communicator);
		break;
	case (DOWNLOAD_FILE_REPLY):
		return new DownloadFileReplyMsg(communicator);
		break;
	case (GET_OBJECT_INFO_REQUEST):
		return new GetObjectInfoRequestMsg(communicator);
		break;
	case (GET_OBJECT_INFO_REPLY):
		return new GetObjectInfoReplyMsg(communicator);
		break;
	case (SAVE_OBJECT_LIST_REQUEST):
		return new SaveObjectListRequestMsg(communicator);
		break;
	case (SET_FILE_SIZE_REQUEST):
		return new SetFileSizeRequestMsg(communicator);
		break;

		//TRANSFER
	case (PUT_OBJECT_INIT_REQUEST):
		return new PutObjectInitRequestMsg(communicator);
		break;
	case (PUT_OBJECT_INIT_REPLY):
		return new PutObjectInitReplyMsg(communicator);
		break;
	case (OBJECT_TRANSFER_END_REQUEST):
		return new ObjectTransferEndRequestMsg(communicator);
		break;
	case (OBJECT_TRANSFER_END_REPLY):
		return new ObjectTransferEndReplyMsg(communicator);
		break;
	case (PUT_SEGMENT_INIT_REQUEST):
		return new PutSegmentInitRequestMsg(communicator);
		break;
	case (PUT_SEGMENT_INIT_REPLY):
		return new PutSegmentInitReplyMsg(communicator);
		break;
	case (SEGMENT_TRANSFER_END_REQUEST):
		return new SegmentTransferEndRequestMsg(communicator);
		break;
	case (SEGMENT_TRANSFER_END_REPLY):
		return new SegmentTransferEndReplyMsg(communicator);
		break;
	case (OBJECT_DATA):
		return new ObjectDataMsg(communicator);
		break;
	case (SEGMENT_DATA):
		return new SegmentDataMsg(communicator);
		break;
	case (GET_OBJECT_REQUEST):
		return new GetObjectRequestMsg(communicator);
		break;
		/*
	case (GET_OBJECT_READY):
		return new GetObjectReadyMsg(communicator);
		break;
	case (GET_OBJECT_REPLY):
		return new GetObjectReplyMsg(communicator);
		break;
		 case (GET_SEGMENT_READY):
		 return new GetSegmentReadyMsg(communicator);
		 break;
		 */
	case (GET_SEGMENT_INIT_REQUEST):
		return new GetSegmentInitRequestMsg(communicator);
		break;

		//STATUS
	case (OSD_STARTUP):
		return new OsdStartupMsg(communicator);
		break;
	case (OSD_SHUTDOWN):
		return new OsdShutdownMsg(communicator);
		break;
	case (OSDSTAT_UPDATE_REQUEST):
		return new OsdStatUpdateRequestMsg(communicator);
		break;
	case (OSDSTAT_UPDATE_REPLY):
		return new OsdStatUpdateReplyMsg(communicator);
		break;

		//NODELIST
	case (GET_PRIMARY_LIST_REQUEST):
		return new GetPrimaryListRequestMsg(communicator);
		break;
	case (GET_PRIMARY_LIST_REPLY):
		return new GetPrimaryListReplyMsg(communicator);
		break;
	case (GET_SECONDARY_LIST_REQUEST):
		return new GetSecondaryListRequestMsg(communicator);
		break;
	case (GET_SECONDARY_LIST_REPLY):
		return new GetSecondaryListReplyMsg(communicator);
		break;
	case (GET_OSD_CONFIG_REQUEST):
		return new GetOsdConfigRequestMsg(communicator);
		break;
	case (GET_OSD_CONFIG_REPLY):
		return new GetOsdConfigReplyMsg(communicator);
		break;


	default:
		debug("Invalid message type : %s\n", EnumToString::toString(messageType));
		break;
	}
	return NULL;
}
