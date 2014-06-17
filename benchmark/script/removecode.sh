ncvfs_home=/home/ncsgroup

alive=`cat alive_client alive_mds alive_monitor alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
	echo "ssh -t -t root@$targetip 'rm -rf $ncvfs_home/shb118/ncvfs/trunk'" >> removecode.job
    echo "ssh -t -t $targetip 'mkdir -p $ncvfs_home/shb118/ncvfs/trunk'" >> removecode.job
done
parallel --jobs 0 < removecode.job
rm removecode.job
