#ifndef __NAMESPACE_MODULE_HH__
#define __NAMESPACE_MODULE_HH__

#include <stdint.h>

class NameSpaceModule {
public:
	uint32_t createFile (char path[]);
	uint32_t deleteFile (char path[]);
	void listFolder (char path[]);
private:
};
#endif
