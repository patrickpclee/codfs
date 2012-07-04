#ifndef __NAMESPACE_MODULE_HH__
#define __NAMESPACE_MODULE_HH__

#include <stdint.h>
#include <string>

class NameSpaceModule {
public:
	uint32_t createFile (string path);
	uint32_t deleteFile (string path);
	void listFolder (string path);
private:
};
#endif
