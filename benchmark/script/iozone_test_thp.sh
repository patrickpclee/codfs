#!/bin/bash

if [ $# -ne 3 ]
then
    echo '[USAGE] ./iozone_test.sh [NUM OF FUSE] [NUM OF THREADS] [IOZONE OPTIONS]' 
    exit
fi

fuseNum=$1
threads=$2
options=$3
addition=""

if [[ "$options" == *-K* ]]
then
    addition="_BG"
fi


ncvfshome="/home/ncsgroup/shb118/ncvfs/trunk/mountdir/"


alive=`cat alive_client | sort | uniq | head -n $fuseNum`
set $alive
for target
do
    targetip=192.168.0.$target
    files="${ncvfshome}benchmarkfile${target}"
    command="/home/ncsgroup/shb118/ncvfs/trunk/iozone $options -f $files"
    screen -dm -t IOZONE${target}${addition} -S IOZONE${target}${addition} ssh -t -t $targetip $command
done

while true
do
    count=`screen -ls | grep IOZONE | wc -l`
    if [ $count = 0 ]; then
        echo "IOZONE DONE"
        break
    fi
    sleep 3
done


