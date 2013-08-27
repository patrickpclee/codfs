#!/bin/bash

if [ $# -ne 1 ]
then
    echo '[USAGE] ./update_file [NUM OF FUSE]'
    exit
fi

fuseNum=$1
alive=`cat alive_client | sort | uniq | head -n $fuseNum`
set $alive
for target
do
    targetip=192.168.0.$target
    command="cd ~/shb118/ncvfs/benchmark; make; ./generate_even_write.py 2000M 512K 4K /home/ncsgroup/shb118/ncvfs/trunk/mountdir/benchmarkfile${target} > even_write; ./evenUpdate < even_write"
    screen -dm -t UPDATE$target -S UPDATE$target ssh -t -t $targetip $command
done
while true
do
    count=`screen -ls | grep UPDATE | wc -l`
    if [ $count = 0 ]; then
        echo "UPDATE DONE"
        exit
    fi
    sleep 3
done
