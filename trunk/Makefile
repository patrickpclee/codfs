CURRENT = $(CURDIR)
SRC=				${CURRENT}/src
OSD_DIR=			${SRC}/osd
MONITOR_DIR=		${SRC}/monitor
CLIENT_DIR=			${SRC}/client
MDS_DIR=			${SRC}/mds


all:
#	make mds;
	make client;
	make osd;
#	make monitor;

mds:
	cd ${MDS_DIR}; make -j6

client:
	cd ${CLIENT_DIR}; make -j6

osd:
	cd ${OSD_DIR}; make -j6

monitor:
	cd ${MONITOR_DIR}; make -j6

clean:
	cd ${OSD_DIR}; make clean
	cd ${MONITOR_DIR}; make clean
	cd ${CLIENT_DIR}; make clean
	cd ${MDS_DIR}; make clean
