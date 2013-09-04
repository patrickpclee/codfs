#!/bin/bash

if [ $# -ne 4 ]
then
    echo '[USAGE] ./update_file [NUM OF FUSE] [FILESIZE] [UPDATESIZE] [UPDATECOUNT]'
    exit
fi

fuseNum=$1
filesize=$2
updatesize=$3
updatecount=$4
alive=`cat alive_client | sort | uniq | head -n $fuseNum`
set $alive
for target
do
    targetip=192.168.0.$target
    command="cd ~/shb118/ncvfs/benchmark; make; ./generate_even_write.py $filesize $updatesize $updatecount /home/ncsgroup/shb118/ncvfs/trunk/mountdir/benchmarkfile${target} > even_write; ./doUpdate < even_write"
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
