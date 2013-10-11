#!/bin/bash

if [ "$(id -u)" -ne 0 ]; then
	echo "Please run the script as root"
	exit
fi

version=`cat /proc/version`

if [[ "$version" =~ "Red Hat" ]]; then
    echo "Please wait patiently as the setting may take several seconds ..."
    yum -y install samba
    ## allow samba server to export any file directory configured in config
    setsebool -P samba_export_all_rw=1
elif [[ "$version" =~ "Ubuntu" ]]; then
    apt-get -y install smbfs cifs-utils samba
fi
