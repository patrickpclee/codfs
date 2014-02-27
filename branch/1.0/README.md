
Welcome
=====

This is the source code for CodFS described in our paper presented in USENIX FAST
2014. The system is tested on Ubuntu 12.04 64-bit.  - January 2014

Setup
=====

The program can be compiled using Linux make. These are the required
libraries that users need to download separately. Brackets denote the package
names in Debian and Ubuntu platforms. Users can use apt-get to install the
required libraries. 

    build-essential (build-essential)
    scons (scons)
    boost libraries (libboost-dev, libboost-program-options-dev, libboost-thread-dev, libboost-filesystem-dev)
    FUSE (libfuse-dev)
    openssl (libssl-dev)
    pkg-config (pkg-config)

The following libraries have to be compiled and installed manually.

    Google Protocol Buffers
    > cd FL/lib/protobuf-2.4.1; ./configure; make; sudo make install
    > sudo ln -s /usr/local/lib/libprotobuf.so.7 /usr/lib/libprotobuf.so.7
    > sudo ln -s /usr/local/lib/libprotoc.so.7 /usr/lib/libprotoc.so.7

    MongoDB C++ Driver
    > cd FL/lib/mongo; sudo scons install

We provide four implementations for the update schemes: FO, FL, PL and PLR, 
located in their respective folders. The above setup procedures only need 
to be executed once.

How to run 
=====

We describe how to run CodFS in FL mode. The procedures for running 
other update schemes are the same.

(I) COMPILE

    > cd FL/; make

(II) IMPORT MONGODB SETTINGS

    1. Set up MongoDB environment
        a. Create the database parent directory
            > mkdir /data/db
        b. Change the MongoDB address "[mongodb_ip]:[mongodb_port]" in "mongo-ncvfs.js" if necessary
            > db = connect("[mongodb_ip]:[mongodb_port]/ncvfs")
    2. Change to directory containing MongoDB binary
        > cd mongo/bin;
    3. Start MongoDB daemon
        > sudo ./mongod --fork --dbpath /data/db --logpath /var/log/mongodb.log --logappend
    4. Import MongoDB Setting
        > ./mongo ../../mongo-ncvfs.js

(III) SETUP XML CONFIGS

    * MONITOR (monitorconfig.xml)
        - Time interval for failure and recovery detection
    * MDS (mdsconfig.xml)
        - MongoDB configuration
    * OSD (osdconfig.xml)
        - Capacity and cache size of Object Storage Servers
    * CLIENT (clientconfig.xml)
        - Client-side cache size
        - Coding scheme settings
    * Common configuration (common.xml)
        - Shared MDS and MONITOR configurations

(IV) RUN CODFS SERVERS

    * Separate components
        1. MONITOR
            > ./MONITOR
        2. MDS
            > ./MDS
        3. OSD
            > ./OSD [component_id] [network_interface] 
            
(V) MOUNT CODFS FUSE CLIENT
    > mkdir fusedir mountdir
    > ./CLIENT_FUSE -o big_writes,large_read,noatime -f mountdir [client_id]

Contact
=====
Patrick P. C. Lee (http://www.cse.cuhk.edu.hk/~pclee)
