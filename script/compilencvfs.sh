./stopncvfs.sh
./clearclient.sh

ncvfs_home=`cat ncvfs_home`

rm -rf $ncvfs_home/shb118/ncvfs/trunk/osd_block
rm -rf $ncvfs_home/shb118/ncvfs/trunk/osd_segment

echo "Start Compiling"

# GW: MDS
# cloud-node1: MONITOR
# cloud-node2: OSD
# cloud-node3: BENCHMARK
# cloud-node4: CLIENT
# cloud-node5: FUSE
# cloud-node6: CODING_TESTER

# push codes to compile server
parallel --jobs 0 < compile_put.job
echo "Code pushed to compile nodes"
# run compile
parallel --jobs 0 < compile_make.job
echo "Compile completes"
# get results
parallel --jobs 0 < compile_get.job
echo "Compile code synchronized"

alive=`cat alive_client alive_mds alive_monitor alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
	echo "rsync -av $ncvfs_home/shb118/ncvfs/trunk -e ssh $targetip:$ncvfs_home/shb118/ncvfs > /dev/null" >> sync.job
done

parallel --jobs 0 < sync.job
rm sync.job
echo "All nodes code synchronized"

