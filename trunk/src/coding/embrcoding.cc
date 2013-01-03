#include "coding.hh"
#include "embrcoding.hh"
#include "../common/debug.hh"
#include "../common/blockdata.hh"
#include "../common/segmentdata.hh"
#include "../common/memorypool.hh"

extern "C" {
#include "../../lib/jerasure/jerasure.h"
#include "../../lib/jerasure/reed_sol.h"
}

using namespace std;

EMBRCoding::EMBRCoding(){

}

EMBRCoding::~EMBRCoding(){

}

vector<BlockData> EMBRCoding::encode(SegmentData segmentData, string setting) {
	vector<uint32_t> params = getParameters(setting);
	const uint32_t n = params[0];
	const uint32_t k = params[1];
	const uint32_t w = params[2];

	const uint32_t rs_k = 
}

//
// PRIVATE FUNCTION
//

vector<uint32_t> EMBRCoding::getParameters(string setting) {
	vector<uint32_t> params(3);
	int i = 0;
	string token;
	stringstream stream(setting);
	while (getline(stream, token, ':')) {
		istringstream(token) >> params[i++];
	}
	return params;
}
