#!/bin/bash

logpath=/home/ncsgroup/shb118/benchmark/benchmark.log

if [ $# -ne 7 ]
then
    echo '[USAGE] ./benchmark_branch.sh [branchName] [minOsdNum] [maxOsdNum] [minClientNum] [maxClientNum]'
    exit
fi

branchName=$1
minOsdNum=$2
maxOsdNum=$3
minClientNum=$4
maxClientNum=$5
reservesize=$6
osdBase=24
benchmark="LATENCY_SINGLE_MERGE_CAUCHY_4G"

echo $1 $2 $3 $4 $5
rm /tmp/update_single.log 

for ((osdNum=$minOsdNum; osdNum<=$maxOsdNum; osdNum=osdNum+1))
do
    for ((fuseNum=$minClientNum; fuseNum<=$maxClientNum; fuseNum=fuseNum+1))
    do
        ## PREPARE LOG ##
        rm ../ncvfs_log/*
        logdir="../benchmark_result/$benchmark/$branchName/$osdNum.$fuseNum"
        echo "$(date): $logdir" >> $logpath

        (
            echo "$(date): $osdNum, $fuseNum" >> $logpath

            ## START NCVFS ##
            cd ../ncvfs/script
            ./stopncvfs.sh
            ./clearmongo.sh
            ./init_disks.sh
            ./runncvfs.sh $osdNum
            sleep 3
            ./mountfuse.sh $fuseNum

            ./fill_disk.sh $osdNum 46080
            ./drop_cache.sh

            # upload
            echo "$(date): start upload $branchName $osdNum $fuseNum $options" >> $logpath
            ./upload_file_thp.sh $fuseNum 256
            echo "$(date): end upload $branchName $osdNum $fuseNum $options" >> $logpath

            ./drop_cache.sh
            ./collect_latency.sh "UPLOAD" $osdNum

            # fill reserve
            for (( i=0; i<256; i++ ))
            do
                segsize=16777216
                bs=$((segsize/4)) #### chunk size
                seek=$((i*segsize/bs))
                count=$((reservesize/bs))
                ./update_file_single.sh $seek $bs $count $fuseNum
                echo "FILL: $i"
            done

            ./drop_cache.sh
            ./collect_latency.sh "FILL" $osdNum

            # normal update
            rm /tmp/update
            for (( i=0; i<256; i++ ))
            do
                segsize=16777216
                bs=4096
                seek=$((i*segsize/bs))
                count=1
                echo $seek $bs $count >> /tmp/update
            done

            sort -R --random-source=/dev/zero /tmp/update > /tmp/update.random
            #cat /tmp/update.random | xargs -L1 -I{} sh -c "./update_file_single.sh {} $fuseNum; ./drop_latency_osd_cache.sh"
            cat /tmp/update.random | xargs -L1 -I{} sh -c "./update_file_single.sh {} $fuseNum;"

            ./collect_latency.sh "UPDATE_$j" $osdNum
       )

        ## MOVE LOGS ##
        mkdir -p $logdir
        rm $logdir/*
        mv ../ncvfs_log/* $logdir
        cp $logpath $logdir
        cp ~/shb118/ncvfs/trunk/*.xml $logdir
        cp ~/shb118/ncvfs/trunk/src/common/define.hh $logdir
        mv /tmp/update_single.log $logdir
    done
done


