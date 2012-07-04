CC=			g++
CFLAGS=		-g -Wall
LDFLAGS=
OBJ=		monitor.o recoverymodule.o statmodule.o selectionmodule.o
EXE=		monitor

all: ${OBJ}
	${CC} ${OBJ} -o ${EXE} ${CFLAGS} ${LDFLAGS}
	
clean:
	rm -f ${OBJ} ${EXE}
