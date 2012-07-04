CC=			g++
CFLAGS=		-g -Wall
LDFLAGS=
SRC=		monitor.cc
OBJS=		$(SRC:.cc=.o)
EXE=		mds

all: $(SRC) $(EXE)
	
$(EXE): $(OBJS) 
	$(CC) -c $(LDFLAGS) $(OBJS) -o $@

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@
	
clean:
	rm -f *.o