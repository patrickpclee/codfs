#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <boost/lexical_cast.hpp>
#include "divide.hpp"

Divide::Divide(char* in, char* out, int cn):
		largestoffset(0){
	linenumber = 0;
	CLIENT_NUM = cn;
	// open input file
	inputfname = in;
	input.open(in, std::fstream::in);
	if (!input.is_open()) { 
		std::cout << "Cannot open input file" << std::endl;
		exit(-1);
	}
	// get the number of traces in file
	getlineno();
	// may include more segments than actual ones, but never get less
	unsigned long long tmpoffset = linenumber / CLIENT_NUM ;
	if (linenumber % CLIENT_NUM) {
		tmpoffset += 1;
	}
	sid.sort(sortbyoffset);
#ifdef DEBUG
	for (std::list<Rec>::iterator it = sid.begin(); it != sid.end(); ++it) {
		printf("[%llu] %llu\n",(*it).id,(*it).offset);
	}
#endif
	output = new std::fstream[CLIENT_NUM];
	for (int i = 0; i < CLIENT_NUM; ++i) {

		// open all output files 
		std::string filename(out);
		filename.append(".");
		filename.append(std::to_string(i));
		output[i].open(filename,std::fstream::out);
		if (!output[i].is_open()) {
			std::cout << "Warning: Cannot open output file " << i << std::endl;
		}

		// find the upperbound on segment id this client will deal with
		long long upperbound = tmpoffset * (i+1);
		upperbounds.push_back(upperbound);
#ifdef DEBUG
		std::cout << "upperbound[" << i << "] " << upperbound << std::endl;
#endif

	}
	upperoffset = new unsigned long long[CLIENT_NUM]();
	// adjust the upperbound
	std::list<long long>::iterator up = upperbounds.begin();
	int boundcnt = 0;
	unsigned long long j=0;
	unsigned long long prevbound = 0;
	for (std::list<Rec>::iterator it = sid.begin(); it != sid.end(); ++it,++j) {
		if ((unsigned)*up <= prevbound) {
			*up = (linenumber - prevbound)/(CLIENT_NUM - boundcnt);
			while (true) {
				unsigned long long pseg = (*it).offset/SEG_SIZE;
				if (++it == sid.end()) {
					*up = j+1;
					break;
				} else if (pseg != (*it).offset/SEG_SIZE) {
					*up = ++j;
					break;
				}
				++j;
			}
		}
		if (it == sid.end()) { break; }
		// check if bound overlap
		// is bound
		if (j == (unsigned)*up) {
			// save the reference, so don't need to loop the record again
			std::list<Rec>::iterator cur = it;
			bool giveup = false;
			while ((*it).offset/SEG_SIZE == (*++it).offset/SEG_SIZE) {
				// give up the current request to next client
				*up -= 1;
				// request[0].offset / SEG = request[1].offset / SEG 
				// -> no more to give up
				if (*up == 0) { 
					*up = -1; 
					std::cout << "upperbound [ ] " << *up << std::endl;
					break; 
				}
				// it pointing to next, get it pointing to prev (next-2)
				--it;--it;
				// then check if prev = cur, if again, give up request to next client
				giveup=true;
			}
			if (giveup && *up != -1) { --it; }
			// restore reference, j does not change

			// giveup all request? 
            // this->record_upperbound = prev->record_upperbound
			//unsigned long long uoffset;
			if (((unsigned)*up <= prevbound) && (*up != -1)) {
				// should use the original position which start to give up request for faster search
				it = cur;
				while (((*it).offset/SEG_SIZE) == ((*++it).offset/SEG_SIZE)) {
					 ++j; 
				}
#ifdef DEBUG
                printf("(*up <= prevbound) && (*up != -1) %llu\n",j);
#endif
				*up = j;
				--it;
				// reset the last request processed 
				cur = it;
			}
			if (*up == -1) {
				upperoffset[boundcnt] = ((*it).offset/SEG_SIZE-1)*SEG_SIZE-1;
			} else {
				// last one may not have upperoffset, but ignored in replay
				upperoffset[boundcnt] = ((*it).offset/SEG_SIZE+1)*SEG_SIZE-1;
			}

#ifdef DEBUG
			std::cout << "it " << (*it).offset << " id " << (*it).id << std::endl;
			std::cout << "upperbound[ ] " << *up << " upperoffset[ ]" 
					<< upperoffset[boundcnt]<< std::endl;
#endif
			// restore the position pointer for getting next request
			it = cur;

			if (*up == -1) {
				prevbound = 0;
			} else {
				prevbound = *up;
			}
			// until next one is not the same, check next client
			++up;
			++boundcnt;
		}
	}
	// manually add the read request to ensure the partial disk size
	for (int i = 0; i < CLIENT_NUM; ++i) {
		unsigned long long toffset = 0;
		if (i != 0) {
			// this one's begining offset should continue the previous one
			toffset = upperoffset[i-1]+1;
		}
		output[i] << "R," << toffset << ",0" << std::endl;
        if (i != CLIENT_NUM-1) {
    		output[i] << "R," << upperoffset[i] << ",0" << std::endl;
        }
	}
}

Divide::~Divide() {
	if (input.is_open()) {
		input.close();
	}
	for (int i=0; i < CLIENT_NUM; ++i) {
		if (output[i].is_open()) 
			output[i].close();
	}
}

void Divide::run() {
	std::vector<int> clientid;
	std::cout << "start parse the trace" << std::endl;
	int linecnt = 0;
	int boundcnt = 0;
	std::list<long long>::iterator bound = upperbounds.begin();
	for (std::list<Rec>::iterator it = sid.begin(); it != sid.end(); ++it,++linecnt) {
		while ((*bound == -1) && (boundcnt+1 != CLIENT_NUM)) {
			++bound;
			++boundcnt;
			std::cout << "bound is -1, next " << *bound << " " << boundcnt << "offset " << upperoffset[boundcnt] << std::endl;
		}
		unsigned long long offset = (*it).offset;
		// if the request accross two segments and last accessed byte is beyond 
		// upperoffset of current client
		// for last client, it will take the remaining trace anyway
		unsigned long long start = offset;
		unsigned long long end = (offset+(*it).len-1);
		if ((start/SEG_SIZE != end/SEG_SIZE) && (boundcnt+1 != CLIENT_NUM) 
				&& (end > upperoffset[boundcnt])) {
			output[boundcnt] << (*it).type << "," << offset << "," << (offset/SEG_SIZE+1)*SEG_SIZE - offset << std::endl;
			output[boundcnt+1] << (*it).type << "," << (unsigned long long)((offset/SEG_SIZE+1)*SEG_SIZE) << "," << offset + (*it).len - (offset/SEG_SIZE+1)*SEG_SIZE << std::endl;
		} else {
			output[boundcnt] << (*it).type << "," << (*it).offset << "," << (*it).len << std::endl;
		}
		if (linecnt == *bound) {
			printf("linecnt: %d\n",linecnt);
			++bound;
			++boundcnt;
		}
	}
	printf("sid size: %u\n",sid.size());
}

int Divide::parseline(char* line, unsigned long long* offset, unsigned long long* len, char* type) {
	char *field = NULL;
    int cnt = 1;
    input.getline(line,4096);

	std::ios::iostate rd = input.rdstate();
	if ((rd && std::ifstream::failbit) || (rd && std::ifstream::eofbit) 
			|| (rd && std::ifstream::badbit) ) {
		return 2;
	}
#ifdef DEBUG
    //printf("%s\n",line);
#endif
    field = strtok(line,",");
    while (field != NULL) {
        if (cnt == 2) {
            *offset = boost::lexical_cast<unsigned long long>(std::string(field));
        } else if (cnt == 3) {
            *len = boost::lexical_cast<unsigned long long>(std::string(field));
        } else if (cnt == 1) {
            *type = field[0];
        }
        ++cnt;
        field = strtok(NULL,",");
    }
	// not enough field provided, i.e. invalid format
    if (cnt < 4) { return -1; }
	return 0;
}

std::vector<int> Divide::getoutfileid(unsigned long long offset,unsigned long long len) {
	int startid = -1;
	int endid = -1;
	int found = 0;
	std::vector<int> cid;
	int i = 0;
	for (std::list<long long>::iterator it = upperbounds.begin(); it != upperbounds.end() && found != 2; ++it,++i) {
		// find the start client id
		if ((unsigned)(*it)*SEG_SIZE-1 >= offset && startid == -1) {
			startid = i;
			++found;
		}
		// find the end client id
		if ((unsigned)(*it)*SEG_SIZE-1 >= offset+len && endid == -1) {
			endid = i;
			++found;
		}
	}
	if (startid == -1 || endid == -1) {	std::cout << offset << "," << len << " & " << found << std::endl; exit (-1); }
	for (int i = startid; i <= endid; ++i) {
		cid.push_back(i);
	}
	return cid;
}

unsigned long long Divide::getlargestoffset() {
	unsigned long long offset;
	unsigned long long len;
	char type;
	char line[4096];
	int ret = -1;
	// go through each line to find the greatest offset accessed
	for (ret = parseline(line,&offset,&len,&type); ret != 2; ret = parseline(line,&offset,&len,&type)) {
		if (ret == -1) { continue; }
		for (int i = 0; i < CLIENT_NUM; ++i) {
			if (largestoffset < offset+len) {
				largestoffset = offset+len;
			}  
		}
	}
	std::cout << "largestoffset: " << largestoffset << std::endl;
	// reset input stream
	input.close();
	input.open(inputfname,std::fstream::in);
	return largestoffset;
}

unsigned long long Divide::getlineno() {
	char line[4096];
	linenumber = 0;
	unsigned long long offset, len;
	char type;
	int ret = parseline(line,&offset,&len,&type);
	while(ret != -1 && ret != 2) {
		saveoffset(linenumber,offset,len,type);
		linenumber++;
		ret = parseline(line,&offset,&len,&type);
	}
	// reset input stream
	input.close();
    input.open(inputfname,std::fstream::in);
    return linenumber;
}

void Divide::saveoffset(int id, unsigned long long offset, unsigned long long len, char type) {
	unsigned long long segmentid = offset / SEG_SIZE;
	if (offset % SEG_SIZE) { ++segmentid; }
	Rec rec;
	rec.id = id;
	rec.offset = offset;
	rec.len = len;
	rec.type = type;
	sid.push_back(rec);
}


bool sortbyoffset(Rec reca, Rec recb) {
	return reca.offset/SEG_SIZE < recb.offset/SEG_SIZE;
}

int main (int argc, char** argv) {
	if (argc != 4) {
		std::cout << "Usage: " << argv[0] << "[input filename] [output filename] [CLIENT_NUM]\n";
		return 0;
	}
	Divide* divide = new Divide(argv[1], argv[2], atoi(argv[3]));
	divide->run();
	return 0;
}

