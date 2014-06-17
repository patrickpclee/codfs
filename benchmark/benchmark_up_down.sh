#!/bin/bash

logpath=/home/ncsgroup/shb118/benchmark/benchmark.log

if [ $# -ne 6 ]
then
  echo '[USAGE] ./benchmark.sh [FILESIZE] [SEGMENTSIZE] [MAX NUM OF OSDs] [NUM OF CLIENT NODE] [NUM OF CLIENT PROC] [CODING TYPE]'
  exit
fi

filesize=$1
segmentsize=$2
maxNumOfOsd=$3
numOfClient=$4
numOfProc=$5
codingType=$6

branchName="overwrite_append_no_fsync"
minOsdNum=6
maxOsdNum=10
minClientNum=10
maxClientNum=10
osdBase=23
benchmark="UP_DOWN_CODING_${segmentsize}_${codingType}"

execute_string="$filesize $segmentsize $maxNumOfOsd $numOfClient $numOfProc $codingType"

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
            ./clearosd.sh
            ./runncvfs.sh $osdNum
            sleep 3

            # upload
            ./start_ifstat.sh ; sleep 3
            echo "$(date): start upload $segmentsize $codingType $branchName $osdNum $fuseNum $options" >> $logpath

            # upload file
            rm job_queue
            client=`cat alive_client | sort -n | uniq | head -n $numOfClient `
            set $client
            for target
            do
                echo "./run_client_upload.sh $osdNum $target $numOfProc $filesize $segmentsize $codingType" >> job_queue
            done
            sleep 3
            parallel --jobs 0 < job_queue

            echo "$(date): end upload $segmentsize $codingType $branchName $osdNum $fuseNum $options" >> $logpath
            ./stop_ifstat.sh "UPLOAD"

            # Get file id into upload.info
            grep 'Upload Done' ~/shb118/ncvfs_log/ncvfs_CLIENTUP_$osdNum* | sed -n 's/.*\[\(.*\)\]/\1/p' > upload.info
            fileids=( $(cat upload.info | cut -d ' ' -f 1) )
            j=$((${#fileids[@]} - 1))
            rm job_queue

            ./drop_cache.sh

            # download
            ./start_ifstat.sh ; sleep 3
            echo "$(date): start download $segmentsize $codingType $branchName $osdNum $fuseNum" >> $logpath

            # download file
            client=`cat alive_client | sort -n | uniq | head -n $numOfClient `
            set $client
            for target
            do
                for ((k=1; k<=$numOfProc; k=k+1))
                do
                    fileid=`echo "${fileids[$j]}" | sed 's/[^0-9]*//g'`
                    echo "./run_client_download.sh $osdNum $target 1 $filesize $segmentsize $codingType $fileid" >> job_queue
                    j=$(($j-1))
                done
            done
            sleep 3
            parallel --jobs 0 < job_queue

            echo "$(date): end download $segmentsize $codingType $branchName $osdNum $fuseNum" >> $logpath
            ./stop_ifstat.sh "DOWNLOAD"

            ./drop_cache.sh
       )

        ## MOVE LOGS ##
        mkdir -p $logdir
        rm $logdir/*
        mv ../ncvfs_log/* $logdir
        cp $logpath $logdir
    done
done


