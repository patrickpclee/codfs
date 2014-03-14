#!/bin/bash

seek=$1
bs=$2
cnt=$3
fuseNum=$4

alive=`cat alive_client | sort | uniq | head -n $fuseNum`
set $alive
for target
do
    targetip=192.168.0.$target
    command="cd ~/shb118/ncvfs/trunk; dd if=/dev/zero of=/home/ncsgroup/shb118/ncvfs/trunk/mountdir/benchmarkfile${target} seek=$seek bs=$bs count=$cnt conv=notrunc,fdatasync"
    echo $command >> /tmp/update_single.log
    screen -dm -t UPDATE$target -S UPDATE$target ssh -t -t $targetip $command
done

while true
do
    count=`screen -ls | grep UPDATE | wc -l`
    if [ $count = 0 ]; then
        echo "UPDATE DONE"
        exit
    fi
    sleep 1
done
