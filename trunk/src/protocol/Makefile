CC=			g++
CFLAGS=		-g -Wall
SRC=		listdirectoryrequest.cc message.cc

all: ${SRC}
	${CC} -c ${SRC} ${CFLAGS} ${LDFLAGS}

message: ${PROTO_SRC}
	protoc -I=. --cpp_out=. ./message.proto
	
clean:
	rm -f *.o
	
