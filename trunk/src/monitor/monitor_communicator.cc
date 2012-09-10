/**
 * monitor _communicator.cc
 * by DING Qian
 * Aug 9, 2012
 */

#include <iostream>
#include <thread>
#include <cstdio>
#include "monitor_communicator.hh"
#include "../config/config.hh"
#include "../common/enums.hh"
#include "../common/memorypool.hh"
#include "../common/debug.hh"
#include "../protocol/nodelist/getprimarylistreply.hh"
#include "../protocol/nodelist/getsecondarylistreply.hh"
#include "../protocol/nodelist/getosdconfigreply.hh"
#include "../protocol/status/onlineosdlistmsg.hh"
#include "../protocol/nodelist/getosdlistreply.hh"

using namespace std;

extern ConfigLayer* configLayer;
mutex osdCountMutex;

/**
 * Constructor
 */

MonitorCommunicator::MonitorCommunicator() {
	_osdCount = 52001;
	_serverPort = configLayer->getConfigInt("Communication>ServerPort");
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

void MonitorCommunicator::replyOsdConfig(uint32_t requestId, uint32_t sockfd){
	GetOsdConfigReplyMsg* getOsdConfigReplyMsg = new GetOsdConfigReplyMsg(this, requestId, sockfd, _osdCount, _osdCount, 10, 5, "./osd_segment/", "./osd_object/");
	getOsdConfigReplyMsg->prepareProtocolMsg();

	addMessage(getOsdConfigReplyMsg);
	lock_guard<mutex> lk(osdCountMutex);
	_osdCount++;
	return;
}

void MonitorCommunicator::sendOnlineOsdList(uint32_t newOsdSockfd, 
		vector<struct OnlineOsd>& onlineOsdList) {
	OnlineOsdListMsg* onlineOsdListMsg = new OnlineOsdListMsg(this, newOsdSockfd, onlineOsdList);
	onlineOsdListMsg->prepareProtocolMsg();
	addMessage(onlineOsdListMsg);
}

void MonitorCommunicator::replyOsdList(uint32_t requestId, uint32_t clientSockfd, 
		vector<struct OnlineOsd>& onlineOsdList) {
	GetOsdListReplyMsg* getOsdListReplyMsg = new GetOsdListReplyMsg(this, requestId, 
		clientSockfd, onlineOsdList);
	getOsdListReplyMsg->prepareProtocolMsg();
	addMessage(getOsdListReplyMsg);
}
