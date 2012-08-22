#ifndef __CLIENTSTORAGEMODULE_HH__
#define __CLIENTSTORAGEMODULE_HH__

#include <stdint.h>
#include <vector>
#include <fstream>
#include "../common/objectdata.hh"
#include "../osd/storagemodule.hh"

using namespace std;

class ClientStorageModule {
public:
	ClientStorageModule();
	~ClientStorageModule();
	uint64_t getFilesize (string filepath);
	uint32_t getObjectCount (string filepath);
	struct ObjectData readObjectFromFile (string filepath, uint32_t objectIndex);
	uint64_t getObjectSize ();

	void createObjectCache(uint64_t objectId, uint32_t length);
	uint32_t writeObjectCache (uint64_t objectId, char* buf, uint64_t offsetInObject, uint32_t length);
	bool locateObjectCache(uint64_t objectId);
	void closeObject(uint64_t objectId);
	struct ObjectCache getObjectCache(uint64_t objectId);

	FILE* createAndOpenFile(string filepath);
	uint32_t writeFile(FILE* file, string filepath, char* buf, uint64_t offset, uint32_t length);
	void closeFile (FILE* filePtr);

private:
	uint64_t _objectSize;
	map <uint64_t, struct ObjectCache> _objectCache;
	string _objectFolder;
};

#endif
