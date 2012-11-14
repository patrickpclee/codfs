#ifndef __RS_CODING_HH__
#define __RS_CODING_HH__

#include "coding.hh"

class RSCoding: public Coding {
public:

	RSCoding();
	~RSCoding();

	vector<struct SegmentData> encode(struct ObjectData objectData,
			string setting);

	struct ObjectData decode(vector<struct SegmentData> &segmentData,
			vector<uint32_t> &requiredSegments, uint32_t objectSize,
			string setting);

	vector<uint32_t> getRequiredSegmentIds(string setting,
			vector<bool> secondaryOsdStatus);

	vector<uint32_t> getRepairSrcSegmentIds(string setting,
			vector<uint32_t> failedSegments, vector<bool> segmentStatus);

	vector<struct SegmentData> repairSegments(
			vector<struct SegmentData> &repairSrcSegments,
			vector<uint32_t> &repairSrcSegmentId, uint32_t objectSize,
			string setting);

	static string generateSetting(uint32_t k, uint32_t m, uint32_t w) {
		return to_string(k)+":"+to_string(m)+":"+to_string(w);
	}

private:
	vector<uint32_t> getParams(string setting);
};

#endif
