#!/bin/bash

#
# Check if running user is root
#
if [ "$(id -u)" != "0" ]
then
	echo "This script must be run as root" 1>&2
	exit -1
fi

USER=root
PRGR=recd
CONF=recd.cfg

mkdir cfg
mkdir bin
mkdir lib
mkdir pkg
mkdir doc
mkdir tools

cp -v ../../config/$CONF                          ./cfg/
cp -v ../../config/default-skin-highlights.png    ./cfg/
cp -v ../../config/default-skin-alpha.png         ./cfg/
cp -v ../../config/default-skin-chroma-keys.png   ./cfg/


cp -v ../../build/$PRGR                            ./bin/
cp -v ../../build/$PRGR-debug                      ./bin/
cp -v /usr/local/lib/libfedlibrary-3.0.1.so        ./lib/
cp -v /usr/local/lib/liblibavcpp-3.1.0.so          ./lib/
cp -v /usr/local/lib/libfedlibrary-3.0.1-debug.so  ./lib/
cp -v /usr/local/lib/liblibavcpp-3.1.0-debug.so    ./lib/
cp -v ../../build/capture                          ./tools/

tar zcvf ./lib/libav.tgz /usr/local/lib/libav* /usr/local/lib/libsw* 

cp -v ../../Dependencies/*.deb               ./pkg/

tar zcvf doc/html.tar.gz ../../docs/doxy/html
find . -type f |xargs md5sum > md5.txt

