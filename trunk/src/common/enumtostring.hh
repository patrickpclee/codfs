#ifndef _H_G__EnumToString
#define _H_G__EnumToString

#include <iostream>
using std::cout;
#include "enums.hh"


class EnumToString {
  EnumToString(); // utility class, no instances

public:
  static const char * toString( ObjectDataStatus en ) {
    switch( en ) {
      case CLEAN: return "CLEAN";
      case DIRTY: return "DIRTY";
      case NEW: return "NEW";
      case UNFETCHED: return "UNFETCHED";
    }
    return "???";
  }

  static const char * toString( ComponentType en ) {
    switch( en ) {
      case CLIENT: return "CLIENT";
      case MDS: return "MDS";
      case MONITOR: return "MONITOR";
      case OSD: return "OSD";
    }
    return "???";
  }

  static const char * toString( MsgType en ) {
    switch( en ) {
      case DEFAULT: return "DEFAULT";
      case DELETE_FILE_REPLY: return "DELETE_FILE_REPLY";
      case DELETE_FILE_REQUEST: return "DELETE_FILE_REQUEST";
      case DOWNLOAD_FILE_REPLY: return "DOWNLOAD_FILE_REPLY";
      case DOWNLOAD_FILE_REQUEST: return "DOWNLOAD_FILE_REQUEST";
      case GET_OBJECT_ID_LIST_REPLY: return "GET_OBJECT_ID_LIST_REPLY";
      case GET_OBJECT_ID_LIST_REQUEST: return "GET_OBJECT_ID_LIST_REQUEST";
      case GET_OBJECT_INFO_REPLY: return "GET_OBJECT_INFO_REPLY";
      case GET_OBJECT_INFO_REQUEST: return "GET_OBJECT_INFO_REQUEST";
      case GET_OBJECT_REQUEST: return "GET_OBJECT_REQUEST";
      case GET_OSD_LIST_REPLY: return "GET_OSD_LIST_REPLY";
      case GET_OSD_LIST_REQUEST: return "GET_OSD_LIST_REQUEST";
      case GET_OSD_STATUS_REPLY: return "GET_OSD_STATUS_REPLY";
      case GET_OSD_STATUS_REQUEST: return "GET_OSD_STATUS_REQUEST";
      case GET_PRIMARY_LIST_REPLY: return "GET_PRIMARY_LIST_REPLY";
      case GET_PRIMARY_LIST_REQUEST: return "GET_PRIMARY_LIST_REQUEST";
      case GET_SECONDARY_LIST_REPLY: return "GET_SECONDARY_LIST_REPLY";
      case GET_SECONDARY_LIST_REQUEST: return "GET_SECONDARY_LIST_REQUEST";
      case GET_SEGMENT_INIT_REQUEST: return "GET_SEGMENT_INIT_REQUEST";
      case HANDSHAKE_REPLY: return "HANDSHAKE_REPLY";
      case HANDSHAKE_REQUEST: return "HANDSHAKE_REQUEST";
      case LIST_DIRECTORY_REPLY: return "LIST_DIRECTORY_REPLY";
      case LIST_DIRECTORY_REQUEST: return "LIST_DIRECTORY_REQUEST";
      case MSGTYPE_END: return "MSGTYPE_END";
      case NEW_OSD_REGISTER: return "NEW_OSD_REGISTER";
      case OBJECT_DATA: return "OBJECT_DATA";
      case OBJECT_TRANSFER_END_REPLY: return "OBJECT_TRANSFER_END_REPLY";
      case OBJECT_TRANSFER_END_REQUEST: return "OBJECT_TRANSFER_END_REQUEST";
      case ONLINE_OSD_LIST: return "ONLINE_OSD_LIST";
      case OSDSTAT_UPDATE_REPLY: return "OSDSTAT_UPDATE_REPLY";
      case OSDSTAT_UPDATE_REQUEST: return "OSDSTAT_UPDATE_REQUEST";
      case OSD_SHUTDOWN: return "OSD_SHUTDOWN";
      case OSD_STARTUP: return "OSD_STARTUP";
      case PUT_OBJECT_INIT_REPLY: return "PUT_OBJECT_INIT_REPLY";
      case PUT_OBJECT_INIT_REQUEST: return "PUT_OBJECT_INIT_REQUEST";
      case PUT_SEGMENT_INIT_REPLY: return "PUT_SEGMENT_INIT_REPLY";
      case PUT_SEGMENT_INIT_REQUEST: return "PUT_SEGMENT_INIT_REQUEST";
      case REPAIR_OBJECT_INFO: return "REPAIR_OBJECT_INFO";
      case SAVE_OBJECT_LIST_REPLY: return "SAVE_OBJECT_LIST_REPLY";
      case SAVE_OBJECT_LIST_REQUEST: return "SAVE_OBJECT_LIST_REQUEST";
      case SEGMENT_DATA: return "SEGMENT_DATA";
      case SEGMENT_TRANSFER_END_REPLY: return "SEGMENT_TRANSFER_END_REPLY";
      case SEGMENT_TRANSFER_END_REQUEST: return "SEGMENT_TRANSFER_END_REQUEST";
      case SET_FILE_SIZE_REQUEST: return "SET_FILE_SIZE_REQUEST";
      case SWITCH_PRIMARY_OSD_REPLY: return "SWITCH_PRIMARY_OSD_REPLY";
      case SWITCH_PRIMARY_OSD_REQUEST: return "SWITCH_PRIMARY_OSD_REQUEST";
      case UPLOAD_FILE_REPLY: return "UPLOAD_FILE_REPLY";
      case UPLOAD_FILE_REQUEST: return "UPLOAD_FILE_REQUEST";
      case UPLOAD_OBJECT_ACK: return "UPLOAD_OBJECT_ACK";
    }
    return "???";
  }

  static const char * toString( CodingScheme en ) {
    switch( en ) {
      case DEFAULT_CODING: return "DEFAULT_CODING";
      case RAID0_CODING: return "RAID0_CODING";
      case RAID1_CODING: return "RAID1_CODING";
      case RAID5_CODING: return "RAID5_CODING";
      case RS_CODING: return "RS_CODING";
    }
    return "???";
  }

  static const char * toString( FailureReason en ) {
    switch( en ) {
      case DISKFAILURE: return "DISKFAILURE";
      case OBJECTLOST: return "OBJECTLOST";
      case UNREACHABLE: return "UNREACHABLE";
    }
    return "???";
  }

  static const char * toString( FileType en ) {
    switch( en ) {
      case FOLDER: return "FOLDER";
      case NORMAL: return "NORMAL";
      case NOTFOUND: return "NOTFOUND";
    }
    return "???";
  }

  static const char * toString( StorageType en ) {
    switch( en ) {
      case MONGODB: return "MONGODB";
      case MYSQL: return "MYSQL";
    }
    return "???";
  }

  static const char * toString( MessageStatus en ) {
    switch( en ) {
      case READY: return "READY";
      case TIMEOUT: return "TIMEOUT";
      case WAITING: return "WAITING";
    }
    return "???";
  }

};

#endif
