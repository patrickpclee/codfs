#ifndef __CACHE_HH__
#define __CACHE_HH__

#include <stdint.h>
#include <exception>
using namespace std;

class Cache {
public:
	Cache();
	virtual ~Cache();
	virtual char * read(uint64_t id) = 0;
	virtual void write(uint64_t id, char* data) = 0;
	virtual void deleteEntry(uint64_t id) = 0;
private:
};
#endif
