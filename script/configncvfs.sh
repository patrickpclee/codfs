alive=`cat alive_client alive_mds alive_monitor alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
	cp ./xml/* ~/shb118/ncvfs/trunk/.
	rsync -av ./xml/*.xml -e ssh $targetip:~/shb118/ncvfs/trunk &
done
