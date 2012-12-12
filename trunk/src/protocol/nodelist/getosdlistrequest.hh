#ifndef __GET_OSD_LIST_REQUEST_HH__
#define __GET_OSD_LIST_REQUEST_HH__

#include <string>
#include "../message.hh"

#include "../../common/onlineosd.hh"
#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Request to list files in a directory from MDS
 */

class GetOsdListRequestMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	GetOsdListRequestMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	numOfObjs	number of Segments
	 * @param	mdsSockfd	Socket descriptor
	 */

	GetOsdListRequestMsg (Communicator* communicator, uint32_t osdSockfd);

	/**
	 * Copy values in private variables to protocol message
	 * Serialize protocol message and copy to private variable
	 */

	void prepareProtocolMsg ();

	/**
	 * Override
	 * Parse message from raw buffer
	 * @param buf Raw buffer storing header + protocol + payload
	 */

	void parse (char* buf);

	/**
	 * Override
	 * Execute the corresponding Processor
	 */

	void doHandle ();

	/**
	 * Override
	 * DEBUG: print protocol message
	 */

	void printProtocol ();

	void setOsdList(vector<struct OnlineOsd>& _osdList, 
		vector<struct OnlineOsd>& osdList);

	vector<struct OnlineOsd>& getOsdList();

private:
	vector<struct OnlineOsd> _osdList;
};

#endif
