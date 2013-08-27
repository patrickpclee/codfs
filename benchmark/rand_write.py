#!/usr/bin/python

fileSize = 20 * 1024 * 1024 * 1024
skipSize = 2 * 1024 * 1024
bs = 4096

skip = 0
i = 0

while i < fileSize / skipSize:
    print "dd if=/dev/zero of=%s conv=notrunc seek=%d count=1 bs=%d" %("./mountdir/test", skip / bs, bs)
    skip += skipSize
    i += 1
