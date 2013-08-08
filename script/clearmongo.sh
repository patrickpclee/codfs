targetip=192.168.0.21
ssh -t $targetip "/home/ncsgroup/mongo/bin/mongo ncvfs --eval "db.dropDatabase();"
