OPTIMIZE := -O2
WARNINGS := -Wall 
EXTRA_CFLAGS := -std=c++0x -DDEBUG=0 -D_FILE_OFFSET_BITS=64

############################################

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
CODING_TESTER_DIR=		${SRC}/coding_tester


default:
	make mds;
	make client;
	make osd;
	make monitor;
	make benchmark;
	make clientfuse;

all:
	make mds;
	make client;
	make osd;
	make monitor;
	make clientfuse;
	make benchmark;

mds:
	cd ${MDS_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

client:
	cd ${CLIENT_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

osd:
	cd ${OSD_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

monitor:
	cd ${MONITOR_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

codingtester:
	cd ${CODING_TESTER_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

clientfuse:
	cd ${FUSE_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

enum:
	cd ${COMMON_DIR}; make

protocol:
	cd ${PROTOCOL_DIR}; make

benchmark:
	cd ${BENCHMARK_DIR}; make OPTIMIZE="$(OPTIMIZE)" WARNINGS="$(WARNINGS)" EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -j3

clean:
	cd ${OSD_DIR}; make clean
	cd ${MONITOR_DIR}; make clean
	cd ${CLIENT_DIR}; make clean
	cd $(FUSE_DIR); make clean
	cd ${MDS_DIR}; make clean
	cd ${CODING_TESTER_DIR}; make clean
	cd ${BENCHMARK_DIR}; make clean
