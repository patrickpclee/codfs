#include "statmodule.hh"
#include "../common/onlineosd.hh"
#include "../common/debug.hh"
#include "../protocol/status/newosdregistermsg.hh"
#include <ctime>


/*  Constructor */
StatModule::StatModule(map<uint32_t, struct OsdStat>& mapRef):
	_osdStatMap(mapRef) { 

}

void StatModule::updateOsdStatMap (Communicator* communicator, uint32_t
	updatePeriod) {

	while (1) {
		printf("-----------------10s start----------------\n");
		{
			lock_guard<mutex> lk(osdStatMapMutex);
			for(auto& entry: _osdStatMap) {
				if (entry.second.osdHealth == ONLINE) {

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
		}
		sleep(updatePeriod);
		printf("-----------------10s finish--------------\n");
	}

}

void StatModule::removeStatById (uint32_t osdId) {
	lock_guard<mutex> lk(osdStatMapMutex);
	_osdStatMap.erase(osdId);
}

void StatModule::removeStatBySockfd (uint32_t sockfd) {
	// Set to OFFLINE
	lock_guard<mutex> lk(osdStatMapMutex);
	map<uint32_t, struct OsdStat>::iterator p;
	p = _osdStatMap.begin();
	while (p != _osdStatMap.end()) {
		if (p->second.osdSockfd == sockfd)
			p->second.osdHealth = OFFLINE;
			//_osdStatMap.erase(p++);
			//else
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


void StatModule::getOsdStatus(vector<uint32_t>& osdListRef, 
	vector<bool>& osdStatusRef) {

	osdStatusRef.clear();
	lock_guard<mutex> lk(osdStatMapMutex);
	for (uint32_t id: osdListRef) {
		map<uint32_t, struct OsdStat>::iterator it = _osdStatMap.find(id);
		if (it != _osdStatMap.end()) {
			if (it->second.osdHealth == ONLINE)
				osdStatusRef.push_back(true);
			else 
				osdStatusRef.push_back(false);
		} else {
			osdStatusRef.push_back(false);
		}
	}
}
