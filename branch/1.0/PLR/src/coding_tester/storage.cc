/*
 * storage.cc
 */

#include <iostream>
#include <fstream>
#include "storage.hh"
#include "../common/memorypool.hh"
using namespace std;

char* readFile(string filepath, uint32_t &filesize) {

	char* buf;

	// open file
	ifstream file(filepath, ios::in | ios::binary | ios::ate);
	if (!file.is_open()) {
		cerr << "Cannot open read " << filepath << endl;
		exit(-1);
	}

	// read file
	filesize = (uint32_t) file.tellg();

	// allocate space
	buf = MemoryPool::getInstance().poolMalloc(filesize);

	file.seekg(0, ios::beg);

	if (!file.read(buf, filesize)) {
		cerr << "Cannot read " << filepath << endl;
		exit(-1);
	}

	file.close();

	return buf;
}

char* readFile(string filepath, uint32_t &filesize,
		vector<offset_length_t> offsetLengths) {

	char* buf;

	// open file
	ifstream file(filepath, ios::in | ios::binary | ios::ate);
	if (!file.is_open()) {
		cerr << "Cannot open read " << filepath << endl;
		exit(-1);
	}

	uint32_t readSize = 0;
	for (auto offsetLength : offsetLengths) {
		readSize += offsetLength.second;
	}
	filesize = readSize;

	// allocate space
	buf = MemoryPool::getInstance().poolMalloc(readSize);
	//fprintf(stderr,"%p\n",buf);

	uint32_t currentOffset = 0;
	for (auto offsetLength : offsetLengths) {
		file.seekg(offsetLength.first, ios::beg);
		if (!file.read(buf + currentOffset, offsetLength.second)) {
			cerr << "Cannot read " << filepath << " Offset = "
					<< offsetLength.first << " Length = " << offsetLength.second
					<< endl;
			exit(-1);
		}
		currentOffset += offsetLength.second;
	}

	file.close();

	return buf;
}

void writeFile(string filepath, char* buf, uint32_t length) {

	// open file
	ofstream file(filepath, ios::out | ios::binary | ios::trunc);

	if (!file.is_open()) {
		cerr << "Cannot open write " << filepath << endl;
		exit(-1);
	}

	printf("buff: %p, length = %u\n", buf, length);
	if (!file.write(buf, length)) {
		perror ("Write");
		cerr << "Cannot write " << filepath << " " << length << endl;
		exit(-1);
	}

	file.close();

}
