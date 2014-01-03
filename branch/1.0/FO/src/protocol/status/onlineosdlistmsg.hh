#ifndef __ONLINEOSDLISTMSG_HH__
#define __ONLINEOSDLISTMSG_HH__

#include <vector>
#include "../message.hh"
#include "../../common/onlineosd.hh"
using namespace std;

/**
 * Extends the Message class
 * Initiate an segment upload
 */

class OnlineOsdListMsg: public Message {
public:

	OnlineOsdListMsg(Communicator* communicator);

	OnlineOsdListMsg(Communicator* communicator, uint32_t dstSockfd,
			vector<struct OnlineOsd>& onlineOsdListRef);

	/**
	 * Copy values in private variables to protocol message
	 * Serialize protocol message and copy to private variable
	 */

	void prepareProtocolMsg();

	/**
	 * Override
	 * Parse message from raw buffer
	 * @param buf Raw buffer storing header + protocol + payload
	 */

	void parse(char* buf);

	/**
	 * Override
	 * Execute the corresponding Processor
	 */

	void doHandle();

	/**
	 * Override
	 * DEBUG: print protocol message
	 */

	void printProtocol();

private:
	vector<struct OnlineOsd>& _onlineOsdListRef;
	vector<struct OnlineOsd>  _onlineOsdList;
};

#endif
