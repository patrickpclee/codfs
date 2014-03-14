#!/bin/bash

tracedir=/home/ncsgroup/shb118/benchmark/script/msr_replay
scriptdir=/home/ncsgroup/shb118/benchmark/script/msr_replay
remotedir=/home/ncsgroup/shb118
remotemountpt=/home/ncsgroup/shb118/ncvfs/trunk/mountdir

if [ $# -lt 3 ]; then
    echo "Usage: ./msr_replay.sh [Input file] [No. of clients] [reserved]"
    exit
fi

cd $scriptdir

input=$1
clientNum=$2
reserved=$3
alive=(`cat ../alive_client | sort | uniq | head -n $clientNum`)

## Divide the traces
make divide

if [ ! -e "./divide" ] || [ ! -e $input ]; then 
    echo "Cannot divide the trace!!!"
    exit
fi

## Divide the trace first ...
./divide $input "client" $clientNum
## add the fileID used by doUpdate
sed -i 's/$/,1/' client.*

## Prepare the commands to run in parallel ...
rm msr_*.job

for (( i=1; i <= $clientNum; i++ ))
do
    ## distribute command
    targetip=192.168.0.${alive[$((i-1))]}
    echo "scp doUpdate.cc replay.cpp Makefile client.$((i-1)) $targetip:$remotedir" >> msr_dis.job

    ## compile program and replay command
    #if [[ "$reserved" =~ 'reserve' ]] || [ "$reserved" == "1" ]; then
        ## for reserved space test, replay prog needs to 'dd' the disk
        #echo "ssh -t -t root@$targetip 'cd $remotedir; make replay_reserve'" >> msr_comp.job
        echo "ssh -t -t root@$targetip 'cd $remotedir; make doUpdate'" >> msr_comp.job
    #else 
    #    ## for non-reserved space test, let python prog to do it 
    #    echo "ssh -t -t root@$targetip 'cd $remotedir; make replay_default; cd ncvfs/trunk/; echo '1' > fusedir/disk'" >> msr_comp.job
    #fi

done

## Distribute source, trace and compile replay program
parallel --jobs 0 < msr_dis.job
parallel --jobs 0 < msr_comp.job
