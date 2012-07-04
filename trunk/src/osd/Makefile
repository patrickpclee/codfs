CC=			g++
CFLAGS=		-g -Wall
LDFLAGS=
OBJ=		osd_communicator.o osd.o
EXE=		osd

all: ${OBJ}
	${CC} ${OBJ} -o ${EXE} ${CFLAGS} ${LDFLAGS}
	
clean:
	rm -f ${OBJ} ${EXE}
