/**
 * repairobjectinfo.hh
 */

#ifndef _REPAIR_OBJECT_INFO_HH_
#define _REPAIR_OBJECT_INFO_HH_

#include <string>
#include <vector>
#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Contains information to repair a object
 * Structure : { objectId, deadSegmentIds, newOsdIds }
 */

class RepairObjectInfoMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	RepairObjectInfoMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	sockfd			Socket descriptor
	 * @param	objectId		ObjectId that needs to repair
	 * @param	deadSegmentIds	List of dead segment locations
	 * @param	newOsdIds		List of new osd to put the segments
	 */

	RepairObjectInfoMsg (Communicator* communicator, uint32_t sockfd, uint64_t
		objectId, vector<uint32_t> deadSegmentIds, vector<uint32_t> newOsdIds);

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

private:
	uint64_t _objectId;
	vector<uint32_t> _deadSegmentIds;
	vector<uint32_t> _newOsdIds;
};

#endif
