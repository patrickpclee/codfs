#!/usr/bin/python

import os
import sys
import math
import collections
import itertools
from pymongo import MongoClient

# config
ipPrefix = "192.168.0."
clientNode = 23
clientId = 1
fileId = 1
path = "/test"
startOsd = 24
numDataBlock = 2
numParityBlock = 1
codingScheme = 3
codingSetting = "3"

ncvfspath = "/home/ncsgroup/shb118/ncvfs/trunk/"
blockpath = ncvfspath + "osd_block/"
mongoNode = 21
mongoport = 27017

def roundTo (x, base):
    return int(base * round(float(x)/base))

if __name__=="__main__":

    if len (sys.argv) != 4:
        print "Usage: ./filegen.py [FILESIZE] [SEGMENT_SIZE] [NUM_OSD]"
        sys.exit()

    segmentSize = int(sys.argv[2])
    fileSize = roundTo (int(sys.argv[1]), segmentSize)
    numOsd = int(sys.argv[3])
    endOsd = startOsd + numOsd - 1

    print "File Size:", fileSize
    print "Segment Size:", segmentSize
    print "Num of OSD:", numOsd

    numSegment = fileSize / segmentSize
    print "Num of Segment:", numSegment
    blockSize = segmentSize / numDataBlock

    ##### BLOCK ROUND-ROBIN DISTRIBUTION #####

    # generate permutations
    permutations = list (itertools.permutations(range(numOsd)))

    segmentList = collections.OrderedDict()
    for i in range(numSegment):
        segmentList[i] = []       # segment ID -> osdList
        curPermutation = permutations [i % len(permutations)]
        for j in range (numDataBlock + numParityBlock):
            curOsd = curPermutation[j % len(curPermutation)] + startOsd

            #print "Segment", i, "Block", j, ":", curOsd
            segmentList[i].append(curOsd)

            # OSD dd
            with open (str(curOsd) + ".sh", 'a') as f:
                command = "dd if=/dev/zero of=%s bs=%d count=1 &> /dev/null\n" % (blockpath+str(i)+'.'+str(j), blockSize)
                f.write(command)

    ##### INSERT INTO MONGODB #####

    # mongodb connection
    client = MongoClient(ipPrefix + str(mongoNode), mongoport)
    db = client['ncvfs']
    FileMetaData = db['FileMetaData']
    SegmentMetaData = db['SegmentMetaData']

    # SegmentMetaData
    fileSegmentObj = collections.OrderedDict()
    for k,v in segmentList.items():

        nodeObj = collections.OrderedDict()
        for blockId, nodeId in enumerate(v):
            nodeObj[str(blockId)] = nodeId + 52000

        segmentObj = {"id": k,
                "primary": v[0]+52000,
                "size": segmentSize,
                "codingScheme": codingScheme,
                "codingSetting": codingSetting,
                "nodeList": nodeObj
                }
        SegmentMetaData.insert(segmentObj)
        fileSegmentObj[str(k)] = k # for FileMetaData

    # FileMetaData
    fileObj = {"clientId": clientId,
            "fileSize": fileSize,
            "id": fileId,
            "path": path,
            "segmentList": fileSegmentObj
            }
    FileMetaData.insert (fileObj)

    # START DD IN OSD
    with open ("dd.job", "w") as f:
        for i in range(startOsd, endOsd +1):
            command = "ssh node%d 'bash -s' < ./%d.sh\n" % (i-10, i)
            f.write(command)

    print "Executing DD on each OSD..."
    os.system("parallel --jobs 0 < dd.job")

    # clean
    os.remove("dd.job")
    for i in range(startOsd, endOsd +1):
        os.remove("%d.sh" % i)
