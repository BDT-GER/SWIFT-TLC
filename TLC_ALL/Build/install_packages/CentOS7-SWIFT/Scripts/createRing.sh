#!/bin/sh
##
#To Do before run script:
#1. stop openstack service on proxy server and all storage node, 
#   run swift-inin all stop
#2. cleanup builder file of account,container,object
#

if [ -e $1 ];then
	echo Please specify device name, Usage: $0 IP deviceName
	exit 1
fi

echo Start create new builder
cd /etc/swift
echo You are in: `pwd`
rm -rf account.builder container.builder object.builder account.builder.gz container.builder.gz object.builder.gz
swift-ring-builder account.builder create 15 3 1
swift-ring-builder container.builder create 15 3 1
swift-ring-builder object.builder create 15 3 1
swift-ring-builder account.builder add z1-$1:6002/$2 100
swift-ring-builder container.builder add z1-$1:6001/$2 100
swift-ring-builder object.builder add z1-$1:6000/$2 100
swift-ring-builder account.builder
swift-ring-builder container.builder
swift-ring-builder object.builder
