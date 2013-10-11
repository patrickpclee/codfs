#!/bin/bash

if [ $# -ne 1 ]; then
	echo "Usage: ./unmountclient.sh [FUSE client directory path]"
	exit
fi

sudo umount $1/fusedir
sudo umount $1/mountdir
