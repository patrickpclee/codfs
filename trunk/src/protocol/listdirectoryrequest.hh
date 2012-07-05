#ifndef __LISTDIRECTORYREQUESTHH__
#define __LISTDIRECTORYREQUESTHH__

#include <string>
#include "message.hh"

using namespace std;

class ListDirectoryRequestMessage : Message {
public:
	ListDirectoryRequestMessage(uint32_t osdId, string directoryPath);
	void prepareProtocolMsg();
private:
	uint32_t _osdId;
	string _directoryPath;
};

#endif
