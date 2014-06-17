#ifndef _H_G__EnumToString
#define _H_G__EnumToString

#include <iostream>
using std::cout;
#include "enums.hh"


class EnumToString {
  EnumToString(); // utility class, no instances

public:
  static const char * toString( MsgType en ) {
    switch( en ) {
      case BLOCK_DATA: return "BLOCK_DATA";
      case BLOCK_TRANSFER_END_REPLY: return "BLOCK_TRANSFER_END_REPLY";
      case BLOCK_TRANSFER_END_REQUEST: return "BLOCK_TRANSFER_END_REQUEST";
      case DEFAULT: return "DEFAULT";
      case DELETE_FILE_REPLY: return "DELETE_FILE_REPLY";
      case DELETE_FILE_REQUEST: return "DELETE_FILE_REQUEST";
      case DOWNLOAD_FILE_REPLY: return "DOWNLOAD_FILE_REPLY";
      case DOWNLOAD_FILE_REQUEST: return "DOWNLOAD_FILE_REQUEST";
      case GET_BLOCK_INIT_REQUEST: return "GET_BLOCK_INIT_REQUEST";
      case GET_OSD_LIST_REPLY: return "GET_OSD_LIST_REPLY";
      case GET_OSD_LIST_REQUEST: return "GET_OSD_LIST_REQUEST";
      case GET_OSD_STATUS_REPLY: return "GET_OSD_STATUS_REPLY";
      case GET_OSD_STATUS_REQUEST: return "GET_OSD_STATUS_REQUEST";
      case GET_PRIMARY_LIST_REPLY: return "GET_PRIMARY_LIST_REPLY";
      case GET_PRIMARY_LIST_REQUEST: return "GET_PRIMARY_LIST_REQUEST";
      case GET_SECONDARY_LIST_REPLY: return "GET_SECONDARY_LIST_REPLY";
      case GET_SECONDARY_LIST_REQUEST: return "GET_SECONDARY_LIST_REQUEST";
      case GET_SEGMENT_ID_LIST_REPLY: return "GET_SEGMENT_ID_LIST_REPLY";
      case GET_SEGMENT_ID_LIST_REQUEST: return "GET_SEGMENT_ID_LIST_REQUEST";
      case GET_SEGMENT_INFO_REPLY: return "GET_SEGMENT_INFO_REPLY";
      case GET_SEGMENT_INFO_REQUEST: return "GET_SEGMENT_INFO_REQUEST";
      case GET_SEGMENT_REQUEST: return "GET_SEGMENT_REQUEST";
      case HANDSHAKE_REPLY: return "HANDSHAKE_REPLY";
      case HANDSHAKE_REQUEST: return "HANDSHAKE_REQUEST";
      case LIST_DIRECTORY_REPLY: return "LIST_DIRECTORY_REPLY";
      case LIST_DIRECTORY_REQUEST: return "LIST_DIRECTORY_REQUEST";
      case MSGTYPE_END: return "MSGTYPE_END";
      case NEW_OSD_REGISTER: return "NEW_OSD_REGISTER";
      case ONLINE_OSD_LIST: return "ONLINE_OSD_LIST";
      case OSDSTAT_UPDATE_REPLY: return "OSDSTAT_UPDATE_REPLY";
      case OSDSTAT_UPDATE_REQUEST: return "OSDSTAT_UPDATE_REQUEST";
      case OSD_SHUTDOWN: return "OSD_SHUTDOWN";
      case OSD_STARTUP: return "OSD_STARTUP";
      case PUT_BLOCK_INIT_REPLY: return "PUT_BLOCK_INIT_REPLY";
      case PUT_BLOCK_INIT_REQUEST: return "PUT_BLOCK_INIT_REQUEST";
      case PUT_SEGMENT_INIT_REPLY: return "PUT_SEGMENT_INIT_REPLY";
      case PUT_SEGMENT_INIT_REQUEST: return "PUT_SEGMENT_INIT_REQUEST";
      case PUT_SMALL_SEGMENT_REQUEST: return "PUT_SMALL_SEGMENT_REQUEST";
      case RECOVERY_TRIGGER_REPLY: return "RECOVERY_TRIGGER_REPLY";
      case RECOVERY_TRIGGER_REQUEST: return "RECOVERY_TRIGGER_REQUEST";
      case RENAME_FILE_REPLY: return "RENAME_FILE_REPLY";
      case RENAME_FILE_REQUEST: return "RENAME_FILE_REQUEST";
      case REPAIR_SEGMENT_INFO: return "REPAIR_SEGMENT_INFO";
      case SAVE_SEGMENT_LIST_REPLY: return "SAVE_SEGMENT_LIST_REPLY";
      case SAVE_SEGMENT_LIST_REQUEST: return "SAVE_SEGMENT_LIST_REQUEST";
      case SEGMENT_DATA: return "SEGMENT_DATA";
      case SEGMENT_TRANSFER_END_REPLY: return "SEGMENT_TRANSFER_END_REPLY";
      case SEGMENT_TRANSFER_END_REQUEST: return "SEGMENT_TRANSFER_END_REQUEST";
      case SET_FILE_SIZE_REQUEST: return "SET_FILE_SIZE_REQUEST";
      case UPLOAD_FILE_REPLY: return "UPLOAD_FILE_REPLY";
      case UPLOAD_FILE_REQUEST: return "UPLOAD_FILE_REQUEST";
      case UPLOAD_SEGMENT_ACK: return "UPLOAD_SEGMENT_ACK";
      case UPLOAD_SEGMENT_ACK_REPLY: return "UPLOAD_SEGMENT_ACK_REPLY";
    }
    return "???";
  }

  static const char * toString( CodingScheme en ) {
    switch( en ) {
      case CAUCHY: return "CAUCHY";
      case DEFAULT_CODING: return "DEFAULT_CODING";
      case EMBR_CODING: return "EMBR_CODING";
      case EVENODD_CODING: return "EVENODD_CODING";
      case RAID0_CODING: return "RAID0_CODING";
      case RAID1_CODING: return "RAID1_CODING";
      case RAID5_CODING: return "RAID5_CODING";
      case RDP_CODING: return "RDP_CODING";
      case RS_CODING: return "RS_CODING";
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

  static const char * toString( BlockType en ) {
    switch( en ) {
      case DATA_BLOCK: return "DATA_BLOCK";
      case DEFAULT_BLOCK_TYPE: return "DEFAULT_BLOCK_TYPE";
      case PARITY_BLOCK: return "PARITY_BLOCK";
    }
    return "???";
  }

  static const char * toString( DataMsgType en ) {
    switch( en ) {
      case DEFAULT_DATA_MSG: return "DEFAULT_DATA_MSG";
      case DOWNLOAD: return "DOWNLOAD";
      case PARITY: return "PARITY";
      case RECOVERY: return "RECOVERY";
      case UPDATE: return "UPDATE";
      case UPLOAD: return "UPLOAD";
    }
    return "???";
  }

  static const char * toString( FailureReason en ) {
    switch( en ) {
      case DISKFAILURE: return "DISKFAILURE";
      case SEGMENTLOST: return "SEGMENTLOST";
      case UNREACHABLE: return "UNREACHABLE";
    }
    return "???";
  }

  static const char * toString( UpdateScheme en ) {
    switch( en ) {
      case FL: return "FL";
      case FO: return "FO";
      case PL: return "PL";
      case PLR: return "PLR";
    }
    return "???";
  }

  static const char * toString( FileType en ) {
    switch( en ) {
      case FOLDER: return "FOLDER";
      case NEWFILE: return "NEWFILE";
      case NORMAL: return "NORMAL";
      case NOTFOUND: return "NOTFOUND";
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
