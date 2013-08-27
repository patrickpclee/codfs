if [ $# -ne 1 ]
then
  echo '[USAGE] ./mountfuse.sh [NUM OF CLIENTs]'
  exit
fi

numOfFuse=$1
if [ $numOfFuse = 0 ]; then
	exit
fi

#######################
#   START FUSE CLIENT #
#######################
fuse=`cat alive_client | sort -n | uniq | head -n $numOfFuse `
set $fuse
for target
do
    targetid=$(($target+1000))
	targetip=192.168.0.$target
	#command="cd ~/shb118/ncvfs/trunk; rm *.log; gdb -ex run --args ./mount.sh"
	command="cd ~/shb118/ncvfs/trunk; rm *.log; fusermount -u mountdir; rm -rf fusedir mountdir; mkdir fusedir mountdir; ./CLIENT_FUSE -o big_writes,large_read,noatime -d mountdir $targetid "
	screen -dm -t FUSE$target -S FUSE$target ssh -t -t $targetip $command
done
echo "Waiting 1 Seconds For FUSE Setup"
sleep 1

screen -ls
