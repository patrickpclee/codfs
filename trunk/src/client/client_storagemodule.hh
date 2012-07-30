#ifndef __CLIENTSTORAGEMODULE_HH__
#define __CLIENTSTORAGEMODULE_HH__

#include <vector>
#include "../common/objectdata.hh"

using namespace std;

class ClientStorageModule {
public:
	ClientStorageModule();
	~ClientStorageModule();
	uint32_t getObjectCount (string filepath);
	struct ObjectData readObjectFromFile (string filepath, uint32_t objectIndex);
private:
	uint64_t _objectSize;
};

#endif
