#ifndef __CACHE_HH__
#define __CACHE_HH__

#include <stdint.h>
#include <exception>
using namespace std;

class Cache {
	public:
		virtual char * read (uint64_t id){};
		virtual void write (uint64_t id, char* data){};
		//uint32_t createEntry (uint32_t id);
		virtual void deleteEntry (uint64_t id){};
	private:
};
#endif
