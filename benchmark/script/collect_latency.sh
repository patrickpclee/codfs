#!/bin/bash

logopt=$1
osdnum=$2
alive=`cat alive_osd | sort | uniq | head -n $osdnum`
set $alive
for target
do
	targetip=192.168.0.$target
	echo "ssh -t -t $targetip 'pkill -USR2 OSD'; scp $targetip:/tmp/latency.out ~/shb118/ncvfs_log/ncvfs_latency_$target.$logopt.log" >> collect_latency.out 
done

parallel --jobs 0 < collect_latency.out 
rm collect_latency.out
