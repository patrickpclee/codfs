CURRENT = $(CURDIR)
SRC=				${CURRENT}/src
OSD_DIR=			${SRC}/osd
MONITOR_DIR=		${SRC}/monitor
CLIENT_DIR=			${SRC}/client
MDS_DIR=			${SRC}/mds


all:
	make mds;
	make client;
	make osd;
	make monitor;

mds:
	cd ${MDS_DIR}; make

client:
	cd ${CLIENT_DIR}; make

osd:
	cd ${OSD_DIR}; make

monitor:
	cd ${MONITOR_DIR}; make

clean:
	cd ${OSD_DIR}; make clean
	cd ${MONITOR_DIR}; make clean
	cd ${CLIENT_DIR}; make clean
	cd ${MDS_DIR}; make clean
