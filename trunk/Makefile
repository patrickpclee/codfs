SRC=				./src
OSD_DIR=			${SRC}/osd
MONITOR_DIR=		${SRC}/monitor
CLIENT_DIR=		${SRC}/client
MDS_DIR=			${SRC}/mds

all:
	cd ${OSD_DIR}; make
	cd ${MONITOR_DIR}; make
	cd ${CLIENT_DIR}; make
	cd ${MDS_DIR}; make
	
clean:
	cd ${OSD_DIR}; make clean
	cd ${MONITOR_DIR}; make clean
	cd ${CLIENT_DIR}; make clean
	cd ${MDS_DIR}; make clean
