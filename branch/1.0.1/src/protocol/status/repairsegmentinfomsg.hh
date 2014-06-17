/**
 * repairsegmentinfo.hh
 */

#ifndef _REPAIR_SEGMENT_INFO_HH_
#define _REPAIR_SEGMENT_INFO_HH_

#include <string>
#include <vector>
#include "../message.hh"

#include "../../common/enums.hh"
#include "../../common/metadata.hh"

using namespace std;

/**
 * Extends the Message class
 * Contains information to repair a segment
 * Structure : { segmentId, deadBlockIds, newOsdIds }
 */

class RepairSegmentInfoMsg: public Message {
public:

	/**
	 * Default Constructor
	 *
	 * @param	communicator	Communicator the Message belongs to
	 */

	RepairSegmentInfoMsg (Communicator* communicator);

	/**
	 * Constructor - Save parameters in private variables
	 *
	 * @param	communicator	Communicator the Message belongs to
	 * @param	sockfd			Socket descriptor
	 * @param	segmentId		SegmentId that needs to repair
	 * @param	deadBlockIds	List of dead block locations
	 * @param	newOsdIds		List of new osd to put the blocks
	 */

	RepairSegmentInfoMsg (Communicator* communicator, uint32_t sockfd, uint64_t
		segmentId, vector<uint32_t> deadBlockIds, vector<uint32_t> newOsdIds);

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
	uint64_t _segmentId;
	vector<uint32_t> _deadBlockIds;
	vector<uint32_t> _newOsdIds;
};

#endif
