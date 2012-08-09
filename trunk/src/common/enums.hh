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
	PUT_OBJECT_INIT_REQUEST,
	PUT_OBJECT_INIT_REPLY,
	PUT_OBJECT_END_REQUEST,
	PUT_OBJECT_END_REPLY,
	PUT_SEGMENT_INIT_REQUEST,
	PUT_SEGMENT_INIT_REPLY,
	PUT_SEGMENT_END_REQUEST,
	PUT_SEGMENT_END_REPLY,
	OBJECT_DATA,
	SEGMENT_DATA
};

enum StorageType {
	MONGODB, MYSQL
};

enum CodingScheme {
	DUMMY_CODING, REPLICATION_CODING
};

#endif
