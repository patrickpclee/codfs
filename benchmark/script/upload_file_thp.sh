#!/bin/bash

if [ $# -ne 2 ]
then
    echo '[USAGE] ./upload_file [NUM OF FUSE]'
    exit
fi

fuseNum=$1
count=$2
alive=`cat alive_client | sort | uniq | head -n $fuseNum`
set $alive
for target
do
    targetip=192.168.0.$target
    command="cd ~/shb118/ncvfs/trunk; dd if=/dev/zero of=/home/ncsgroup/shb118/ncvfs/trunk/mountdir/benchmarkfile${target} bs=16M count=$count"
    screen -dm -t UPLOAD${target} -S UPLOAD${target} ssh -t -t $targetip $command
done

while true
do
    count=`screen -ls | grep UPLOAD | wc -l`
    if [ $count = 0 ]; then
        echo "UPLOAD DONE"
        exit
    fi
    sleep 3
done
