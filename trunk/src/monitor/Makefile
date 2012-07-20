CC=			g++
CFLAGS=		-g -Wall -std=c++0x
LDFLAGS=	`pkg-config --cflags --libs protobuf` -lpthread
COMMON_OBJ=	../common/*.o
COMM_OBJ=	../communicator/*.o
CONFIG_OBJ=	../config/*.o
PROTO_OBJ=	../protocol/*.o
EXE=		MONITOR

all:
	pkg-config --cflags protobuf
	cd ../common; make; cd ../monitor
	cd ../config; make; cd ../monitor
	cd ../communicator; make; cd ../monitor
	cd ../protocol; make; cd ../monitor
	${CC} ${CFLAGS} -c *.cc
	${CC} *.o ${COMMON_OBJ} ${COMM_OBJ} ${CONFIG_OBJ} ${PROTO_OBJ} -o ${EXE} ${CFLAGS} ${LDFLAGS}
	cp ${EXE} ../../
	
clean:
	cd ../config; make clean; cd ../osd
	cd ../communicator; make clean; cd ../osd
	cd ../protocol; make clean; cd ../osd
	rm -f ${OBJ} ${EXE}
