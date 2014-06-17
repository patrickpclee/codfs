#!/bin/bash

logpath=/home/ncsgroup/shb118/benchmark/benchmark.log

if [ $# -ne 6 ]
then
    echo '[USAGE] ./benchmark_branch.sh [branchName] [minOsdNum] [maxOsdNum] [minClientNum] [maxClientNum]'
    exit
fi

branchName=$1
minOsdNum=$2
maxOsdNum=$3
minClientNum=$4
maxClientNum=$5
osdBase=24
benchmark="IOZONE_LATENCY_32K_10_10_FSYNC_CAUCHY_ODIRECT"

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

            ./drop_cache.sh

            # upload
            echo "$(date): start upload $branchName $osdNum $fuseNum $options" >> $logpath
            ./upload_file_thp.sh $fuseNum 128 ################ upload 2GB
            echo "$(date): end upload $branchName $osdNum $fuseNum $options" >> $logpath

            ./drop_cache.sh
            ./collect_latency.sh "UPLOAD" $osdNum

            ### IOZONE WITHOUT BG JOB##
            options="-Ra -i 2 -s 2g -r 32k -w -O -e -c"
            echo "$(date): start iozone $branchName $osdNum $fuseNum $options" >> $logpath
            ./iozone_test_thp.sh $fuseNum 10 "$options"
            echo "$(date): end iozone $branchName $osdNum $fuseNum $options" >> $logpath

            ./collect_latency.sh "UPDATE" $osdNum
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


