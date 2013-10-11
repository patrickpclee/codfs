#!/bin/bash



if [ $# -ne 5 ]; then
	echo "Usage: ./mountclient.sh [username] [password] [MDS host ip] [FUSE client parent path] [client id]"
	exit
fi

fusedir="$4/fusedir"
mountdir="$4/mountdir"

echo "1. UNMOUNT FUSEDIR"
sudo umount $fusedir
rm -rf $fusedir 
mkdir $fusedir

echo "2. MOUNT FUSEDIR"
./mountcifs.sh $1 $2 $3 $fusedir

status=$?
if [ $status -ne 0 ]; then
	exit
fi

echo "3. MOUNT FUSEDIR"
$(cd $4; mkdir mountdir; nohup ./CLIENT_FUSE -d mountdir $5 > $4/FUSE$1.log 2>&1 &)
disown -ah

echo "wait for 2 seconds for setup"
sleep 2

ps aux | grep FUSE | grep -v grep 
