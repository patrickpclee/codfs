CC=			g++
CFLAGS=		-g -Wall -std=c++0x
LDFLAGS=	`pkg-config --cflags --libs protobuf` -lpthread
CACHE_OBJ=	../cache/*.o
COMMON_OBJ=	../common/*.o
COMM_OBJ=	../communicator/*.o
CONFIG_OBJ=	../config/*.o
PROTO_OBJ=	../protocol/*.o
#STORAGE_OBJ=../storage/*.o
EXE=		MDS

ALL_OBJ = ${CACHE_OBJ} ${COMMON_OBJ} ${COMM_OBJ} ${CONFIG_OBJ} ${PROTO_OBJ}
CURRENT = $(CURDIR)

all:
	make libraries
	make mds

libraries:
	cd ../cache; make; cd ${CURRENT}
	cd ../common; make; cd ${CURRENT}
	cd ../config; make; cd ${CURRENT}
	cd ../communicator; make; cd ${CURRENT}
	cd ../protocol; make; cd ${CURRENT}

mds: *.cc
	${CC} ${LDFLAGS} ${CFLAGS} -c *.cc
	${CC} *.o ${ALL_OBJ} -o ${EXE} ${CFLAGS} ${LDFLAGS}
	cp ${EXE} ../../
	
clean:
	cd ../common; make; cd ${CURRENT}
	cd ../config; make clean; cd ${CURRENT}
	cd ../communicator; make clean; cd ${CURRENT}
	cd ../protocol; make clean; cd ${CURRENT}
	rm -f ${OBJ} ${EXE}
