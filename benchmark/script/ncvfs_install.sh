#!/bin/bash

## a script for installing the software required by NCVFS

## password for sudo
passwd="ncsncsncs"

sudo apt-get update
echo $passwd | sudo -S apt-cdrom add
echo $passwd | sudo -S apt-get -y install g++ git
echo $passwd | sudo -S apt-get -y install build-essential scons libboost-dev libboost-program-options-dev libboost-thread-dev libboost-filesystem-dev
echo $passwd | sudo -S apt-get -y install make
echo $passwd | sudo -S apt-get -y install libssl-dev pkg-config libfuse-dev
echo $passwd | sudo -S apt-get -y install iftop ifstat htop iozone3

#git clone ssh://git@projgw.cse.cuhk.edu.hk:2298/ncvfs.git

if [ ! -f /usr/local/lib/libprotobuf.so.7 ]; then
	cd ~/ncvfs/trunk/lib/protobuf-2.4.1;  ./configure; make clean; make; sudo make install
	sudo ln -s /usr/local/lib/libprotobuf.so.7 /usr/lib/libprotobuf.so.7
	sudo ln -s /usr/local/lib/libprotoc.so.7 /usr/lib/libprotoc.so.7
fi

cd ~/ncvfs/trunk/lib/mongo; echo $passwd | sudo -S scons install

