CC=			g++
CFLAGS=		-g -Wall
LDFLAGS=	`pkg-config --cflags --libs protobuf`
COMM_OBJ=	../communicator/communicator.o ../communicator/connection.o
PROTO_OBJ=	../protocol/listdirectoryrequest.o ../protocol/message.o ../protocol/message.pb.o
OBJ=		osd_communicator.o osd.o
EXE=		osd

all: ${OBJ} ${COMM_OBJ} ${PROTO_OBJ}
	cd ../communicator; make; cd ../osd
	cd ../protocol; make; cd ../osd
	pkg-config --cflags protobuf
	${CC} ${OBJ} ${COMM_OBJ} ${PROTO_OBJ} -o ${EXE} ${CFLAGS} ${LDFLAGS}
	
clean:
	cd ../communicator; make clean; cd ../protocol; make clean; cd ../osd;
	rm -f ${OBJ} ${EXE}
