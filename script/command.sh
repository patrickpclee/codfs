alive=`cat alive_client alive_mds alive_monitor alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
#	ssh -t $targetip "echo 'NCDSshb720..' | sudo -S apt-get install -y gdb"
#	ssh -t $targetip "mkdir -p ~/.ssh; echo 'ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDON6xtT7LYkUNG9M8KGSo73dGtggZsoqT5mDx1vJRoaSEXV0MQBmuTkT+cqjWVeANZDt7BixHJXDEl4tJElk9HQOvkVFjgPfBxYjxJyeC/WvRUTeFQJCWHlLOfutKxrZLfnCd7bRmCcLOQu2+l6B+5WysDR287U2zhk3bEZkcxY5ohkAx3YfItrDxz25Lgc9AuNmELNowUxv8UB2/1HmLVTyl9FkqiqA2OabfXyQdXkAJ3SEp2nYS2f5qZevfGe20FcelSIlJ7/odR5gxrJEvLqS/aw9nIrf08ELJbBrwYPjkUzFd535j49Ppi2DWsAq77+0ry0b8TWsX03tncxxHB ncsgroup@cloud-controller' > ~/.ssh/authorized_keys"
#	scp ./installncvfs.sh $targetip:./script/.
#	ssh -t $targetip './script/installncvfs.sh'
#	ssh -t $targetip 'ln -s /srv/home/ncsgroup/shb118/ncvfs/ ncvfs'
#	ssh -t $targetip "echo 'ncsncsncs' | sudo -S rm -rf /srv/home/ncsgroup/shb118/ncvfs/trunk; mkdir /srv/home/ncsgroup/shb118/ncvfs/trunk"
#	ssh -t $targetip 'ln -s /srv/home/ncsgroup/shb118 ~/shb118'
#	ssh -t $targetip "echo 'ncsncsncs' | sudo -S apt-get install -y ifstat"
#	ssh -t $targetip "echo \"export LC_ALL='C'\" > .bashrc"
#	ssh -t $targetip "rm ~/ifstat.log"
#	ssh -t $targetip "echo 'ncsncsncs' | sudo -S /etc/init.d/ntp stop;"
#	ssh -t $targetip "echo 'ncsncsncs' | sudo -S ntpdate ntp1.cse.cuhk.edu.hk"
	#ssh -t $targetip "echo 'ncsncsncs' | sudo -S apt-get install iotop"
	#ssh -t $targetip "date"
	#ssh -t $targetip "ps aux | grep swift"
	#ssh -t $targetip "ps aux | grep java"
	#ssh -t $targetip "echo 'ncsncsncs' | sudo -S killall -9 java"
	#ssh -t $targetip "ls -l ~/shb118/ncvfs/trunk"
	#ssh -t $targetip "ps -eo pcpu,pid,user,args | sort -k 1 -r | head -6"
	#ssh -t $targetip "echo 'ncsncsncs' | sudo -S rm -rf ncvfs; mkdir -p /home/ncsgroup/shb118/ncvfs"
    ssh -t root@$targetip "apt-get install dstat"
done
