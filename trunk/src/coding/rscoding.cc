#include <sstream>
#include <iostream>
#include <algorithm>
#include <set>
#include <string.h>
#include "coding.hh"
#include "rscoding.hh"
#include "../common/debug.hh"
#include "../common/segmentdata.hh"
#include "../common/objectdata.hh"
#include "../common/memorypool.hh"

extern "C" {
#include "../../lib/jerasure/jerasure.h"
#include "../../lib/jerasure/reed_sol.h"
}

using namespace std;

RSCoding::RSCoding() {

}

RSCoding::~RSCoding() {

}

vector<struct SegmentData> RSCoding::encode(struct ObjectData objectData,
		string setting) {

	vector<struct SegmentData> segmentDataList;
	vector<uint32_t> params = getParams(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	const uint32_t w = params[2];
	const uint32_t size = roundTo(objectData.info.objectSize, k) / k;

	if (k <= 0 || m <= 0 || (w != 8 && w != 16 && w != 32)
			|| (w <= 16 && k + m > (1 << w))) {
		cerr << "Bad Parameters" << endl;
		exit(-1);
	}

	int *matrix = reed_sol_vandermonde_coding_matrix(k, m, w);

	char **data, **code;
	data = talloc<char*, uint32_t>(k);

	for (uint32_t i = 0; i < k; i++) {
		struct SegmentData segmentData;
		segmentData.info.objectId = objectData.info.objectId;
		segmentData.info.segmentId = i;
		segmentData.info.segmentSize = size;

		segmentData.buf = MemoryPool::getInstance().poolMalloc(size);
		char* bufPos = objectData.buf + i * size;
		memcpy(segmentData.buf, bufPos, size);

		segmentDataList.push_back(segmentData);

		data[i] = talloc<char, uint32_t>(size);
		memcpy(data[i], segmentData.buf, size);
	}

	code = talloc<char*, uint32_t>(m);
	for (uint32_t i = 0; i < m; i++) {
		code[i] = talloc<char, uint32_t>(size);
	}

	jerasure_matrix_encode(k, m, w, matrix, data, code, size);

	for (uint32_t i = 0; i < m; i++) {
		struct SegmentData segmentData;
		segmentData.info.objectId = objectData.info.objectId;
		segmentData.info.segmentId = k + i;
		segmentData.info.segmentSize = size;

		segmentData.buf = MemoryPool::getInstance().poolMalloc(size);
		memcpy(segmentData.buf, code[i], size);

		segmentDataList.push_back(segmentData);
	}

	// free memory
	for (uint32_t i = 0; i < k; i++) {
		tfree(data[i]);
	}
	tfree(data);

	for (uint32_t i = 0; i < m; i++) {
		tfree(code[i]);
	}
	tfree(code);

	return segmentDataList;
}

struct ObjectData RSCoding::decode(vector<struct SegmentData> &segmentData,
		vector<uint32_t> &requiredSegments, uint32_t objectSize,
		string setting) {

	vector<uint32_t> params = getParams(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	const uint32_t w = params[2];
	const uint32_t size = roundTo(objectSize, k) / k;

	if (requiredSegments.size() < k) {
		cerr << "Not enough segments for decode " << requiredSegments.size()
				<< endl;
		exit(-1);
	}

	struct ObjectData objectData;

	// copy objectID from first available segment
	objectData.info.objectId = segmentData[requiredSegments[0]].info.objectId;
	objectData.info.objectSize = objectSize;
	objectData.buf = MemoryPool::getInstance().poolMalloc(objectSize);

	set<uint32_t> requiredSegmentsSet(requiredSegments.begin(),
			requiredSegments.end());

	if (requiredSegments.size() != k + m) {
		int *matrix = reed_sol_vandermonde_coding_matrix(k, m, w);
		char **data, **code;
		int *erasures;
		int j = 0;

		data = talloc<char*, uint32_t>(k);
		code = talloc<char*, uint32_t>(m);
		erasures = talloc<int, uint32_t>(k + m - requiredSegments.size() + 1);

		for (uint32_t i = 0; i < k + m; i++) {
			i < k ? data[i] = talloc<char, uint32_t>(size) : code[i - k] =
							talloc<char, uint32_t>(size);
			if (requiredSegmentsSet.count(i) > 0) {
				i < k ? memcpy(data[i], segmentData[i].buf, size) : memcpy(
								code[i - k], segmentData[i].buf, size);
			} else {
				erasures[j++] = i;
			}
		}
		erasures[j] = -1;

		jerasure_matrix_decode(k, m, w, matrix, 1, erasures, data, code, size);

		for (uint32_t i = 0; i < k + m - requiredSegments.size(); i++) {
			struct SegmentData temp;
			temp.info.objectId = objectData.info.objectId;
			temp.info.segmentId = erasures[i];
			temp.info.segmentSize = size;

			temp.buf = MemoryPool::getInstance().poolMalloc(size);
			memcpy(temp.buf,
					(uint32_t) erasures[i] < k ?
							data[erasures[i]] : code[erasures[i] - k], size);

			segmentData[erasures[i]] = temp;
		}

		for (uint32_t i = 0; i < m; i++) {
			MemoryPool::getInstance().poolFree(segmentData[k + i].buf);
		}

		// free memory
		for (uint32_t i = 0; i < k; i++) {
			tfree(data[i]);
		}
		tfree(data);

		for (uint32_t i = 0; i < m; i++) {
			tfree(code[i]);
		}
		tfree(code);

	}

	requiredSegments.clear();
	for (uint32_t i = 0; i < k; i++) {
		requiredSegments.push_back(i);
	}

	uint64_t offset = 0;
	for (uint32_t i = 0; i < k; i++) {
		memcpy(objectData.buf + offset, segmentData[i].buf,
				segmentData[i].info.segmentSize);
		offset += segmentData[i].info.segmentSize;
	}

	return objectData;
}

vector<uint32_t> RSCoding::getRequiredSegmentIds(string setting,
		vector<bool> secondaryOsdStatus) {

	// if more than one in secondaryOsdStatus is false, return {} (error)
	int failedOsdCount = (int) count(secondaryOsdStatus.begin(),
			secondaryOsdStatus.end(), false);

	// for raid 5, only requires n-1 stripes (noOfDataStripes) to decode
	vector<uint32_t> params = getParams(setting);
	const uint32_t k = params[0];
	const uint32_t m = params[1];
	const uint32_t noOfDataStripes = k + m;
	vector<uint32_t> requiredSegments;
	requiredSegments.reserve(noOfDataStripes);

	if ((uint32_t) failedOsdCount > m) {
		return {};
	}

	for (uint32_t i = 0; i < noOfDataStripes; i++) {
		if (secondaryOsdStatus[i] != false) {
			requiredSegments.push_back(i);
		}
	}

	return requiredSegments;
}

//
// PRIVATE FUNCTION
//

vector<uint32_t> RSCoding::getParams(string setting) {
	vector<uint32_t> params(3);
	int i = 0;
	string token;
	stringstream stream(setting);
	while (getline(stream, token, ':')) {
		istringstream(token) >> params[i++];
	}
	return params;
}
