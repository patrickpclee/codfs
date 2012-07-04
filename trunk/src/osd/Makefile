CC=			g++
CFLAGS=		-g -Wall
LDFLAGS=
COMM_OBJ=	../communicator/communicator.o
OBJ=		osd_communicator.o osd.o
EXE=		osd

all: ${OBJ}
	${CC} ${OBJ} ${COMM_OBJ} -o ${EXE} ${CFLAGS} ${LDFLAGS}
	
clean:
	rm -f ${OBJ} ${EXE}
