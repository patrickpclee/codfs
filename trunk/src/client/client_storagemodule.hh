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

	/**
	 * Constructor
	 */
	ClientStorageModule();

	/**
	 * Destructor
	 */
	~ClientStorageModule();

	/**
	 * get the Size of a file
	 * @param filepath 	Path of the file on storage
	 * @return Size of the file
	 */
	uint64_t getFilesize (string filepath);

	/**
	 * get the number of objects of a file
	 * @param filepath 	Path of the file on storage
	 * @return Number of objects
	 */
	uint32_t getObjectCount (string filepath);

	/**
	 * get the particular object of a file when given an index.
	 * @param filepath 		Path of the file on storage
	 * @param objectIndex 	the index of the object
	 * @return the ObjectData of the object
	 */
	struct ObjectData readObjectFromFile (string filepath, uint32_t objectIndex);

	/**
	 * get the Size of an object
	 *
	 * @return Size of one object
	 */
	uint64_t getObjectSize ();

	/**
	 * Create cache of a object
	 * @param objectId 	object ID
	 * @param length 	length of the data
	 */
	void createObjectCache(uint64_t objectId, uint32_t length);

	/**
	 * write cache of a object
	 * @param objectId 			object ID
	 * @param buf				buffer contains the data
	 * @param offsetInObject	offset of the Object
	 * @param length 			length of the data
	 */
	uint32_t writeObjectCache (uint64_t objectId, char* buf, uint64_t offsetInObject, uint32_t length);

	/**
	 * test whether cache of an object exist
	 * @param objectId 			object ID
	 *
	 * @return true/false
	 */
	bool locateObjectCache(uint64_t objectId);

	/**
	 * Close and remove the cache of an object
	 * @param objectId object ID
	 */
	void closeObject(uint64_t objectId);

	/**
	 * get the Object Cache
	 * @param objectId object ID
	 *
	 * @return struct ObjectCache contains the object info.
	 */
	struct ObjectCache getObjectCache(uint64_t objectId);


	/**
	 * Create a file on disk and open it
	 * @param filepath 	Path of the file on storage
	 * @return Pointer to the opened file
	 */
	FILE* createAndOpenFile(string filepath);

	/**
	 * Open a file and write data from buffer
	 * @param file		file pointer
	 * @param filepath 	Path of the file in the storage
	 * @param buf 		Pointer to source buffer
	 * @param offset 	Offset in the file
	 * @param length 	Length to write
	 * @return Number of bytes written
	 */
	uint32_t writeFile(FILE* file, string filepath, char* buf, uint64_t offset, uint32_t length);

	/**
	 * Close the file and remove it from _openedFile map
	 * @param filePtr 	file Pointer to the file on disk
	 */
	void closeFile (FILE* filePtr);

private:
	uint64_t _objectSize;
	map <uint64_t, struct ObjectCache> _objectCache;
	string _objectFolder;
};

#endif
