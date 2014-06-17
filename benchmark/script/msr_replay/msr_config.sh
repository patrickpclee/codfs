#!/bin/bash

scriptdir=/home/ncsgroup/shb118/benchmark/script/msr_replay
remotedir=/home/ncsgroup/shb118
remotencvfs=$remotedir/ncvfs/trunk
remotemountpt=/home/ncsgroup/shb118/ncvfs/trunk/mountdir
segSize=16777216

if [ $# -lt 1 ]; then
    echo "Usage: ./msr_config.sh [Input file]"
    exit
fi

cd $scriptdir

input=$1
alive=(`cat ../alive_client | sort | uniq | head -n 1`)

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

## Change the preallocate segment no. locally
sed -i "s/\(<PreallocateSegmentNumber>\).*\(<\/PreallocateSegmentNumber>\)/\1${ssize}\2/" ../xml/clientconfig.xml

## Sync the new config to Client 1, where new empty file will be created
targetip=192.168.0.${alive[0]}
scp ../xml/clientconfig.xml $targetip:$remotencvfs

