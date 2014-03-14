all: replay_reserve divide msr_check_size

DFLAG=-DPROMPT_NEW_DISK -DPROMPT_CONT -DPROMPT_END
CXX=g++
CFLAG=-std=c++0x -Wall -Wextra
LDFLAG=-lrt -pthread 
 
doUpdate: doUpdate.cc Makefile
	g++ -std=c++0x -o doUpdate doUpdate.cc -pthread

replay_default: replay.cpp Makefile
	$(CXX) $(CFLAG) -DWRITE_ONLY -o replay -lm replay.cpp $(LDFLAG)

replay_reserve: replay.cpp Makefile
	$(CXX) $(CFLAG) -DWRITE_ONLY -DPROMPT_NEW_DISK -o replay -lm replay.cpp $(LDFLAG)

replay_test: replay_test.cpp Makefile
	$(CXX) $(CFLAG) -g -DWRITE_ONLY -DPROMPT_NEW_DISK -o replay_test -lm replay_test.cpp $(LDFLAG)

msr_check_size: replay.cpp Makefile
	$(CXX) $(CFLAG) -DCHECK_SIZE -o msr_check_size -lm replay.cpp $(LDFLAG)

divide: divide.cpp divide.hpp Makefile
	$(CXX) $(CFLAG) -o divide divide.cpp

clean:
	rm divide replay msr_check_size 
