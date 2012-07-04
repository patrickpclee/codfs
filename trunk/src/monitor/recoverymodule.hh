#ifndef __RECOVERYMODULE_HH__
#define __RECOVERYMODULE_HH__

#include <stdint.h>

class RecoveryModule {
public:
	void reportOsdFailure (uint32_t osdId, uint32_t health);
	uint32_t* getObjectIdList(uint32_t osdId);
	uint32_t* doRecovery (struct ObjectOwnership* objectOwnershipList);
private:
};
#endif
