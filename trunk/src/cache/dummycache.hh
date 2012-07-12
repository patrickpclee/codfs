#ifndef __DUMMY_CACHE_HH__
#define __DUMMY_CACHE_HH__

#include <stdint.h>

#include "cache.hh"

class DummyCache : public Cache {
	public:
		uint32_t read (uint32_t id);
		uint32_t write (uint32_t id, char* data);
		uint32_t createEntry (uint32_t id);
		void deleteEntry (uint32_t id);
	private:
};

#endif
