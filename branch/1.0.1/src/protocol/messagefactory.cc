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
#include "metadata/deletefilerequest.hh"
#include "metadata/deletefilereply.hh"
#include "metadata/uploadsegmentack.hh"
#include "metadata/uploadsegmentackreply.hh"
#include "metadata/getsegmentidlistrequest.hh"
#include "metadata/getsegmentidlistreply.hh"
#include "metadata/downloadfilerequest.hh"
#include "metadata/downloadfilereply.hh"
#include "metadata/getsegmentinforequest.hh"
#include "metadata/getsegmentinforeply.hh"
#include "metadata/savesegmentlistrequest.hh"
#include "metadata/savesegmentlistreply.hh"
#include "metadata/setfilesizerequest.hh"
#include "metadata/renamefilerequest.hh"
#include "metadata/renamefilereply.hh"

#include "transfer/putsegmentinitrequest.hh"
#include "transfer/putsegmentinitreply.hh"
#include "transfer/blocktransferendrequest.hh"
#include "transfer/blocktransferendreply.hh"
#include "transfer/putblockinitrequest.hh"
#include "transfer/putblockinitreply.hh"
#include "transfer/segmenttransferendreply.hh"
#include "transfer/segmenttransferendrequest.hh"
#include "transfer/segmentdatamsg.hh"
#include "transfer/blockdatamsg.hh"
#include "transfer/getblockinitrequest.hh"
#include "transfer/getsegmentrequest.hh"
#include "transfer/putsmallsegmentrequest.hh"

#include "handshake/handshakerequest.hh"
#include "handshake/handshakereply.hh"

#include "status/osdstartupmsg.hh"
#include "status/osdshutdownmsg.hh"
#include "status/osdstatupdaterequestmsg.hh"
#include "status/osdstatupdatereplymsg.hh"
#include "status/newosdregistermsg.hh"
#include "status/onlineosdlistmsg.hh"

#include "nodelist/getprimarylistrequest.hh"
#include "nodelist/getprimarylistreply.hh"
#include "nodelist/getsecondarylistrequest.hh"
#include "nodelist/getsecondarylistreply.hh"
#include "nodelist/getosdlistrequest.hh"
#include "nodelist/getosdlistreply.hh"

#include "status/getosdstatusrequestmsg.hh"
#include "status/getosdstatusreplymsg.hh"

#include "status/recoverytriggerrequest.hh"
#include "status/recoverytriggerreply.hh"
#include "status/repairsegmentinfomsg.hh"

MessageFactory::MessageFactory() {

}

MessageFactory::~MessageFactory() {

}

Message* MessageFactory::createMessage(Communicator* communicator,
		MsgType messageType) {

	switch (messageType) {

	//HANDSHAKE
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
	case (DELETE_FILE_REQUEST):
		return new DeleteFileRequestMsg(communicator);
		break;
	case (DELETE_FILE_REPLY):
		return new DeleteFileReplyMsg(communicator);
		break;
	case (UPLOAD_SEGMENT_ACK):
		return new UploadSegmentAckMsg(communicator);
		break;
	case (UPLOAD_SEGMENT_ACK_REPLY):
		return new UploadSegmentAckReplyMsg(communicator);
		break;
	case (GET_SEGMENT_ID_LIST_REQUEST):
		return new GetSegmentIdListRequestMsg(communicator);
		break;
	case (GET_SEGMENT_ID_LIST_REPLY):
		return new GetSegmentIdListReplyMsg(communicator);
		break;
	case (DOWNLOAD_FILE_REQUEST):
		return new DownloadFileRequestMsg(communicator);
		break;
	case (DOWNLOAD_FILE_REPLY):
		return new DownloadFileReplyMsg(communicator);
		break;
	case (GET_SEGMENT_INFO_REQUEST):
		return new GetSegmentInfoRequestMsg(communicator);
		break;
	case (GET_SEGMENT_INFO_REPLY):
		return new GetSegmentInfoReplyMsg(communicator);
		break;
	case (SAVE_SEGMENT_LIST_REQUEST):
		return new SaveSegmentListRequestMsg(communicator);
		break;
	case (SAVE_SEGMENT_LIST_REPLY):
		return new SaveSegmentListReplyMsg(communicator);
		break;
	case (SET_FILE_SIZE_REQUEST):
		return new SetFileSizeRequestMsg(communicator);
		break;
	case (RENAME_FILE_REQUEST):
		return new RenameFileRequestMsg(communicator);
		break;
	case (RENAME_FILE_REPLY):
		return new RenameFileReplyMsg(communicator);
		break;

	//TRANSFER
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
	case (PUT_BLOCK_INIT_REQUEST):
		return new PutBlockInitRequestMsg(communicator);
		break;
	case (PUT_BLOCK_INIT_REPLY):
		return new PutBlockInitReplyMsg(communicator);
		break;
	case (BLOCK_TRANSFER_END_REQUEST):
		return new BlockTransferEndRequestMsg(communicator);
		break;
	case (BLOCK_TRANSFER_END_REPLY):
		return new BlockTransferEndReplyMsg(communicator);
		break;
	case (SEGMENT_DATA):
		return new SegmentDataMsg(communicator);
		break;
	case (BLOCK_DATA):
		return new BlockDataMsg(communicator);
		break;
	case (GET_SEGMENT_REQUEST):
		return new GetSegmentRequestMsg(communicator);
		break;
	case (GET_BLOCK_INIT_REQUEST):
		return new GetBlockInitRequestMsg(communicator);
		break;
	case (PUT_SMALL_SEGMENT_REQUEST):
	    return new PutSmallSegmentRequestMsg(communicator);
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
	case (NEW_OSD_REGISTER):
		return new NewOsdRegisterMsg(communicator);
		break;
	case (ONLINE_OSD_LIST):
		return new OnlineOsdListMsg(communicator);
		break;
	case (GET_OSD_STATUS_REQUEST):
		return new GetOsdStatusRequestMsg(communicator);
		break;
	case (GET_OSD_STATUS_REPLY):
		return new GetOsdStatusReplyMsg(communicator);
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
	case (GET_OSD_LIST_REQUEST):
		return new GetOsdListRequestMsg(communicator);
		break;
	case (GET_OSD_LIST_REPLY):
		return new GetOsdListReplyMsg(communicator);
		break;
	case (REPAIR_SEGMENT_INFO):
		return new RepairSegmentInfoMsg(communicator);
		break;


	//RECOVERY
	case (RECOVERY_TRIGGER_REQUEST):
		return new RecoveryTriggerRequestMsg(communicator);
		break;
	case (RECOVERY_TRIGGER_REPLY):
		return new RecoveryTriggerReplyMsg(communicator);
		break;

	default:
		debug("Invalid message type : %s\n", EnumToString::toString(messageType));
		break;
	}
	return NULL;
}
