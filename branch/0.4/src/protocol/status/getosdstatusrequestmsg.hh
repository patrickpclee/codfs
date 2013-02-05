/**
 * getosdstatusrequestmsg.hh
 */

#ifndef __GET_OSD_STATUS_REQUEST_HH__
#define __GET_OSD_STATUS_REQUEST_HH__

#include <string>
#include <vector>
#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Request to list files in a directory from MDS
 */

class GetOsdStatusRequestMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	GetOsdStatusRequestMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	sockfd			Socket descriptor
	 * @param	osdListRef		vector reference for request osd IDs
	 */

	GetOsdStatusRequestMsg (Communicator* communicator, uint32_t osdSockfd, vector<uint32_t>& osdListRef);

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

	void setOsdStatus(vector<bool>& statusRef);

	vector<bool>& getOsdStatus();

private:
	vector<uint32_t> _osdList;
	vector<bool> _osdStatus;
};

#endif
