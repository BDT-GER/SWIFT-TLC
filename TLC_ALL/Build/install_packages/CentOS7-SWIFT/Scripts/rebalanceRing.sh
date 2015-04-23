#!/bin/sh
##

echo Start rebalance container, account, object rings
cd /etc/swift
echo You are in: `pwd`
swift-ring-builder account.builder
swift-ring-builder container.builder
swift-ring-builder object.builder
swift-ring-builder account.builder rebalance
swift-ring-builder container.builder rebalance
swift-ring-builder object.builder rebalance
echo change owner and group for /etc/swift path
chown -R swift:swift /etc/swift
#scp /etc/swift/*.gz 10.40.3.124:/etc/swift
