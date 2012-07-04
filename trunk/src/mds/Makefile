CC=			g++
CFLAGS=		-g -Wall
LDFLAGS=
OBJ=		cache_filemetadata.o cache_objectmetadata.o filemetadata.o \
mds_communicator.o mds.o metadata.o namespace.o objectmetadata.o
EXE=		mds

all: ${OBJ}
	${CC} ${OBJ} -o ${EXE} ${CFLAGS} ${LDFLAGS}
	
clean:
	rm -f ${OBJ} ${EXE}
