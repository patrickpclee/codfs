CC=			g++
CFLAGS=		-g -Wall
LDFLAGS=
SRC=		client.cc
OBJS=		$(SRC:.cc=.o)
EXE=		client

all: $(SRC) $(EXE)
	
$(EXE): $(OBJS) 
	$(CC) -c $(LDFLAGS) $(OBJS) -o $@

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@
	
clean:
	rm -f *.o