#!/bin/bash

seek=$1
bs=$2
cnt=$3

set $alive
for target
do
    targetip=192.168.0.$target
    command="cd ~/shb118/ncvfs/trunk; dd if=/dev/zero of=/home/ncsgroup/shb118/ncvfs/trunk/mountdir/benchmarkfile${target} seek=$seek bs=$bs count=$cnt conv=notrunc,fdatasync"
    echo $command >> /tmp/update_single.log
    ssh -t -t $targetip $command
done
