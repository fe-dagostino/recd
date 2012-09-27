#!/bin/bash

#
# Check if running user is root
#
if [ "$(id -u)" != "0" ]
then
	echo "This script must be run as root" 1>&2
	exit -1
fi

USER=recd
PRGR=recd
PRGD=recd-debug
CONF=recd.cfg
CRON=check.cron

echo "Check if user $USER exists"
/bin/id $USER 2>/dev/null
if [ $? -eq 0 ]
then
	echo "User $USER found"
else
	echo "User $USER not found"
	echo "Creating user $USER ... "
	useradd -N -m -Gcrontab -s/bin/false $USER
fi

# Pakages installation
dpkg -i ./pkg/libmp3lame0_3.98.4-0.0_amd64.deb
dpkg -i ./pkg/multiarch-support_2.13-33_amd64.deb
dpkg -i ./pkg/libvo-aacenc0_0.1.2-1_amd64.deb

# New dependencies coming with libavcpp-1.1.0 and recd version 1.1.0.0
apt-get install libvpx0
apt-get install libdirac-decoder0
apt-get install libdirac-encoder0
apt-get install librtmp0
apt-get install frei0r-plugins

#
# Deamon installation
#
if [ ! -d /home/$USER/bin ]
then
	echo "Create bin directory ..."
	mkdir /home/$USER/bin
fi

echo "Copy deamon ..."
pkill $PRGR
sleep 1
cp ./bin/$PRGR         /home/$USER/bin/
cp ./bin/$PRGD         /home/$USER/bin/

echo "Copy debug version ..."
cp ./bin/$PRGD   /home/$USER/bin/

echo "Copy cron check version ..."
cp ./$CRON /home/$USER/bin/

#
# Tools installation
#
if [ ! -d /home/$USER/tools ]
then
	echo "Create tools directory ..."
	mkdir /home/$USER/tools
fi

echo "Copy capture tool ..."
cp ./tools/capture /home/$USER/tools/

echo "Change owner for /home/$USER"
chown -R $USER:users /home/$USER

cp ./lib/*.so /usr/local/lib/
tar zxvf ./lib/libav.tgz -C /

chown -Rhf root:staff /usr/local/lib/libav*

ldconfig

#
# Deamon configuration  
#
if [ ! -d /home/$USER/cfg ]
then
	echo "Create cfg directory ..."
	mkdir /home/$USER/cfg
fi

echo "Save previous deamon cfg ..."
if [ -f /home/$USER/cfg/$CONF ]
then
	echo "backup current configuration as $CONF.bak ..."
	cp /home/$USER/cfg/$CONF /home/$USER/cfg/$CONF.bak
fi
echo "Copy deamon cfg ..."
cp ./cfg/$CONF                          /home/$USER/cfg/
echo "Copy default skin cfg ..."
cp ./cfg/*.png                          /home/$USER/cfg/

if [ ! -d /etc/$USER ]
then
	echo "Create cfg directory ..."
	mkdir /etc/$USER
fi
if [ ! -d /var/log/$USER ]
then
	echo "Create log directory ..."
	mkdir /var/log/$USER
fi

ln -s /home/$USER/cfg/recd.cfg                        /etc/$USER/recd.cfg 
ln -s /home/$USER/cfg/default-skin-highlights.png     /etc/$USER/default-skin-highlights.png 
ln -s /home/$USER/cfg/default-skin-alpha.png          /etc/$USER/default-skin-alpha.png 
ln -s /home/$USER/cfg/default-skin-chroma-keys.png    /etc/$USER/default-skin-chroma-keys.png 

echo "Change owner for /etc/$USER"
chown -Rh $USER:users /etc/$USER
echo "Change owner for /var/log/$USER"
chown -R $USER:users /var/log/$USER


crontab -u $USER ./cron.txt

