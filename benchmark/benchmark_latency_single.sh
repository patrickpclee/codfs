#!/bin/bash

ncvfshome=/home/ncsgroup/shb118/ncvfs/trunk
scripthome=/home/ncsgroup/shb118/ncvfs/script
minOsd=6
maxOsd=6
minClient=1
maxClient=1

function copy_plr_files {
    cp ~/tmp/selectionmodule.cc $ncvfshome/src/monitor/
    cp ~/tmp/osd.cc.plr $ncvfshome/src/osd/osd.cc
    cp ~/tmp/storagemodule.cc.plr $ncvfshome/src/osd/storagemodule.cc
    cp ~/tmp/storagemodule.hh.plr $ncvfshome/src/osd/storagemodule.hh
    cp ~/tmp/filelrucache.hh $ncvfshome/src/osd/
    cp ~/tmp/memorypool.cc $ncvfshome/src/common/memorypool.cc
    cp -r ./script/* $scripthome
    cp ./script/fill_disk_single.sh $ncvfshome
    cp ./iozone $ncvfshome
}

function copy_fo_files {
    cp ~/tmp/selectionmodule.cc $ncvfshome/src/monitor/
    cp ~/tmp/osd.cc.fo $ncvfshome/src/osd/osd.cc
    cp ~/tmp/storagemodule.cc.fo $ncvfshome/src/osd/storagemodule.cc
    cp ~/tmp/storagemodule.hh.fo $ncvfshome/src/osd/storagemodule.hh
    cp ~/tmp/filelrucache.hh $ncvfshome/src/osd/
    cp ~/tmp/memorypool.cc $ncvfshome/src/common/memorypool.cc
    cp -r ./script/* $scripthome
    cp ./script/fill_disk_single.sh $ncvfshome
    cp ./iozone $ncvfshome
}

function set_xml {
    sed "s/<codingScheme>.*<\/codingScheme>/<codingScheme>7<\/codingScheme>/" -i $scripthome/xml/clientconfig.xml
    sed "s/<segmentSize>.*<\/segmentSize>/<segmentSize>16M<\/segmentSize>/" -i $scripthome/xml/clientconfig.xml
    sed "s/<C_N>.*<\/C_N>/<C_N>4<\/C_N>/" -i $scripthome/xml/clientconfig.xml
    sed "s/<C_K>.*<\/C_K>/<C_K>2<\/C_K>/" -i $scripthome/xml/clientconfig.xml
    sed "s/#define USE_FSYNC/\/\/#define USE_FSYNC/" -i $ncvfshome/src/common/define.hh
    sed "s/.*MAX_NUM_PROCESSING_SEGMENT.*/#define MAX_NUM_PROCESSING_SEGMENT 1/" -i $ncvfshome/src/common/define.hh
    sed "s/.*USLEEP_DURATION.*/#define USLEEP_DURATION 1000/" -i $ncvfshome/src/common/define.hh
}

for (( i=0; i<1; i++ ))
do

    echo "Checking out PLR branch" >> benchmark.log
    (cd $ncvfshome; git reset --hard HEAD; git checkout bm_reserve)
    copy_plr_files
    set_xml
    sed "s/.*RESERVE_SPACE_SIZE.*/#define RESERVE_SPACE_SIZE 4194304/" -i $ncvfshome/src/common/define.hh
    (cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
    echo "Code compiled" >> benchmark.log

    echo "$(date): Start benchmark" >> benchmark.log
    ./benchmark_branch_latency_single.sh PLR64_4M_FS $minOsd $maxOsd $minClient $maxClient $i
    ./benchmark_branch_latency_merge.sh PLR64_4M_FS $minOsd $maxOsd $minClient $maxClient 4194304 $i
    echo "$(date): End benchmark" >> benchmark.log

    #sed "s/.*RESERVE_SPACE_SIZE.*/#define RESERVE_SPACE_SIZE 8388608/" -i $ncvfshome/src/common/define.hh
    #(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)

    #echo "$(date): Start benchmark" >> benchmark.log
    #./benchmark_branch_latency_single.sh PLR64_8M_FS $minOsd $maxOsd $minClient $maxClient $i
    #./benchmark_branch_latency_merge.sh PLR64_8M_FS $minOsd $maxOsd $minClient $maxClient 8388608 $i
    #echo "$(date): End benchmark" >> benchmark.log

    #sed "s/.*RESERVE_SPACE_SIZE.*/#define RESERVE_SPACE_SIZE 12582912/" -i $ncvfshome/src/common/define.hh
    #(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)

    #echo "$(date): Start benchmark" >> benchmark.log
    #./benchmark_branch_latency_single.sh PLR64_12M_FS $minOsd $maxOsd $minClient $maxClient $i
    #./benchmark_branch_latency_merge.sh PLR64_12M_FS $minOsd $maxOsd $minClient $maxClient 12582912 $i
    #echo "$(date): End benchmark" >> benchmark.log

    #sed "s/.*RESERVE_SPACE_SIZE.*/#define RESERVE_SPACE_SIZE 16777216/" -i $ncvfshome/src/common/define.hh
    #(cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)

    #echo "$(date): Start benchmark" >> benchmark.log
    #./benchmark_branch_latency_single.sh PLR64_16M_FS $minOsd $maxOsd $minClient $maxClient $i
    #./benchmark_branch_latency_merge.sh PLR64_16M_FS $minOsd $maxOsd $minClient $maxClient 16777216 $i
    #echo "$(date): End benchmark" >> benchmark.log

    ##########################

    echo "Checking out FO branch" >> benchmark.log
    (cd $ncvfshome; git reset --hard HEAD; git checkout bm_overwrite_inplace)
    copy_fo_files
    set_xml
    (cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
    echo "Code compiled" >> benchmark.log

    echo "$(date): Start benchmark" >> benchmark.log
    ./benchmark_branch_latency_single.sh FO64_FS $minOsd $maxOsd $minClient $maxClient $i
    echo "$(date): End benchmark" >> benchmark.log

    ###########################

    exit
    exit
    exit
    exit

    echo "Checking out PL branch" >> benchmark.log
    (cd $ncvfshome; git reset --hard HEAD; git checkout bm_overwrite_append)
    copy_fo_files
    set_xml
    (cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
    echo "Code compiled" >> benchmark.log

    echo "$(date): Start benchmark" >> benchmark.log
    ./benchmark_branch_latency_single.sh PL64_FS $minOsd $maxOsd $minClient $maxClient $i
    echo "$(date): End benchmark" >> benchmark.log

    ###########################

    echo "Checking out FL branch" >> benchmark.log
    (cd $ncvfshome; git reset --hard HEAD; git checkout bm_lfs)
    copy_fo_files
    set_xml
    (cd $scripthome; ./removecode.sh; ./compilencvfs.sh; ./configncvfs.sh;)
    echo "Code compiled" >> benchmark.log

    echo "$(date): Start benchmark" >> benchmark.log
    ./benchmark_branch_latency_single.sh FL64_FS $minOsd $maxOsd $minClient $maxClient $i
    echo "$(date): End benchmark" >> benchmark.log

    echo "$(date): ALL FINISHED" >> benchmark.log

done
