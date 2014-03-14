#!/bin/bash

ncvfshome=/home/ncsgroup/shb118/ncvfs/trunk
scripthome=/home/ncsgroup/shb118/ncvfs/script
minOsd=10
maxOsd=10
minClient=10
maxClient=10

echo "Checking out PLR branch" >> benchmark.log
(cd $ncvfshome; git reset --hard HEAD; git checkout bm_reserve)
#(cd $ncvfshome; git reset --hard HEAD; git checkout f3a90a83e4f2488662dee1385ab21eb302a13003)
cp -r ./script/* $scripthome
sed "s/<codingScheme>.*<\/codingScheme>/<codingScheme>7<\/codingScheme>/" -i $scripthome/xml/clientconfig.xml
sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>16M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>4<\/C_N>/<C_N>4<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>2<\/C_K>/<C_K>2<\/C_K>/" -i $scripthome/xml/clientconfig.xml
sed "s/.*RESERVE_SPACE_SIZE.*/#define RESERVE_SPACE_SIZE 4194304/" -i $ncvfshome/src/common/define.hh
sed "s/#define USE_FSYNC/\/\/#define USE_FSYNC/" -i $ncvfshome/src/common/define.hh
cp ./iozone $ncvfshome
(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
echo "Code compiled" >> benchmark.log

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_reserve_overhead.sh PLR64 $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

###

sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>16M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>4<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>4<\/C_K>/" -i $scripthome/xml/clientconfig.xml
(cd $scripthome; ./configncvfs.sh;)

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_reserve_overhead.sh PLR84 $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

##


sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>24M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>6<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>2<\/C_K>/" -i $scripthome/xml/clientconfig.xml
(cd $scripthome; ./configncvfs.sh;)

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_reserve_overhead.sh PLR86 $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

##########################

echo "Checking out FO branch" >> benchmark.log
#(cd $ncvfshome; git reset --hard HEAD; git checkout eb7788b561d954cb1c088f22aaa989745d6b4627)
(cd $ncvfshome; git reset --hard HEAD; git checkout bm_overwrite_inplace)
cp -r ./script/* $scripthome
sed "s/<codingScheme>.*<\/codingScheme>/<codingScheme>7<\/codingScheme>/" -i $scripthome/xml/clientconfig.xml
sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>16M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>4<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>2<\/C_K>/" -i $scripthome/xml/clientconfig.xml
sed "s/#define USE_FSYNC/\/\/#define USE_FSYNC/" -i $ncvfshome/src/common/define.hh
cp ./iozone $ncvfshome
(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
echo "Code compiled" >> benchmark.log

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_reserve_overhead.sh FO64 $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

###

sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>16M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>4<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>4<\/C_K>/" -i $scripthome/xml/clientconfig.xml
(cd $scripthome; ./configncvfs.sh;)

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_reserve_overhead.sh FO84 $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

###

sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>24M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>6<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>2<\/C_K>/" -i $scripthome/xml/clientconfig.xml
(cd $scripthome; ./configncvfs.sh;)

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_reserve_overhead.sh FO86 $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

##########################

echo "Checking out PL branch" >> benchmark.log
(cd $ncvfshome; git reset --hard HEAD; git checkout bm_overwrite_append)
cp -r ./script/* $scripthome
sed "s/<codingScheme>.*<\/codingScheme>/<codingScheme>7<\/codingScheme>/" -i $scripthome/xml/clientconfig.xml
sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>16M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>4<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>2<\/C_K>/" -i $scripthome/xml/clientconfig.xml
sed "s/#define USE_FSYNC/\/\/#define USE_FSYNC/" -i $ncvfshome/src/common/define.hh
cp ./iozone $ncvfshome
(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
echo "Code compiled" >> benchmark.log

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_reserve_overhead.sh PL64 $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

###

sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>16M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>4<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>4<\/C_K>/" -i $scripthome/xml/clientconfig.xml
(cd $scripthome; ./configncvfs.sh;)

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_reserve_overhead.sh PL84 $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

###

sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>24M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>6<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>2<\/C_K>/" -i $scripthome/xml/clientconfig.xml
(cd $scripthome; ./configncvfs.sh;)

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_reserve_overhead.sh PL86 $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

##########################

echo "Checking out FL branch" >> benchmark.log
(cd $ncvfshome; git reset --hard HEAD; git checkout bm_lfs)
cp -r ./script/* $scripthome
sed "s/<codingScheme>.*<\/codingScheme>/<codingScheme>7<\/codingScheme>/" -i $scripthome/xml/clientconfig.xml
sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>16M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>4<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>2<\/C_K>/" -i $scripthome/xml/clientconfig.xml
sed "s/#define USE_FSYNC/\/\/#define USE_FSYNC/" -i $ncvfshome/src/common/define.hh
cp ./iozone $ncvfshome
(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
echo "Code compiled" >> benchmark.log

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_reserve_overhead.sh FL64 $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

###

sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>16M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>4<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>4<\/C_K>/" -i $scripthome/xml/clientconfig.xml
(cd $scripthome; ./configncvfs.sh;)

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_reserve_overhead.sh FL84 $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

###

sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>24M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_N>.*<\/C_N>/<C_N>6<\/C_N>/" -i $scripthome/xml/clientconfig.xml
sed "s/<C_K>.*<\/C_K>/<C_K>2<\/C_K>/" -i $scripthome/xml/clientconfig.xml
(cd $scripthome; ./configncvfs.sh;)

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_iozone_reserve_overhead.sh FL86 $minOsd $maxOsd $minClient $maxClient
echo "$(date): End benchmark" >> benchmark.log

echo "$(date): ALL FINISHED" >> benchmark.log
