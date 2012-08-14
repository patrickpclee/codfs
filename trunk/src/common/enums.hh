#ifndef __ENUMS_HH__
#define __ENUMS_HH__

enum ComponentType {
	CLIENT = 1, OSD = 2 , MDS = 3, MONITOR = 4	// numbers match message.proto
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
	HANDSHAKE_REQUEST,
	HANDSHAKE_REPLY,
	LIST_DIRECTORY_REQUEST,
	LIST_DIRECTORY_REPLY,
	UPLOAD_FILE_REQUEST,
	UPLOAD_FILE_REPLY,
	PUT_OBJECT_INIT_REQUEST,
	PUT_OBJECT_INIT_REPLY,
	PUT_OBJECT_END_REQUEST,
	PUT_OBJECT_END_REPLY,
	PUT_SEGMENT_INIT_REQUEST,
	PUT_SEGMENT_INIT_REPLY,
	PUT_SEGMENT_END_REQUEST,
	PUT_SEGMENT_END_REPLY,
	OBJECT_DATA,
	SEGMENT_DATA,
	UPLOAD_OBJECT_ACK,
	OSD_STARTUP,
	OSD_SHUTDOWN,
	OSDSTAT_UPDATE_REQUEST,
	OSDSTAT_UPDATE_REPLY
};

enum StorageType {
	MONGODB, MYSQL
};

enum CodingScheme {
	RAID0_CODING, RAID1_CODING
};

#endif
