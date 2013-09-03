#!/usr/bin/python

import sys
import random 
import re

def convertHumanReadable(input):
    ret = re.search(r"(^\d+)", input)
    num = int(ret.group(1))
    ret = re.search(r"(\D+$)", input)
    base = 1
    if (ret):
        if re.match(r"[kK].*", ret.group(1)):
            base = 1 << 10
        if re.match(r"[mM].*", ret.group(1)):
            base = 1 << 20
        if re.match(r"[gG].*", ret.group(1)):
            base = 1 << 30
    return num * base

def main():
    if len(sys.argv) != 5:
        print "Usage: ./generate_even_write.py [fileSize] [updateSize] [num to generate] [full file path]"
        sys.exit()

    fileSize = convertHumanReadable(sys.argv[1]) 
    updateSize = convertHumanReadable(sys.argv[2]) 
    numOfWrite = convertHumanReadable(sys.argv[3])
    segmentSize = convertHumanReadable("10M")
    fileName = sys.argv[4]

    random.seed(fileName)
    initOffset = random.randint(0, segmentSize-updateSize) 
    for i in range(numOfWrite):
        print "%s %d %d" %(fileName, initOffset, updateSize)
        initOffset += segmentSize
        if (initOffset >= fileSize):
            initOffset = random.randint(0, segmentSize-updateSize)

if __name__ == '__main__':
    main()
