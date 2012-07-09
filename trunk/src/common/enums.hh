#ifndef __ENUMS_HH__
#define __ENUMS_HH__

enum ComponentType {CLIENT, OSD, MDS, MONITOR};

enum FailureReason {UNREACHABLE, DISKFAILURE, OBJECTLOST};

enum MsgType {LIST_DIRECTORY_REQUEST};

enum StorageType {MONGODB, MYSQL};

#endif
