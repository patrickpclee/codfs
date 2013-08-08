alive=`cat alive_osd | sort | uniq `
set $alive
for target
do
	targetip=192.168.0.$target
	echo "ssh -t -t $targetip 'kill -USR1 `pgrep OSD`'" >> clearosdcache.job
done

parallel --jobs 0 < clearosdcache.job
rm clearosdcache.job
