if [ $# -ne 1 ] 
then
    echo "[USAGE] ./kill_osd.sh [OSD NODE]"
    exit
fi
targetip=192.168.0.$1
command="ssh -t -t $targetip 'pkill -9 ifstat; pkill -9 MONITOR; pkill -9 CLIENT_FUSE; pkill -9 gdb; pkill -9 MDS; pkill -9 OSD; "
echo $command

echo "KILLED OSD # $1"
screen -ls
