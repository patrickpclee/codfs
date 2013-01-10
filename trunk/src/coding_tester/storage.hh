/*
 * storage.hh
 */

#ifndef STORAGE_HH_
#define STORAGE_HH_

#include <vector>
#include "../common/define.hh"

char* readFile(std::string filepath, uint32_t &filesize);
char* readFile(std::string filepath, uint32_t &filesize,
		std::vector<offset_length_t> offsetLengths);
void writeFile(std::string filepath, char* buf, uint32_t length);

#endif /* STORAGE_HH_ */
