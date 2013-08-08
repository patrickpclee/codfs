ncvfs_home=`cat ncvfs_home`

alive=`cat alive_client alive_mds alive_monitor alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
	echo "ssh -t -t $targetip 'pkill OSD; rm -rf $ncvfs_home/shb118/ncvfs/trunk/osd_segment/*; rm -rf $ncvfs_home/shb118/ncvfs/trunk/osd_block/*'" >> clearosd.job
done

parallel --jobs 0 < clearosd.job
rm clearosd.job
