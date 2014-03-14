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
echo "Objects retrieved from compile nodes"

echo "Synchronizing all nodes"
parallel --jobs 0 < sync.job

echo "All nodes synchronized"

./ntpdate.sh
