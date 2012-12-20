/**
 * getsecondarylistrequest.hh
 */

#ifndef __GET_SECONDARY_LIST_REQUEST_HH__
#define __GET_SECONDART_LIST_REQUEST_HH__

#include <string>
#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Request to list files in a directory from MDS
 */

class GetSecondaryListRequestMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	GetSecondaryListRequestMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	numOfObjs	number of Segments
	 * @param	mdsSockfd	Socket descriptor
	 */

	GetSecondaryListRequestMsg (Communicator* communicator, uint32_t osdSockfd, 
		uint32_t numOfSegs, uint32_t primaryId, uint64_t blockSize);

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

	void setSecondaryList(vector<struct BlockLocation> secondaryList);
	vector<BlockLocation> getSecondaryList();


private:
	uint32_t _numOfSegs;
	uint32_t _primaryId;
	uint64_t _blockSize;
	vector<struct BlockLocation> _secondaryList;
};

#endif
