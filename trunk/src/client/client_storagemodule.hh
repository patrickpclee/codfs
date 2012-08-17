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
	void writeObjectToFile (string dstPath, struct ObjectData objectData, uint32_t objectIndex);

	struct ObjectCache getObjectCache(uint64_t objectId);
	string writeObject(uint64_t objectId, char* buf, uint64_t offsetInObject, uint32_t length);
	uint32_t writeObjectCache (uint64_t objectId, char* buf, uint64_t offsetInObject, uint32_t length);
	void closeObject(uint64_t objectId);

	string generateObjectPath(uint64_t objectId, string objectFolder);
	uint32_t writeFile(string filepath, char* buf, uint64_t offset, uint32_t length);
	void closeFile (string filepath);
	FILE* openFile(string filepath);

private:
	uint64_t _objectSize;
	map <uint64_t, struct ObjectCache> _objectCache;
	map <string, FILE*> _openedFile;
	string _objectFolder;
};

#endif
