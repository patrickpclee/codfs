/*
 * convertor.cc
 */

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string.h>
#include "convertor.hh"

std::string md5ToHex(unsigned char* hash) {
	std::string result;
	result.reserve(32); // C++11 only, otherwise ignore

	for (std::size_t i = 0; i != 16; ++i) {
		result += "0123456789abcdef"[hash[i] / 16];
		result += "0123456789abcdef"[hash[i] % 16];
	}

	return result;
}

uint64_t stringToByte(std::string sizeString) {
	uint64_t byte;
	char unit = toupper((int) *sizeString.rbegin());

	if (unit < 'A' || unit > 'Z') {
		return boost::lexical_cast<uint64_t>(sizeString);
	}

	byte = boost::lexical_cast<uint64_t>(
			sizeString.substr(0, sizeString.size() - 1));

	switch (unit) {
	case 'T':
		return byte * 1024 * 1024 * 1024 * 1024;
		break;
	case 'G':
		return byte * 1024 * 1024 * 1024;
		break;
	case 'M':
		return byte * 1024 * 1024;
		break;
	case 'K':
		return byte * 1024;
		break;
	default:
		std::cerr << "casting error!" << std::endl;
		exit (-1);
	}
}
