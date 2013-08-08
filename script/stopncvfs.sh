alive=`cat alive_client alive_mds alive_monitor alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
	echo "ssh -t -t $targetip 'pkill -9 ifstat; pkill -9 MONITOR; pkill -9 CLIENT_FUSE; pkill -9 gdb; pkill -9 MDS; pkill -9 OSD;'" >> stopncvfs.job
done
parallel --jobs 0 < stopncvfs.job 
rm stopncvfs.job

screen -ls
