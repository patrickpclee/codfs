#!/usr/bin/python

import os
import sys
import time

startClient = 1
infile = "~/shb118/ncvfs/trunk/mountdir/test"
segmentSize = 10485760

if __name__=="__main__":
    if len (sys.argv) != 3:
        print "Usage: ./seq_read.py [PER_CLIENT_READSIZE] [NUM_CLIENT]"
        sys.exit()

    readSize = int(sys.argv[1])
    numClient = int(sys.argv[2])
    numReadSegment = readSize / segmentSize

    for i in range(numClient):
        os.system("screen -dm -t CLIENT%d -S CLIENT%d ssh -t -t node%d 'cd ~/shb118/ncvfs/trunk; ./mount.sh'" % (i, i, (startClient+i)))

    with open ("seq_read.job", "w") as f:
        for i in range(numClient):
            command = "ssh -t -t node%d 'dd if=%s of=/dev/null skip=%d count=%d bs=%d'\n" % (startClient + i, infile, readSize/segmentSize*i, numReadSegment, segmentSize)
            print command
            f.write(command)

    print "Executing DD (seq_read) on each OSD..."
    os.system("parallel --jobs 0 < seq_read.job")
    os.remove ("seq_read.job")
