/**
 * listdirectoryrequest.hh
 */

#ifndef __LISTDIRECTORYREQUESTHH__
#define __LISTDIRECTORYREQUESTHH__

#include <string>
#include "../common/enums.hh"
#include "message.hh"

using namespace std;

/**
 * Extends the Message class
 * Request to list files in a directory from MDS
 */

class ListDirectoryRequestMsg : public Message {
public:
	ListDirectoryRequestMsg(uint32_t osdId, string directoryPath, uint32_t mdsSockfd);
	void prepareProtocolMsg();
	void printProtocol();

	void parse (char* buf);

	void handle ();
private:
	uint32_t _osdId;
	string _directoryPath;
};

#endif
