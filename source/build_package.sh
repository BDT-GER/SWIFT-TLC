#!/bin/bash

VMAIN=$1
VSUB=$2
VBUILD=$3
VERSION=${VMAIN}.${VSUB}.${VBUILD}

function execCmd()
{
    CMD=$1
    pwd
    echo "CMD:" $CMD
    $CMD
    if [ $? -ne 0 ];then
        echo "Failed to execute " $CMD
        exit 1
    fi
}

PACKAGE_FOLDER_SE="./Build/install_packages/TLC/"
PACKAGE_FOLDER_SE_VS="${PACKAGE_FOLDER_SE}/VS/"
PACKAGE_FOLDER_SIM="./Build/install_packages/simulator_switcher/"
PACKAGE_FOLDER_VERIFYTOOL="./Build/install_packages/SWIFT-Tape-Auditor/TapeVerifyTool/"
PACKAGE_OPENSTACK_INS="./Build/install_packages/CentOS7-SWIFT/"

execCmd "cd ./Server/tlc-server"
execCmd "make"
execCmd "rm -rf ../../${PACKAGE_FOLDER_SE_VS}/bin/vfs*"
execCmd "mkdir -p ../../${PACKAGE_FOLDER_SE_VS}/bin/"
execCmd "cp vfsserver  ../../${PACKAGE_FOLDER_SE_VS}/bin/"
execCmd "cp vfsclient  ../../${PACKAGE_FOLDER_SE_VS}/bin/"
execCmd "cp lfs_tool  ../../${PACKAGE_FOLDER_SE_VS}/bin/"
execCmd "cp library_tool  ../../${PACKAGE_FOLDER_SE_VS}/bin/"
execCmd "rm -rf ${PACKAGE_FOLDER_SIM}/bin/vfs*sim*"
execCmd "mkdir -p ../../${PACKAGE_FOLDER_SIM}/bin/"
execCmd "cp vfsserver-simulator ../../${PACKAGE_FOLDER_SIM}/bin/"
execCmd "cp vfsclient-simulator ../../${PACKAGE_FOLDER_SIM}/bin/"
execCmd "cd ../../" 
execCmd "cd ./Server/TapeVerify"
execCmd "./build.sh IBM"
execCmd "./build.sh HP"
execCmd "rm -rf ../../${PACKAGE_FOLDER_VERIFYTOOL}/TapeVerifyTool*.gz"
execCmd "cp TapeVerifyTool*.gz ../../${PACKAGE_FOLDER_VERIFYTOOL}/"
execCmd "cd ../../"
execCmd "cd ./Build/install_packages"
execCmd "tar zcf tlc.${VERSION}.tar.gz TLC"
execCmd "tar zcf  simulator_switcher.${VERSION}.tar.gz simulator_switcher"
execCmd "tar zcf  SWIFT-Tape-Auditor.${VERSION}.tar.gz SWIFT-Tape-Auditor"
execCmd "tar zcf  CentOS7-SWIFT.${VERSION}.tar.gz CentOS7-SWIFT"
exit 0

