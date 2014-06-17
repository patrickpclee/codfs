alive=`cat alive_client alive_mds alive_monitor alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
	echo "ssh -t root@$targetip 'ntpdate ntp1.cse.cuhk.edu.hk'" >> ntpdate.job
done
parallel --jobs 0 < ntpdate.job 
rm ntpdate.job
