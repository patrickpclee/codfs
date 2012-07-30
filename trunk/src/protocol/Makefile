CC=			g++
CFLAGS=		-g -Wall -std=c++0x
LDFLAGS=	-lpthread
SRC=		*.cc	

all: ${SRC}
	make message
	${CC} -c ${SRC} ${CFLAGS} ${LDFLAGS}

message: ${PROTO_SRC}
	protoc -I=. --cpp_out=. ./message.proto

mds: ${SRC}
	${CC} -c ${SRC} ${CFLAGS} ${LDFLAGS} -DCOMPILE_FOR_MDS

client: ${SRC}
	${CC} -c ${SRC} ${CFLAGS} ${LDFLAGS} -DCOMPILE_FOR_CLIENT
	
monitor: ${SRC}
	${CC} -c ${SRC} ${CFLAGS} ${LDFLAGS} -DCOMPILE_FOR_MONITOR
	
osd: ${SRC}
	${CC} -c ${SRC} ${CFLAGS} ${LDFLAGS} -DCOMPILE_FOR_OSD
	
clean:
	rm -f *.o
	
