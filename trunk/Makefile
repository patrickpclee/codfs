CURRENT = $(CURDIR)
SRC=				${CURRENT}/src
OSD_DIR=			${SRC}/osd
MONITOR_DIR=		${SRC}/monitor
CLIENT_DIR=			${SRC}/client
MDS_DIR=			${SRC}/mds


all:
	make clean;
	make libraries;
	make mds;
	make client;
	make osd;
	make monitor;

test:
	make clean; 
	make libraries;
	make mds; 
	make client;
	
mds:
	cd ${MDS_DIR}; make mds

client:
	cd ${CLIENT_DIR}; make client

osd:
	cd ${OSD_DIR}; make osd

monitor:
	cd ${MONITOR_DIR}; make monitor

libraries:
	cd ${SRC}/cache; make; cd ${CURRENT}
	cd ${SRC}/common; make; cd ${CURRENT}
	cd ${SRC}/config; make; cd ${CURRENT}
	cd ${SRC}/communicator; make; cd ${CURRENT}
	cd ${SRC}/protocol; make; cd ${CURRENT}

clean:
	cd ${OSD_DIR}; make clean
	cd ${MONITOR_DIR}; make clean
	cd ${CLIENT_DIR}; make clean
	cd ${MDS_DIR}; make clean
