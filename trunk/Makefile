CURRENT = $(CURDIR)
SRC=				${CURRENT}/src
OSD_DIR=			${SRC}/osd
MONITOR_DIR=		${SRC}/monitor
CLIENT_DIR=			${SRC}/client
FUSE_DIR=			${SRC}/fuse
MDS_DIR=			${SRC}/mds
COMMON_DIR=			${SRC}/common
PROTOCOL_DIR=		${SRC}/protocol
BENCHMARK_DIR=		${SRC}/benchmark


all:
	make mds;
	make client;
	make osd;
	make monitor;

optimized:
	make mds_o3;
	make client_o3;
	make osd_o3;
	make monitor_o3;

mds:
	cd ${MDS_DIR}; make -j3

client:
	cd ${CLIENT_DIR}; make -j3

osd:
	cd ${OSD_DIR}; make -j3

monitor:
	cd ${MONITOR_DIR}; make -j3

mds_o3:
	cd ${MDS_DIR}; make EXTRA_CFLAGS="-std=c++0x" OPTIMIZE="-O3" -j3

client_o3:
	cd ${CLIENT_DIR}; make EXTRA_CFLAGS="-std=c++0x" OPTIMIZE="-O3" -j3

osd_o3:
	cd ${OSD_DIR}; make EXTRA_CFLAGS="-std=c++0x" OPTIMIZE="-O3" -j3

monitor_o3:
	cd ${MONITOR_DIR}; make EXTRA_CFLAGS="-std=c++0x" OPTIMIZE="-O3" -j3

clientfuse:
	cd ${FUSE_DIR}; make -j3

enum:
	cd ${COMMON_DIR}; make

protocol:
	cd ${PROTOCOL_DIR}; make

benchmark:
	cd ${BENCHMARK_DIR}; make -j3

clean:
	cd ${OSD_DIR}; make clean
	cd ${MONITOR_DIR}; make clean
	cd ${CLIENT_DIR}; make clean
	cd $(FUSE_DIR); make clean
	cd ${MDS_DIR}; make clean
