CC=			g++
CFLAGS=		-g -Wall -std=c++0x
LDFLAGS=	-lpthread
SRC=		$(wildcard *.cc)

all: ${SRC}
	${CC} -c $^ ${CFLAGS} ${LDFLAGS}
	
clean:
	rm -f *.o
