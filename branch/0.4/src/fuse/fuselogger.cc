#include "fuselogger.hh"
#include "stdio.h"
#include "../common/debug.hh"
using namespace std;

FuseLogger::FuseLogger(string filename, const char* mode) {
	_filePtr = fopen(filename.c_str(), mode);
	if(_filePtr == NULL) {
		perror("fopen()");
	}
}

void FuseLogger::logRead (uint32_t clientId, uint64_t segmentId, uint64_t offset, uint32_t length) {
	fprintf(_filePtr,"[%s] {READ} Client: %" PRIu32 ", Segment: %" PRIu64 " - %" PRIu64 ":%" PRIu32 "\n",getTime().c_str(), clientId, segmentId, offset, length);
}

FuseLogger::~FuseLogger() {
	fclose(_filePtr);
}
