#!/bin/bash

if [ $# -ne 2 ]
then
    echo '[USAGE] ./iozone_test.sh [NUM OF FUSE] [IOZONE OPTIONS]'
    exit
fi

fuseNum=$1
options=$2
addition=""

if [[ "$options" == *-K* ]]
then
    addition="_BG"
fi

alive=`cat alive_client | sort | uniq | head -n $fuseNum`
set $alive
for target
do
    targetip=192.168.0.$target
    command="iozone $options -f /home/ncsgroup/shb118/ncvfs/trunk/mountdir/iozonetest${target}"
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


