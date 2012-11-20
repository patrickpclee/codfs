/*
 * storage.hh
 */

#ifndef STORAGE_HH_
#define STORAGE_HH_

char* readFile (std::string filepath, uint32_t &filesize);
void writeFile (std::string filepath, char* buf, uint32_t length);

#endif /* STORAGE_HH_ */
