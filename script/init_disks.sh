osdblock=/home/ncsgroup/shb118/ncvfs/trunk/osd_block

alive=`cat alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
    uuid=`ssh -t -q root@$targetip "cat /etc/fstab | grep /data | cut -d '=' -f2 | cut -d ' ' -f 1"`
    uuid=`echo -n "$uuid" | sed 's/\r//g'`
	echo "ssh -t -t root@$targetip 'unlink $osdblock; rm -rf $osdblock; umount /data; mke2fs -T ext4 -U $uuid /dev/sdb1 && mount -a; mkdir -p /data/osd_block; chown ncsgroup:ncsgroup /data/osd_block; ln -s /data/osd_block $osdblock;'" >> initdisks.job
done
parallel --jobs 0 < initdisks.job
rm initdisks.job
