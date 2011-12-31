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

cp ../../config/$CONF                               ./cfg/
cp ../../config/skin-aspect-ratio.png               ./cfg/
cp ../../config/skin-aspect-ratio-chroma-keys.png   ./cfg/


cp ../../build/$PRGR                      ./bin/
cp ../../build/$PRGR-debug                ./bin/
cp /usr/local/lib/libfedlibrary-3.0.0.so  ./lib/
cp /usr/local/lib/liblibavcpp-1.0.0.so    ./lib/
cp /usr/local/lib/libavcodec.so           ./lib/
cp /usr/local/lib/libavdevice.so          ./lib/
cp /usr/local/lib/libavfilter.so          ./lib/
cp /usr/local/lib/libavformat.so          ./lib/
cp /usr/local/lib/libavutil.so            ./lib/
cp /usr/local/lib/libswscale.so           ./lib/

ldconfig

