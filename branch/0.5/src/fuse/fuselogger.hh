#ifndef __FUSE_LOGGER_HH__
#define __FUSE_LOGGER_HH__
#include <string>

class FuseLogger {
	public:
		FuseLogger(std::string filename, const char* mode = "w");
		void logRead (uint32_t clientId, uint64_t segmentId, uint64_t offset, uint32_t length);
		~FuseLogger();
	private:
		FILE* _filePtr;
};

#endif
