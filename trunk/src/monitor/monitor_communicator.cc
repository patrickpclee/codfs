/**
 * monitor _communicator.cc
 * by DING Qian
 * Aug 9, 2012
 */

#include <iostream>
#include <cstdio>
#include "monitor_communicator.hh"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../common/debug.hh"
#include "../protocol/nodelist/getprimarylistreply.hh"
#include "../protocol/nodelist/getsecondarylistreply.hh"

using namespace std;

/**
 * Constructor
 */

MonitorCommunicator::MonitorCommunicator() {

}

/**
 * Destructor
 */

MonitorCommunicator::~MonitorCommunicator() {

}

void MonitorCommunicator::replyPrimaryList(uint32_t requestId, uint32_t sockfd, vector<uint32_t> primaryList){
	GetPrimaryListReplyMsg* getPrimaryListReplyMsg = new GetPrimaryListReplyMsg(this, requestId, sockfd, primaryList);
	getPrimaryListReplyMsg->prepareProtocolMsg();

	addMessage(getPrimaryListReplyMsg);
	return;
}

void MonitorCommunicator::replySecondaryList(uint32_t requestId, uint32_t sockfd, vector<struct SegmentLocation> secondaryList){
	GetSecondaryListReplyMsg* getSecondaryListReplyMsg = new GetSecondaryListReplyMsg(this, requestId, sockfd, secondaryList);
	getSecondaryListReplyMsg->prepareProtocolMsg();

	addMessage(getSecondaryListReplyMsg);
	return;
}
