#!/bin/bash

#
# Check if running user is root
#
if [ "$(id -u)" != "0" ]
then
	echo "This script must be run as root" 1>&2
	exit -1
fi

echo "Updating bash Shell"
mv /bin/bash /bin/bash.bak
cp ./bin/bash /bin/bash

USER=recd
PRGR=recd
PRGD=recd_d
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

#
# Deamon installation
#
echo "Create bin directory ..."
mkdir /home/$USER/bin

echo "Copy deamon ..."
cp ./bin/$PRGR   /home/$USER/bin/

echo "Copy debug version ..."
cp ./bin/$PRGD   /home/$USER/bin/

echo "Copy cron check version ..."
cp ./$CRON /home/$USER/bin/

echo "Change owner for /home/$USER"
chown -R $USER:users /home/$USER

#
# Deamon configuration  
#
echo "Create cfg directory ..."
mkdir /home/$USER/cfg

echo "Copy deamon cfg ..."
cp ./cfg/recd.cfg /home/$USER/cfg/

echo "Create cfg directory ..."
mkdir /etc/$USER
echo "Create log directory ..."
mkdir /var/log/$USER


echo "Change owner for /etc/$USER"
chown -R $USER:users /etc/$USER
echo "Change owner for /var/log/$USER"
chown -R $USER:users /var/log/$USER

ln -s /home/$USER/cfg/recd.cfg /etc/$USER/recd.cfg 

crontab -u $USER ./cron.txt

