targetip=192.168.0.21
ssh -t $targetip "/home/ncsgroup/mongo/bin/mongo ncvfs --eval 'db.FileMetaData.remove(); db.SegmentMetaData.remove()'"
#(cd /home/ncsgroup/shb118/nfs; find . -type f -delete)
