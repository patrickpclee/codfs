#!/bin/bash

tracedir=/home/ncsgroup/shb118/benchmark/script/msr_replay
scriptdir=/home/ncsgroup/shb118/benchmark/script/msr_replay
remotedir=/home/ncsgroup/shb118
remotemountpt=/home/ncsgroup/shb118/ncvfs/trunk/mountdir
segSize=16777216

if [ $# -lt 2 ]; then
    echo "Usage: ./msr_replay.sh [Input file] [No. of clients]"
    exit
fi

cd $scriptdir

input=$1
clientNum=$2
alive=(`cat ../alive_client | sort | uniq | head -n $clientNum`)

## Measure the size of the trace
make msr_check_size

if [ ! -e "./msr_check_size" ] || [ ! -e $input ]; then 
    echo "Cannot measure the trace size!!!"
    exit
fi

## Calculate the segments to be created by each client first
## total size
tsize=`./msr_check_size $input "client" $clientNum`
ssize=$((tsize/segSize))
if [ $((tsize - segSize * ssize)) -gt 0 ]; then
    ssize=$((ssize+1))
fi
######################### TEST
#ssize=18
#########################

## size per client
csize=$((ssize / clientNum))
## size for last client (some more segment if necessary)
lsize=$((csize+ssize%clientNum))


## Prepare the commands for Extending file to run in parallel ...
rm msr_*.job


## Preallocate segment, and then distribute fid to other clients
fid=`ssh -t -t 192.168.0.${alive[0]} "touch $remotemountpt/benchmarkfile; cat $remotemountpt/../fusedir/benchmarkfile"`


## Report the parm 
echo "Total: $tsize  Per Client: $csize  Last Client: $lsize FID: $fid"


for (( i=1; i <= $clientNum; i++ ))
do
    targetip=192.168.0.${alive[$((i-1))]}
    ## distribute fid command
    echo "ssh -t -t $targetip 'echo -n $fid > $remotemountpt/../fusedir/benchmarkfile'" >> msr_fid.job
    ## extend file command
    if [ $i -lt $clientNum ]; then
        echo "screen -dm -t UPLOAD$i -S UPLOAD$i ssh -t -t $targetip 'cd $remotedir; dd if=/dev/zero of=$remotemountpt/benchmarkfile bs=16M count=$csize seek=$(((i-1)*csize)) conv=notrunc,nocreat'" >> msr_upload.job
    else ## last client
        echo "screen -dm -t UPLOAD$i -S UPLOAD$i ssh -t -t $targetip 'cd $remotedir; dd if=/dev/zero of=$remotemountpt/benchmarkfile bs=16M count=$lsize seek=$(((i-1)*csize)) conv=notrunc,nocreat'" >> msr_upload.job
    fi
    done

## Distibute the fid
parallel --jobs 0 < msr_fid.job
## Extend the file per client
parallel --jobs 0 < msr_upload.job
## Loop until replay finishes
while true
do
    count=`screen -ls | grep UPLOAD | wc -l`
    if [ $count = 0 ]; then
        echo "MSR_UPLOAD DONE"
        break
    fi
    sleep 3
done

