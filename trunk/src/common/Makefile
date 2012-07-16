CC=			g++
CFLAGS=		-g -Wall
LDFLAGS=	#`pkg-config --cflags --libs protobuf`
SRC=		memorypool.cc

all: ${SRC}
#	pkg-config --cflags protobuf
	${CC} -c ${SRC} ${CFLAGS} ${LDFLAGS}
	
clean:
	rm -f *.o
