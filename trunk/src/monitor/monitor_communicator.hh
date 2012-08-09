/**
 * monitor_communicator.hh
 * by DING Qian
 * Aug 9, 2012
 */

#ifndef __MONITOR_COMMUNICATOR_HH__
#define __MONITOR_COMMUNICATOR_HH__

#include <iostream>
#include <stdint.h>
#include "../communicator/communicator.hh"
#include "segmentlocationcache.hh"

using namespace std;

/**
 * Extends Communicator class
 * Handles all MONITOR communications
 */

class MonitorCommunicator: public Communicator {
public:
	MonitorCommunicator();
	~MonitorCommunicator();

private:
};

#endif
