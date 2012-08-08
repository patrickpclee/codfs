/**
 * garbagecollector.cc
 */

#include <stdlib.h>
#include <list>
#include "garbagecollector.hh"
#include "../common/debug.hh"

GarbageCollector::GarbageCollector() {

}

GarbageCollector::~GarbageCollector() {

}

void GarbageCollector::addToDeleteList(Message* ptr) {
	lock_guard<mutex> lk(_pendingDeleteListMutex);
	_pendingDeleteList.push_back(ptr);
}

// garbage collect every one second

void GarbageCollector::start() {

	list<Message*>::iterator itr;

	while (1) {
		{
			//debug ("%s\n", "Collecting Garbage");
			lock_guard<mutex> lk(_pendingDeleteListMutex);

			for (itr = _pendingDeleteList.begin();
					itr != _pendingDeleteList.end();) {
				Message* message = *itr;
				if (message->isDeletable()) {
					delete message;
					itr = _pendingDeleteList.erase(itr);
				} else {
					++itr;
				}

			}
		}
		sleep(1);
	}
}
