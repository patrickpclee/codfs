ncvfs_home=/srv/home/ncsgroup

alive=`cat alive_client alive_mds alive_monitor alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
	ssh -t $targetip "rm -rf $ncvfs_home/shb118/ncvfs/trunk; mkdir -p $ncvfs_home/shb118/ncvfs/trunk"
done
