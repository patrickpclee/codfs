logopt=$1
alive=`cat alive_client alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
	echo "ssh -t -t $targetip 'pkill -9 ifstat'; scp $targetip:ifstat.log ~/shb118/ncvfs_log/ncvfs_ifstat_$target.$logopt.log" >> stop_ifstat.job
done

parallel --jobs 0 < stop_ifstat.job
rm stop_ifstat.job 
