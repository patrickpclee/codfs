#ifndef __CACHE_HH__
#define __CACHE_HH__

#include <stdint.h>
#include <exception>
using namespace std;

class CacheMissException : public exception {
	virtual const char* what() const throw()
	{
		return "Cache Miss";
	}
} CacheMiss;

class Cache {
	public:
		uint32_t read (uint32_t id);
		uint32_t write (uint32_t id, char* data);
		uint32_t createEntry (uint32_t id);
		uint32_t deleteEntry (uint32_t id);
	private:
};
#endif
