CC=			g++
CFLAGS=		-g -Wall
LDFLAGS=
OBJ=		client_communicator.o client.o	
EXE=		client

all: ${OBJ}
	${CC} ${OBJ} -o ${EXE} ${CFLAGS} ${LDFLAGS}
	
clean:
	rm -f ${OBJ} ${EXE}
