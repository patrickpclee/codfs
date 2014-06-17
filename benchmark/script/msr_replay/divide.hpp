#ifndef __DIVIDE_HPP__
#define __DIVIDE_HPP__
#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#define SEG_SIZE 16777216 		// 16M
#define BLOCK_SIZE 4194304		// 16M / 4 = 4M
#define TIME_INT 1
//#define DEBUG 

struct Rec {
	unsigned long long id;
	unsigned long long offset;
	unsigned long long len;
	char type;
	//bool operator() (Rec i, Rec j) { return i.offset <= j.offset; }
};

class Divide {

	public:

		Divide(char* in, char* out, int cn);

		~Divide();

		void run();


	private:
		char* inputfname;
		std::fstream input;
		std::fstream* output;
		// number of client
		int CLIENT_NUM;
		// upper bound for segment id that is held by a client	
		std::list<long long> upperbounds;
		unsigned long long* upperoffset;
		// largest offset accessed in the trace
		unsigned long long largestoffset;
		// total number of request
		unsigned long long linenumber;
		// list of all request
		std::list<Rec> sid;

		int parseline(char* line, unsigned long long* offset, unsigned long long* len, char* type);

		std::vector<int> getoutfileid(unsigned long long offset,unsigned long long len);

		unsigned long long getlargestoffset();

		unsigned long long getlineno();

		void saveoffset(int id, unsigned long long offset, unsigned long long len, char type);

	//	bool sortbyoffset(Rec, Rec);
		
};

static bool sortbyoffset(Rec, Rec);
#endif
