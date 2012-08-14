#ifndef __CACHE_HH__
#define __CACHE_HH__

#include <stdint.h>
#include <exception>
#include <sstream>
#include "mm/cache_map.hpp"
#include "../common/metadata.hh"

using namespace std;

template<class T>
class Cache {
public:
	std::string tostring(uint64_t id);

	T * read(uint64_t id);

	void write(uint64_t id, T* data);

	void deleteEntry(uint64_t id);
private:
	mm::cache_map<std::string, T*> _cacheMap;
};
#endif
