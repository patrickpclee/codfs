#!/bin/bash

tracedir=/home/ncsgroup/shb118/benchmark/script/msr_replay
scriptdir=/home/ncsgroup/shb118/benchmark/script/msr_replay
remotedir=/home/ncsgroup/shb118
remotemountpt=/home/ncsgroup/shb118/ncvfs/trunk/mountdir

if [ $# -lt 2 ]; then
    echo "Usage: ./msr_replay.sh [No. of clients] [reserved]"
    exit
fi

cd $scriptdir


clientNum=$1
reserved=$2
alive=(`cat ../alive_client | sort | uniq | head -n $clientNum`)

rm msr_*.job

for (( i=1; i <= $clientNum; i++ ))
do
    targetip=192.168.0.${alive[$((i-1))]}
    ## replay command
    #if [[ "$reserved" =~ 'reserve' ]] || [ "$reserved" == "1" ]; then
        ## TODO: whether write to the same file name, or diff ones?
        #echo "screen -dm -S MSR_REPLAY$i -t MSR_REPLAY$i ssh -t -t $targetip 'cd $remotedir; ./replay client.$((i-1)) $remotemountpt/disk.$i 1 1'" >> msr_replay.job
    #else 
    #    echo "screen -dm -S MSR_REPLAY$i -t MSR_REPLAY$i ssh -t -t $targetip 'cd $remotedir; ./replay client.$((i-1)) $remotemountpt/disk'" >> msr_replay.job
    #fi
    echo "screen -dm -S MSR_REPLAY$i -t MSR_REPLAY$i ssh -t -t $targetip 'cd $remotedir; ./doUpdate < client.$((i-1))'" >> msr_replay.job
    
done

## Replay the trace
parallel --jobs 1 < msr_replay.job

## Loop until replay finishes
while true
do
    count=`screen -ls | grep MSR_REPLAY | wc -l`
    if [ $count = 0 ]; then
        echo "MSR_REPLAY DONE"
        break
    fi
    sleep 3
done

