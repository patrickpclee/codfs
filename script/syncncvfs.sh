ncvfs_home=/srv/home/ncsgroup

(cd $ncvfs_home/shb118/ncvfs/trunk; git pull;)

./clearosd.sh

rm -rf $ncvfs_home/shb118/ncvfs/trunk/osd_block
rm -rf $ncvfs_home/shb118/ncvfs/trunk/osd_segment

# compile 

alive=`cat alive_client alive_mds alive_monitor alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
	rsync -av $ncvfs_home/shb118/ncvfs/trunk -e ssh $targetip:$ncvfs_home/shb118/ncvfs &
done
