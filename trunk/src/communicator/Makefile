CC=			g++
CFLAGS=		-g -Wall -std=c++0x
LDFLAGS=	-lpthread
SRC=		communicator.cc connection.cc socket.cc

all: ${SRC}
	${CC} -c ${SRC} ${CFLAGS} ${LDFLAGS}
	
clean:
	rm -f *.o
