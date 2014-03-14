#!/bin/bash

ncvfshome=/home/ncsgroup/shb118/ncvfs/trunk
scripthome=/home/ncsgroup/shb118/benchmark/script
loghome=/home/ncsgroup/shb118/benchmark/log
minOsd=10
maxOsd=10
minClient=10
maxClient=10
## iozone, msr
setting="msr"


config() {
    sed "s/<codingScheme>.*<\/codingScheme>/<codingScheme>5<\/codingScheme>/" -i $scripthome/xml/clientconfig.xml
    #sed "s/#define USE_FSYNC/\/\/#define USE_FSYNC/" -i $ncvfshome/src/common/define.hh
}

rm $loghome/benchmark.log

cd $scripthome/../

echo "----- Benchmark Type: $setting -----"
echo "Checking out overwrite (APPEND_PARITY) branch" >> $loghome/benchmark.log
## checkout the working branch for benchmarking
(cd $ncvfshome; git reset --hard HEAD; git checkout bm_overwrite_append)
sed "s/.*RESERVE_SPACE_SIZE.*/#define RESERVE_SPACE_SIZE 16777216/" -i $ncvfshome/src/common/define.hh
config
## use scripts from THIS benchmark directory
(cd $scripthome; ./stopncvfs.sh; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
echo "Code compiled" >> benchmark.log

## exec the looping script for this branch
echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_msr.sh PL $minOsd $maxOsd $minClient $maxClient
echo -e "$(date): Benchmark finished\n" >> benchmark.log


echo "Checking out overwrite (INPLACE_PARITY) branch" >> benchmark.log
(cd $ncvfshome; git reset --hard HEAD; git checkout bm_overwrite_inplace)
config
(cd $scripthome; ./stopncvfs.sh; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
echo "Code compiled" >> benchmark.log

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_msr.sh FO $minOsd $maxOsd $minClient $maxClient
echo -e "$(date): Benchmark finished\n" >> benchmark.log


echo "Checking out reserve branch" >> benchmark.log
(cd $ncvfshome; git reset --hard HEAD; git checkout bm_reserve)
config
(cd $scripthome; ./stopncvfs.sh; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
echo "Code compiled" >> benchmark.log

echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_msr.sh PLR $minOsd $maxOsd $minClient $maxClient
echo -e "$(date): Benchmark finished\n" >> benchmark.log


echo "Checking out lfs branch" >> benchmark.log
(cd $ncvfshome; git reset --hard HEAD; git checkout bm_lfs)
config
(cd $scripthome; ./stopncvfs.sh; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
echo "Code compiled" >> benchmark.log


echo "$(date): Start benchmark" >> benchmark.log
./benchmark_branch_msr.sh FL $minOsd $maxOsd $minClient $maxClient
echo -e "$(date): Benchmark finished\n" >> benchmark.log

