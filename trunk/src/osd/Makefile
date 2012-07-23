CC=			g++
CFLAGS=		-g -Wall -std=c++0x
LDFLAGS=	`pkg-config --cflags --libs protobuf` -lpthread
COMMON_OBJ=	../common/*.o
COMM_OBJ=	../communicator/*.o
CONFIG_OBJ=	../config/*.o
PROTO_OBJ=	../protocol/*.o
EXE=		OSD

all:
	pkg-config --cflags protobuf
	cd ../common; make; cd ../osd
	cd ../config; make; cd ../osd
	cd ../communicator; make; cd ../osd
	cd ../protocol; make; cd ../osd
	${CC} ${CFLAGS} -c *.cc
	${CC} *.o ${COMMON_OBJ} ${COMM_OBJ} ${CONFIG_OBJ} ${PROTO_OBJ} -o ${EXE} ${CFLAGS} ${LDFLAGS}
	cp ${EXE} ../../

osd:
	${CC} ${LDFLAGS} ${CFLAGS} -c *.cc

	
clean:
	cd ../config; make clean; cd ../osd
	cd ../communicator; make clean; cd ../osd
	cd ../protocol; make clean; cd ../osd
	rm -f ${OBJ} ${EXE}
