CC=			g++
CFLAGS=		-g -Wall
LDFLAGS=
SRC=		cache_filemetadata.cc cache_objectmetadata.cc filemetadata.cc \
			mds.cc metadata.cc namespace.cc objectmetadata.cc 
OBJS=		$(SRC:.cc=.o)
EXE=		mds

all: $(SRC) $(EXE)
	
$(EXE): $(OBJS) 
	$(CC) -c $(LDFLAGS) $(OBJS) -o $@

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@
	
clean:
	rm -f *.o