#include <cstdio>
#include "monitor.hh"
#include "../config/config.hh"

ConfigLayer* configLayer;

int main (void) {
	Monitor* monitor = new Monitor();
	configLayer = new ConfigLayer("monitorconfig.xml");

	printf ("MONITOR\n");

	delete configLayer;
	delete monitor;

	return 0;
}
