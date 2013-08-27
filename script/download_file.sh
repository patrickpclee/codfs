#!/bin/bash

if [ $# -ne 1 ]
then
    echo '[USAGE] ./download_file [NUM OF FUSE]'
    exit
fi

fuseNum=$1
alive=`cat alive_client | sort | uniq | head -n $fuseNum`
set $alive
for target
do
    targetip=192.168.0.$target
    command="cd ~/shb118/ncvfs/trunk; dd if=/home/ncsgroup/shb118/ncvfs/trunk/mountdir/benchmarkfile${target} of=/dev/null bs=10M count=200"
    screen -dm -t DOWNLOAD$target -S DOWNLOAD$target ssh -t -t $targetip $command
done


while true
do
    count=`screen -ls | grep DOWNLOAD | wc -l`
    if [ $count = 0 ]; then
        echo "DOWNLOAD DONE"
        exit
    fi
    sleep 3
done
