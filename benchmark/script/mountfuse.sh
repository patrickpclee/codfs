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
	#command="cd /home/ncsgroup/shb118/ncvfs/trunk; rm *.log; umount fusedir; rm -rf fusedir ; mkdir fusedir; mount 192.168.0.1:/home/ncsgroup/shb118/nfs/$targetid ./fusedir/; "
	command="cd /home/ncsgroup/shb118/ncvfs/trunk; rm *.log; rm -rf fusedir ; mkdir fusedir;"
	screen -dm -t MOUNT$target -S MOUNT$target ssh -t -t $targetip $command
	command="cd /home/ncsgroup/shb118/ncvfs/trunk; fusermount -u mountdir; rm -rf mountdir; mkdir mountdir; ./CLIENT_FUSE -o big_writes,large_read,noatime -f mountdir $targetid "
    echo $command
	screen -dm -t FUSE$target -S FUSE$target ssh -t -t $targetip $command
done
echo "Waiting 1 Seconds For FUSE Setup"
sleep 1

screen -ls
