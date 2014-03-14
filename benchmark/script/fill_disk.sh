#!/bin/bash

if [ $# -ne 2 ]
then
    echo '[USAGE] ./fill_disk [OSD NUM] [NUM OF CHUNK]'
    exit
fi

osdNum=$1
chunks=$2
alive=`cat alive_osd | sort | uniq | head -n $osdNum`
set $alive
for target
do
    targetip=192.168.0.$target
    echo "ssh -t -t root@$targetip '/home/ncsgroup/shb118/ncvfs/trunk/fill_disk_single.sh $chunks'" >> filldisk.job
done
echo "Start filling the disks ......"
parallel --jobs 0 < filldisk.job 
rm filldisk.job
