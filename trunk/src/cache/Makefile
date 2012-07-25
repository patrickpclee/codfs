CC=			g++
CFLAGS=		-g -Wall -std=c++0x
LDFLAGS=	-lpthread
SRC=		*.cc

all: ${SRC}
	${CC} -c ${SRC} ${CFLAGS} ${LDFLAGS}
	
clean:
	rm -f *.o
