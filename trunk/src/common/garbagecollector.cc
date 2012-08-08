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

	list<Message*>::iterator p;

	while (1) {
		{
			//debug ("%s\n", "Collecting Garbage");
			lock_guard<mutex> lk(_pendingDeleteListMutex);

			for (p = _pendingDeleteList.begin();
					p != _pendingDeleteList.end();) {
				Message* message = *p;
				if (message->isDeletable()) {
					delete message;
					p = _pendingDeleteList.erase(p);
				} else {
					++p;
				}

			}
		}
		sleep(1);
	}
}
