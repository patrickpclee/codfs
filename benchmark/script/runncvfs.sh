if [ $# -ne 1 ]
then
  echo '[USAGE] ./runncvfs.sh [NUM OF OSDs]'
  exit
fi

numOfOsd=$1

#####################
#   START MONITOR   #
#####################
monitor=`cat alive_monitor | sort | uniq `
set $monitor
for target
do
	targetip=192.168.0.$target
	screen -dm -S MONITOR -t MONITOR ssh -t -t $targetip 'cd ~/shb118/ncvfs/trunk; ./MONITOR'
done
echo "Waiting 2 Seconds For MONITOR Setup"
sleep 2

#####################
#   START MDS       #
#####################
mds=`cat alive_mds | sort | uniq `
set $mds
for target
do
	targetip=192.168.0.$target
	screen -dm -S MDS -t MDS ssh -t -t $targetip 'cd ~/shb118/ncvfs/trunk; ./MDS'
done
echo "Waiting 2 Seconds For MDS Setup"
sleep 2

#####################
#   START OSD       #
#####################
osd=`cat alive_osd | sort -n | uniq | head -n $numOfOsd `
if [ $numOfOsd = 0 ]; then
	exit
fi
set $osd
for target
do
	targetid=$((52000+$target))
	targetip=192.168.0.$target
	#command="cd ~/shb118/ncvfs/trunk; rm *.log; gdb -ex run --args ./OSD ${targetid} eth0"
	command="cd ~/shb118/ncvfs/trunk; rm *.log; ./OSD ${targetid} eth0"
	screen -dm -t OSD$target -S OSD$target ssh -t -t $targetip $command
done
echo "Waiting 2 Seconds For OSD Setup"
sleep 2

screen -ls
