#!/bin/bash

if [ -e ./scripts/ComFnc ];then
    . ./scripts/ComFnc
fi

/etc/init.d/vs stop

LTFS_INSTALL_ROOT="/usr/VS"
TARGET_CONF_FOLDER="/etc/vs"
VFS_ROOT=${LTFS_INSTALL_ROOT}/vfs
#cp /etc/init.d/vs ./vs.tmp
isSim=`cat $LTFS_INSTALL_ROOT/version | grep "SIMULATOR"`
if [ $? -ne 0 ];then
    sed -i "s/vfsserver/vfsserver-simulator/g" /etc/init.d/vs  #./vs.tmp > ./vs
    newInstallSimulator="yes"
else
    newInstallSimulator="no"
fi
#chmod 777 ./vs

if [ ! -e $LTFS_INSTALL_ROOT/version ];then
    echo -e "Have you installed the VS? I can't find the version file, run me after you install the full package."
    exit 1
fi

#if [ ! -e $LTFS_INSTALL_ROOT/conf/simulator.xml ];then
if [ ! -e $TARGET_CONF_FOLDER/simulator.xml ];then
    echo -e "Copying simulator config file"
    cp ./conf/simulator.xml $TARGET_CONF_FOLDER
else
    echo -e "Simulator configure exists, no needs to replace it."
fi


echo -e "Copying simulator vfs binraries"
cp ./bin/vfsserver-simulator* $VFS_ROOT/

echo -e "Copying simulator vfsclient binraries"
cp ./bin/vfsclient-simulator* $VFS_ROOT/

mkdir /opt/VS/ltfsTapes -p
/etc/init.d/vs start

if [ $? = 0 ];then
    cp ./version $LTFS_INSTALL_ROOT/version
    echo -e "Your system has been switched to use simulator."
else
    echo -e "Failed start vfs service after switch to simulator, please check!"
fi
