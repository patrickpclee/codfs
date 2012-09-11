#include "statmodule.hh"
#include "../osd/onlineosd.hh"
#include "../protocol/status/newosdregistermsg.hh"
#include "../common/debug.hh"
#include <ctime>


/*  Constructor */
StatModule::StatModule(map<uint32_t, struct OsdStat>& mapRef):
	_osdStatMap(mapRef) { 

}

void StatModule::updateOsdStatMap (Communicator* communicator) {
	while (1) {
		printf("-----------------10s start----------------\n");
		{
			lock_guard<mutex> lk(osdStatMapMutex);
			for(auto& entry: _osdStatMap) {
				printf("Entry %3d:\n",entry.first);
				printf("		 "); 
				entry.second.out();			

				OsdStatUpdateRequestMsg* requestMsg = new 
				  OsdStatUpdateRequestMsg(communicator, entry.second.osdSockfd);
				requestMsg -> prepareProtocolMsg();
				// Do not need to wait for reply.
				communicator -> addMessage (requestMsg);
			}
		}
		sleep(10);
		printf("-----------------10s finish--------------\n");
	}

}

void StatModule::removeStatById (uint32_t osdId) {
	lock_guard<mutex> lk(osdStatMapMutex);
	_osdStatMap.erase(osdId);
}

void StatModule::removeStatBySockfd (uint32_t sockfd) {
	lock_guard<mutex> lk(osdStatMapMutex);
	debug_yellow("Try to delete sockfd = %" PRIu32 "\n", sockfd);
	map<uint32_t, struct OsdStat>::iterator p;
	p = _osdStatMap.begin();
	while (p != _osdStatMap.end()) {
		if (p->second.osdSockfd == sockfd)
			_osdStatMap.erase(p++);
		else
			p++;
	}	
	debug_yellow("Delete sockfd = %" PRIu32 "\n", sockfd);
}

void StatModule::setStatById (uint32_t osdId, uint32_t sockfd, 
	uint32_t capacity, uint32_t loading, enum OsdHealthStat health) {
	map<uint32_t, struct OsdStat>::iterator iter;

	lock_guard<mutex> lk(osdStatMapMutex);
	iter = _osdStatMap.find(osdId);
	if (iter == _osdStatMap.end()) {
		_osdStatMap[osdId] =OsdStat(osdId, sockfd, capacity, loading, health,
		time(NULL));
	} else {
		iter->second.osdSockfd = sockfd;
		iter->second.osdCapacity = capacity;
		iter->second.osdLoading = loading;
		iter->second.osdHealth = health;
		iter->second.timestamp = time(NULL);
	}
}

void StatModule::setStatById (uint32_t osdId, uint32_t sockfd, 
	uint32_t capacity, uint32_t loading, enum OsdHealthStat health, uint32_t ip,
	uint16_t port) {
	map<uint32_t, struct OsdStat>::iterator iter;

	lock_guard<mutex> lk(osdStatMapMutex);
	iter = _osdStatMap.find(osdId);
	if (iter == _osdStatMap.end()) {
		_osdStatMap[osdId] =OsdStat(osdId, sockfd, capacity, loading, health,
		ip, port, time(NULL));
	} else {
		iter->second.osdSockfd = sockfd;
		iter->second.osdCapacity = capacity;
		iter->second.osdLoading = loading;
		iter->second.osdHealth = health;
		iter->second.osdIp = ip;
		iter->second.osdPort = port;
		iter->second.timestamp = time(NULL);
	}
}



void StatModule::getOnlineOsdList(vector<struct OnlineOsd>& list) {
	list.clear();
	{
		lock_guard<mutex> lk(osdStatMapMutex);
		for (auto& entry: _osdStatMap) {
			if (entry.second.osdHealth == ONLINE) 
				list.push_back (OnlineOsd (entry.first, 
					entry.second.osdIp, entry.second.osdPort));
		}
	}
}

void StatModule::broadcastNewOsd(Communicator* communicator,
	uint32_t osdId, uint32_t ip, uint32_t port) {

	lock_guard<mutex> lk(osdStatMapMutex);
	for (auto& entry: _osdStatMap) {
		if (entry.second.osdHealth == ONLINE) {

			NewOsdRegisterMsg* newOsdRegisterMsg = new NewOsdRegisterMsg(
				communicator, entry.second.osdSockfd, osdId, ip, port);
			newOsdRegisterMsg->prepareProtocolMsg();

			communicator->addMessage(newOsdRegisterMsg);
		}
	}
}
