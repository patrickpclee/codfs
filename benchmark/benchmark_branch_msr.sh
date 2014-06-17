#!/bin/bash

homedir=/home/ncsgroup/shb118
screenlogpath=$homedir/ncvfs_log
logpath=$homedir/benchmark/log/benchmark.log
scriptdir=$homedir/benchmark/script
msrscriptdir=$homedir/benchmark/script/msr_replay
## !! Please change the following to the MSR trace path
tracedir=/home/ncsgroup/shb118/benchmark
tracesrc=/data/trace
tracepath=$tracedir/src2_2.csv.remap.4m

if [ $# -ne 5 ] 
then
    echo '[USAGE] ./benchmark_branch.sh [branchName] [minOsdNum] [maxOsdNum] [minClientNum] [maxClientNum]'
    exit
fi

branchName=$1
minOsdNum=$2
maxOsdNum=$3
minClientNum=$4
maxClientNum=$5
osdBase=23
segSize=16777216 # 16M
benchmarkName="MSR_REPLAY_RESERVE_OVERHEAD_SRC22"
   
for ((osdNum=$minOsdNum; osdNum<=$maxOsdNum; osdNum=osdNum+1))
do
    for ((fuseNum=$minClientNum; fuseNum<=$maxClientNum; fuseNum=fuseNum+1))
    do  
        ## PREPARE LOG ##
        rm $screenlogpath/*
        logdir="/home/ncsgroup/shb118/benchmark_result/$benchmarkName/$branchName/$osdNum.$fuseNum"
        echo "$(date): $logdir" >> $logpath

        (
            echo "$(date): $osdNum, $fuseNum" >> $logpath
            ## START NCVFS ##
            cd $scriptdir 
            ./stopncvfs.sh
            ./clearmongo.sh

            ./init_disks.sh $osdNum
            #if [[ ! $branchName =~ 'reserve' ]]; then
            #    cd $msrscriptdir
            #    tSize=$(./msr_check_size $tracepath /dev/null)
            #    ./filegen.py $tSize $segSize $osdNum
            #    cd $scriptdir 
            #fi
            ## modify the client configuration 
            (cd $msrscriptdir; ./msr_config.sh $tracepath)
            ./runncvfs.sh $osdNum
            sleep 3
            ./mountfuse.sh $fuseNum
            echo "$(date): start upload $branchName $osdNum $fuseNum" >> $logpath
            ## upload the file by 1. prealloc segment, 2. dd on each client
            (cd $msrscriptdir; ./msr_upload.sh $tracepath $fuseNum)
            echo "$(date): end upload $branchName $osdNum $fuseNum" >> $logpath
            sleep 3
           
            ## CREATE FILE, REPLAY MSR TRACE, RECOVERY
            ## CREATE FILE
            #./start_ifstat.sh; sleep 3;
            (cd $msrscriptdir; ./msr_prepare.sh $tracepath $fuseNum $branchName)
            #./stop_ifstat.sh "UPLOAD"
            ./drop_cache.sh
            
            ## REPLAY_MSR_TRACE
            ./start_ifstat.sh; sleep 3;
            echo "$(date): start replay msr writes $branchName $osdNum $fuseNum" >> $logpath
            (cd $msrscriptdir; ./msr_replay.sh $fuseNum $branchName $branchName)
            echo "$(date): end replay msr writes $branchName $osdNum $fuseNum" >> $logpath
            ./stop_ifstat.sh "MSR"
            ./drop_cache.sh

            ## RECOVERY
            killNum=2
            #### KILL FIRST OSD ###
            ./kill_osd.sh $((osdNum+osdBase))
            #### KILL SECOND OSD ###
            ./kill_osd.sh $((osdNum+osdBase-1))

            
            ./start_ifstat.sh; sleep 3;
            echo "$(date): start recovery $branchName $osdNum $fuseNum" >> $logpath
            ./recovery.sh $fuseNum $killNum
            echo "$(date): end recovery $branchName $osdNum $fuseNum" >> $logpath
            ./stop_ifstat.sh "RECOVERY"

            mkdir -p $logdir
            rm $logdir/*
            mv $screenlogpath/ncvfs_*.log $logpath $logdir
            cp $homedir/ncvfs/trunk/*.xml $homedir/ncvfs/trunk/src/common/define.hh $logdir
        )
    done
done

