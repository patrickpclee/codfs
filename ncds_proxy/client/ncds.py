#!/usr/bin/env python

import sys
import urllib, urllib2
from poster.encode import multipart_encode
from poster.streaminghttp import register_openers

SERVER_ADDRESS="http://localhost/ncds/server/"

def printUsage():
    print "Usage:"
    print "UPLOAD:\t\t./ncds.py put [SRC FILE] [DST PATH]"
    print "DOWNLOAD:\t./ncds.py get [FILE ID] [DST PATH]"
    print "LIST:\t\t./ncds.py list"
    print "DELETE:\t\t./ncds.py delete [SRC FILE]"
    sys.exit()

def ncdsPut(src, dst):
    print "Source Path:", src
    print "Dst Path:", dst
    try:
        f = open(src, 'rb')
    except IOError:
        print "Cannot read file"
        sys.exit()

    register_openers()
    url = SERVER_ADDRESS + 'do_upload.php'
    values = {'path': dst,
          'upload_file': f,
          'submit':''}
    data, headers = multipart_encode(values)
    request = urllib2.Request(url, data, headers)
    request.unverifiable = True
    response = urllib2.urlopen(request)
    the_page = response.read()

    print "=========="
    the_page = "\n".join(the_page.split("<br />"))
    the_page = the_page.replace('@', '/')
    print the_page
    print "=========="

def ncdsGetById (fileid, dst):
    print "File ID:", fileid
    print "Dst Path:", dst

    url = SERVER_ADDRESS + 'do_download.php?fileid=' + str(fileid)
    u = urllib2.urlopen(url)

    try:
        localFile = open(dst, 'w')
        localFile.write(u.read())
        localFile.close()
    except IOError:
        print "Cannot write file"
        sys.exit()

def ncdsGet (src, dst):
    print "Src Path:", src
    print "Dst Path:", dst

    fileid = getFileIdFromPath (src)
    ncdsGetById (fileid, dst)

def getFileIdFromPath(src):
    filelist = readFileList()

    isFound = False
    for line in filelist.split('\n')[1::]:
        if len(line.split()) < 2:
            continue
        path = line.split()[0]
        if src == path:
            isFound = True
            fileid = int(line.split()[1])
            break

    if not isFound:
        print "No such file"
        sys.exit()

    return fileid

def readFileList():
    url = SERVER_ADDRESS + 'list.php?python'
    u = urllib2.urlopen(url)

    return u.read()

def ncdsList():
    print "Listing Files"
    print readFileList()

def ncdsDelete(src):
    fileid = getFileIdFromPath (src)
    url = SERVER_ADDRESS + 'do_delete.php?fileid=' + str(fileid)
    u = urllib2.urlopen(url)

    print "\n".join(u.read().split("<br />"))

if __name__ == "__main__":

    if (len(sys.argv) > 4 or len(sys.argv) < 2):
        printUsage()

    action = sys.argv[1]

    if (action == "put"):

        if (len(sys.argv) != 4):
            printUsage()
        src = sys.argv[2]
        dst = sys.argv[3]
        ncdsPut (src, dst)

    elif (action == "get"):

        if (len(sys.argv) != 4):
            printUsage()
        src = sys.argv[2]
        dst = sys.argv[3]
        ncdsGet (src, dst)

    elif (action == "list"):

        if (len(sys.argv) != 2):
            printUsage()
        ncdsList ()

    elif (action == "delete"):

        if (len(sys.argv) != 3):
            printUsage()
        src = sys.argv[2]
        ncdsDelete (src)

    else:
        printUsage()
