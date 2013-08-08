alive=`cat alive_client alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
	echo "ssh -t -t $targetip 'nohup sh -c "( ( ifstat -t -i eth0 > ~/ifstat.log ) & )" > /dev/null < /dev/null 2>&1'" >> start_ifstat.job
done

parallel --jobs 0 < start_ifstat.job
rm start_ifstat.job
