#ifndef __CACHE_HH__
#define __CACHE_HH__

#include <stdint.h>

class Cache {
public:
	uint32_t readCache (uint32_t id);
	uint32_t writeCache (uint32_t id, char data[]);
	uint32_t createEntry (uint32_t id);
	uint32_t deleteEntry (uint32_t id);
private:
};
#endif
