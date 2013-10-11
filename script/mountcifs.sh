#!/bin/bash

## 1. umount the target directory
## 2. mount the target directory

if [ $# -ne 4 ]; then
    echo "Usage: ./mountcifs.sh [username] [password] [host ip] [target directory]"
    echo "NOTICE: You need root permission to proceed."
    exit
fi

if [ ! -z "$(sudo -v)" ]; then
    echo "Please make sure you have the root/sudo access to this machine"
    exit
fi

userid="u$1"
passwd=$2
#read -sp "Please enter your password: " passwd
#echo ""
#echo ""
hostip=$3
targetdir=$4
curuser=`whoami`

echo "Install CIFS and Samba client..."
sudo apt-get -y install cifs-utils smbclient
echo "Umount Target Directory ..."
sudo umount $targetdir
echo "Mount Target Directory ..."
sudo mount -t cifs //$hostip/$userid $targetdir -o "user=$userid,password=$passwd,uid=$curuser,gid=$curuser"

