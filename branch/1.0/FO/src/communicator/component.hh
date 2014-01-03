/*
 * component.hh
 */

#ifndef COMPONENT_HH_
#define COMPONENT_HH_

#include "../common/enums.hh"

using namespace std;

struct Component {
	ComponentType type;
	uint32_t id;
	string ip;
	uint16_t port;
};

#endif /* COMPONENT_HH_ */
