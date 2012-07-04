#ifndef __SELECTIONMODULE_HH__
#define __SELECTIONMODULE_HH__

#include <stdint.h>

class SelectionModule {
public:
	uint32_t findNextOsd(); // osd to store the next object
	uint32_t* findSecondaryOsdList(uint32_t splitFactor);
	uint32_t findRecoveryOsd(struct OjbectOwnership objectOwnership);
};

#endif
