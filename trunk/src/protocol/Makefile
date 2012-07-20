CC=			g++
CFLAGS=		-g -Wall -std=c++0x
SRC=	*.cc	

all: ${SRC}
	${CC} -c ${SRC} ${CFLAGS} ${LDFLAGS}

message: ${PROTO_SRC}
	protoc -I=. --cpp_out=. ./message.proto

mds: 
	${CC} -c ${SRC} ${CFLAGS} ${LDFLAGS} -DCOMPILE_FOR_MDS
	
clean:
	rm -f *.o
	
