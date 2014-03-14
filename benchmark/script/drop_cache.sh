#!/bin/bash

clients=`cat alive_client alive_osd | sort | uniq`

set $clients
for client
do
    targetip="192.168.0.$client"
    echo "ssh -t -t root@$targetip 'sync && echo 3 > /proc/sys/vm/drop_caches'" >> drop_cache.job
done

parallel --jobs 0 < drop_cache.job
rm drop_cache.job
