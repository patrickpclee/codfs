alive=`cat alive_client | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
	echo "ssh -t -t $targetip 'pkill BENCHMARK;'" >> clearclient.job
done

parallel --jobs 0 < clearclient.job
rm clearclient.job
