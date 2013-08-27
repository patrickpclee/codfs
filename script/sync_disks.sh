alive=`cat alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
    ssh -t -q root@$targetip "sync; echo 3 > /proc/sys/vm/drop_caches"
done
parallel --jobs 0 < syncdisks.job
rm syncdisks.job
