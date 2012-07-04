#ifndef __CACHE_HH__
#define __CACHE_HH__
class Cache {
Public:
	readCache (uint32_t id);
	writeCache (uint32_t id, char[] data);
	createEntry (uint32_t id);
	deleteEntry (uint32_t id);
Private:
}
#endif
