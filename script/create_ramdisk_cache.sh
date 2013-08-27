ncvfshome=`cat ncvfs_home`/shb118/ncvfs/trunk

alive=`cat alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target

    # put osd_segment in ramdisk
	ssh -t $targetip "unlink $ncvfshome/osd_segment"
	ssh -t $targetip "rm -rf $ncvfshome/osd_segment"
	ssh -t $targetip "rm -rf /dev/shm/osd_segment"
	ssh -t $targetip "mkdir /dev/shm/osd_segment"
	ssh -t $targetip "ln -s /dev/shm/osd_segment $ncvfshome/osd_segment"

    # put osd_block in ramdisk
    #ssh -t $targetip "echo 'ncsncsncs' | sudo -S umount ~/shb118/ramdisk"
    #ssh -t $targetip "rm -rf ~/shb118/ramdisk"
    #ssh -t $targetip "mkdir ~/shb118/ramdisk/"
    #ssh -t $targetip "echo 'ncsncsncs' | sudo -S sudo mount -t tmpfs -o size=3G,mode=777 tmpfs ~/shb118/ramdisk/"
    #ssh -t $targetip "mkdir ~/shb118/ramdisk/osd_block"

	#ssh -t $targetip "unlink $ncvfshome/osd_block"
	#ssh -t $targetip "rm -rf $ncvfshome/osd_block"
	#ssh -t $targetip "ln -s /home/ncsgroup/shb118/ramdisk/osd_block $ncvfshome/osd_block"

    # put osd_block in harddisk

#    ssh -t $targetip "echo 'ncsncsncs' | sudo -S sudo umount tmpfs"
	ssh -t $targetip "unlink $ncvfshome/osd_block"
	ssh -t $targetip "rm -rf $ncvfshome/osd_block"
	ssh -t $targetip "mkdir $ncvfshome/osd_block"
done
