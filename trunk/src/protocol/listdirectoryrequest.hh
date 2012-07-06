#ifndef __LISTDIRECTORYREQUESTHH__
#define __LISTDIRECTORYREQUESTHH__

#include <string>
#include "../common/enums.hh"
#include "message.hh"

using namespace std;

class ListDirectoryRequestMessage : public Message {
public:
	ListDirectoryRequestMessage(uint32_t osdId, string directoryPath);
	void prepareProtocolMsg();
	void printProtocol();
private:
	uint32_t _osdId;
	string _directoryPath;
};

#endif
