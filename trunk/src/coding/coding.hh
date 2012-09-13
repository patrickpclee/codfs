#ifndef __CODING_HH__
#define __CODING_HH__

#include <vector>
#include "../common/objectdata.hh"
#include "../common/segmentdata.hh"

class Coding {
	public:

		Coding();
		virtual ~Coding();

		virtual vector<struct SegmentData> encode(struct ObjectData objectData,
				string setting) = 0;
		virtual struct ObjectData decode(vector<struct SegmentData> &segmentData,
				vector<uint32_t> &requiredSegments, uint32_t objectSize, string setting) = 0;

		virtual vector<uint32_t> getRequiredSegmentIds(string setting,
				vector<bool> secondaryOsdStatus) = 0;

		static uint32_t roundTo(unsigned int value, unsigned int roundTo);
		static void bitwiseXor (char* result, char* srcA, char* srcB, uint32_t length);
};

#endif
