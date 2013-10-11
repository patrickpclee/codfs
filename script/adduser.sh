#!/bin/bash

homedirp="/data/cifs"
## DEFAULT username: u[uid]
## DEFAULT homedir: $homedirp/[username]
user="u$1"
homedir="$homedirp/$user"

## 1. Add a user to the server OS
## 2. Add the entry to the samba file server config
## 3. Restart samba file server

## need root access to proceed
if [ "$(id -u)" != "0" ]; then
    echo "Please run this script as root"
    exit
fi

if [ $# -ne 1 ]; then
    echo "Usage: ./adduser.sh [username]"
    exit
fi

## check the OS type, as they have different environment setting
redhat=0
ubuntu=0
osversion=`cat /proc/version`
if [[ "$osversion" =~ "Red Hat" ]]; then
	echo "Red Hat Machine"
	redhat=1
elif [[ "$osversion" =~ Ubuntu ]]; then
	echo "Ubuntu"
	ubuntu=1
fi

## "hardcoded" samba config entry
entry="[$user]\npath=$homedir\nbrowseable=yes\ncreate mask=0755\nwritable=yes\nread only=no\n"

## add user, add password to samba server, create home directory for samba server, 
## change permission of home directory, add record to samab server, restart samba server
adduser $user
passwd -l $user
smbpasswd -a $user || smbpaswd -U $user
mkdir -p $homedir && chown $user:$user $homedir

if [ -z "$(grep '\['$user'\]' /etc/samba/smb.conf)" ]; then
	if [ -w /etc/samba/smb.conf ]; then
		echo -e $entry >> /etc/samba/smb.conf
	else
		echo "----- Please MANUALLY add the following to /etc/samba/smb.conf -----"
		echo -e $entry
		echo "----- end of entry -----"
	fi
else
	echo "User entry exists"
fi

if [ $ubuntu -eq 1 ]; then
	/etc/init.d/smbd restart && echo "User is added successfully." || echo "Error occured."
elif [ $redhat -eq 1 ]; then
	/sbin/service smb restart && echo "User is added successfully." || echo "Error occured."
fi

