
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
    > cd FL/lib/protobuf; ./configure; make; make install
    > sudo ln -s /usr/local/lib/libprotobuf.so.7 /usr/lib/libprotobuf.so.7
    > sudo ln -s /usr/local/lib/libprotobuf.so.7 /usr/lib/libprotobuf.so.7

    MongoDB C++ Driver
    > cd FL/lib/mongo; scons install

We provide four implementations for the update schemes: FO, FL, PL and PLR, 
located in their respective folders.

How to run 
=====

(I) COMPILE


(II) IMPORT MONGODB SETTINGS


(III) SETUP XML CONFIGS


(IV) RUN CODFS SERVERS


(V) MOUNT CODFS FUSE CLIENT


Contact
=====
Patrick P. C. Lee (http://www.cse.cuhk.edu.hk/~pclee)
