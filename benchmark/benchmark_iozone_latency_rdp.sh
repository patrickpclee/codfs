#!/bin/bash

ncvfshome=/home/ncsgroup/shb118/ncvfs/trunk
scripthome=/home/ncsgroup/shb118/ncvfs/script
minOsd=6
maxOsd=6
minClient=1
maxClient=1

echo "Checking out PLR branch" >> benchmark.log
(cd $ncvfshome; git reset --hard HEAD; git checkout bm_reserve)
cp /tmp/selectionmodule.cc $ncvfshome/src/monitor/
cp -r ./script/* $scripthome
sed "s/<codingScheme>.*<\/codingScheme>/<codingScheme>5<\/codingScheme>/" -i $scripthome/xml/clientconfig.xml
sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>16M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>4<\/C_N>/<C_N>4<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>2<\/C_K>/<C_K>2<\/C_K>/" -i $scripthome/xml/clientconfig.xml
sed "s/.*RESERVE_SPACE_SIZE.*/#define RESERVE_SPACE_SIZE 4194304/" -i $ncvfshome/src/common/define.hh
sed "s/#define USE_FSYNC/\/\/#define USE_FSYNC/" -i $ncvfshome/src/common/define.hh
sed "s/.*MAX_NUM_PROCESSING_SEGMENT.*/#define MAX_NUM_PROCESSING_SEGMENT 1/" -i $ncvfshome/src/common/define.hh
sed "s/.*USLEEP_DURATION.*/#define USLEEP_DURATION 1000/" -i $ncvfshome/src/common/define.hh
cp ./iozone $ncvfshome
(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
echo "Code compiled" >> benchmark.log

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_latency.sh PLR64_4M_RDP $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

sed "s/.*RESERVE_SPACE_SIZE.*/#define RESERVE_SPACE_SIZE 8388608/" -i $ncvfshome/src/common/define.hh
(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)


echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_latency.sh PLR64_8M_RDP $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

sed "s/.*RESERVE_SPACE_SIZE.*/#define RESERVE_SPACE_SIZE 12582912/" -i $ncvfshome/src/common/define.hh
(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_latency.sh PLR64_12M_RDP $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

sed "s/.*RESERVE_SPACE_SIZE.*/#define RESERVE_SPACE_SIZE 16777216/" -i $ncvfshome/src/common/define.hh
(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_latency.sh PLR64_16M_RDP $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

##########################

echo "Checking out FO branch" >> benchmark.log
(cd $ncvfshome; git reset --hard HEAD; git checkout bm_overwrite_inplace)
cp /tmp/selectionmodule.cc $ncvfshome/src/monitor/
cp -r ./script/* $scripthome
sed "s/<codingScheme>.*<\/codingScheme>/<codingScheme>5<\/codingScheme>/" -i $scripthome/xml/clientconfig.xml
sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>16M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>4<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>2<\/C_K>/" -i $scripthome/xml/clientconfig.xml
sed "s/#define USE_FSYNC/\/\/#define USE_FSYNC/" -i $ncvfshome/src/common/define.hh
sed "s/.*MAX_NUM_PROCESSING_SEGMENT.*/#define MAX_NUM_PROCESSING_SEGMENT 1/" -i $ncvfshome/src/common/define.hh
sed "s/.*USLEEP_DURATION.*/#define USLEEP_DURATION 1000/" -i $ncvfshome/src/common/define.hh
cp ./iozone $ncvfshome
(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
echo "Code compiled" >> benchmark.log

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_latency.sh FO64_RDP $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

##########################

echo "Checking out PL branch" >> benchmark.log
(cd $ncvfshome; git reset --hard HEAD; git checkout bm_overwrite_append)
cp /tmp/selectionmodule.cc $ncvfshome/src/monitor/
cp -r ./script/* $scripthome
sed "s/<codingScheme>.*<\/codingScheme>/<codingScheme>5<\/codingScheme>/" -i $scripthome/xml/clientconfig.xml
sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>16M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>4<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>2<\/C_K>/" -i $scripthome/xml/clientconfig.xml
sed "s/#define USE_FSYNC/\/\/#define USE_FSYNC/" -i $ncvfshome/src/common/define.hh
sed "s/.*MAX_NUM_PROCESSING_SEGMENT.*/#define MAX_NUM_PROCESSING_SEGMENT 1/" -i $ncvfshome/src/common/define.hh
sed "s/.*USLEEP_DURATION.*/#define USLEEP_DURATION 1000/" -i $ncvfshome/src/common/define.hh
cp ./iozone $ncvfshome
(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
echo "Code compiled" >> benchmark.log

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_latency.sh PL64_RDP $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

##########################

echo "Checking out FL branch" >> benchmark.log
(cd $ncvfshome; git reset --hard HEAD; git checkout bm_lfs)
cp /tmp/selectionmodule.cc $ncvfshome/src/monitor/
cp -r ./script/* $scripthome
sed "s/<codingScheme>.*<\/codingScheme>/<codingScheme>5<\/codingScheme>/" -i $scripthome/xml/clientconfig.xml
sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>16M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>4<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>2<\/C_K>/" -i $scripthome/xml/clientconfig.xml
sed "s/#define USE_FSYNC/\/\/#define USE_FSYNC/" -i $ncvfshome/src/common/define.hh
sed "s/.*MAX_NUM_PROCESSING_SEGMENT.*/#define MAX_NUM_PROCESSING_SEGMENT 1/" -i $ncvfshome/src/common/define.hh
sed "s/.*USLEEP_DURATION.*/#define USLEEP_DURATION 1000/" -i $ncvfshome/src/common/define.hh
cp ./iozone $ncvfshome
(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
echo "Code compiled" >> benchmark.log

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_latency.sh FL64_RDP $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

echo "$(date): ALL FINISHED" >> benchmark.log
