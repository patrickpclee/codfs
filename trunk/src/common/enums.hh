#ifndef __ENUMS_HH__
#define __ENUMS_HH__

enum ComponentType {
	CLIENT = 1, OSD = 2, MDS = 3, MONITOR = 4 // numbers match message.proto
};

enum FailureReason {
	UNREACHABLE, DISKFAILURE, OBJECTLOST
};

enum MessageStatus {
	WAITING, READY, TIMEOUT
};

enum FileType {
	NORMAL, FOLDER
};

enum MsgType {
	DEFAULT,

	// HANDSHAKE
	HANDSHAKE_REQUEST,
	HANDSHAKE_REPLY,

	// METADATA
	LIST_DIRECTORY_REQUEST,
	LIST_DIRECTORY_REPLY,
	UPLOAD_FILE_REQUEST,
	UPLOAD_FILE_REPLY,
	UPLOAD_OBJECT_ACK,
	GET_OBJECT_ID_LIST_REQUEST,
	GET_OBJECT_ID_LIST_REPLY,
	DOWNLOAD_FILE_REQUEST,
	DOWNLOAD_FILE_REPLY,
	GET_OBJECT_INFO_REQUEST,
	GET_OBJECT_INFO_REPLY,
	SAVE_OBJECT_LIST_REQUEST,
	SET_FILE_SIZE_REQUEST,

	// TRANSFER
	PUT_OBJECT_INIT_REQUEST,
	PUT_OBJECT_INIT_REPLY,
	OBJECT_TRANSFER_END_REQUEST,
	OBJECT_TRANSFER_END_REPLY,
	PUT_SEGMENT_INIT_REQUEST,
	PUT_SEGMENT_INIT_REPLY,
	SEGMENT_TRANSFER_END_REQUEST,
	SEGMENT_TRANSFER_END_REPLY,
	OBJECT_DATA,
	SEGMENT_DATA,
	GET_OBJECT_REQUEST,
//	GET_OBJECT_REPLY,
	GET_SEGMENT_INIT_REQUEST,
//	GET_SEGMENT_INIT_REPLY,
//	GET_SEGMENT_READY,
//	GET_OBJECT_READY,

	// STATUS
	OSD_STARTUP,
	OSD_SHUTDOWN,
	OSDSTAT_UPDATE_REQUEST,
	OSDSTAT_UPDATE_REPLY,

	// NODE_LIST
	GET_PRIMARY_LIST_REQUEST,
	GET_PRIMARY_LIST_REPLY,
	GET_SECONDARY_LIST_REQUEST,
	GET_SECONDARY_LIST_REPLY,
	GET_OSD_CONFIG_REQUEST,
	GET_OSD_CONFIG_REPLY,

	// END
	MSGTYPE_END
};

enum StorageType {
	MONGODB, MYSQL
};

enum CodingScheme {
	DEFAULT_CODING = 15, RAID0_CODING = 1, RAID1_CODING = 2
};

#endif
