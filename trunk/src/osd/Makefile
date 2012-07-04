CC=			g++
CFLAGS=		-g -Wall
LDFLAGS=
SRC=		osd.cc
OBJS=		$(SRC:.cc=.o)
EXE=		osd

all: $(SRC) $(EXE)
	
$(EXE): $(OBJS) 
	$(CC) -c $(LDFLAGS) $(OBJS) -o $@

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@
	
clean:
	rm -f *.o