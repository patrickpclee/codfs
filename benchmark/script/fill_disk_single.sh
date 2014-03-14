#!/bin/bash

cd /home/ncsgroup/shb118/ncvfs/trunk/osd_block
for (( i=0; i<$1; i++ ))
do
    dd if=/dev/zero of=fill_$i count=1 bs=4M
done

