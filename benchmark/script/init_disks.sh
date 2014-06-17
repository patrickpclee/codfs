osdblock=/home/ncsgroup/shb118/ncvfs/trunk/osd_block

alive=`cat alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
    uuid=`ssh -t -q root@$targetip "cat /etc/fstab | grep /data | cut -d '=' -f2 | cut -d ' ' -f 1"`
    uuid=`echo -n "$uuid" | sed 's/\r//g'`
#	echo "ssh -t -t root@$targetip 'unlink $osdblock; rm -rf $osdblock; umount /data; mkfs.ext4 /dev/sdb1 && tune2fs -o journal_data_writeback /dev/sdb1 && e2fsck -f /dev/sdb1 && mount -o data=writeback,noatime,nodiratime /dev/sdb1 /data; mkdir -p /data/osd_block; chown ncsgroup:ncsgroup /data/osd_block; ln -s /data/osd_block $osdblock;'" >> initdisks.job
    echo "ssh -t -t root@$targetip 'unlink $osdblock; rm -rf $osdblock; umount /data; mkfs.ext4 /dev/sdb1 && tune2fs -o journal_data_writeback /dev/sdb1 && tune2fs -O ^has_journal /dev/sdb1 && e2fsck -f /dev/sdb1 && mount -o data=writeback,noatime,nodiratime /dev/sdb1 /data; mkdir -p /data/osd_block; chown ncsgroup:ncsgroup /data/osd_block; ln -s /data/osd_block $osdblock;'" >> initdisks.job
	#echo "ssh -t -t root@$targetip 'unlink $osdblock; rm -rf $osdblock; umount /data; mkfs.ext4 /dev/sdb1 && tune2fs -o journal_data_writeback /dev/sdb1 && tune2fs -O ^has_journal /dev/sdb1 && tune2fs -o nobarrier /dev/sdb1 && e2fsck -f /dev/sdb1 && mount -o data=writeback,noatime,nodiratime /dev/sdb1 /data; mkdir -p /data/osd_block; chown ncsgroup:ncsgroup /data/osd_block; ln -s /data/osd_block $osdblock;'" >> initdisks.job
done
echo "Start initializing the disks ......"
parallel --jobs 0 < initdisks.job
rm initdisks.job
