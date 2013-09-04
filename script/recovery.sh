#!/bin/bash

if [ $# -ne 2 ]
then
    echo '[USAGE] ./recovery [NUM OF FUSE] [NUM OSD KILLED]'
    exit
fi

fuseNum=$1
alive=`cat alive_monitor`
set $alive
for target
do
    targetip=192.168.0.$target
    command="pkill -USR1 MONITOR"
    screen -dm -t RECOVERY${target}_${killNum} -S RECOVERY${target}_${killNum} ssh -t -t $targetip $command
done

mds_logpath="/home/ncsgroup/shb118/ncvfs_log/ncvfs_MDS.log"

while true
do
    count=`grep "Recovery Ends" | wc -l`
    if [ $count = 1 ]; then
        echo "RECOVERY DONE"
        exit
    fi
    sleep 3
done
