#!/bin/bash 
RED="\e[0;31m"
NOR="\e[0;0m"
GREEN="\e[0;32m"
YELLOW="\e[0;33m"

#IPaddr=10.40.4.22
HostName=`hostname`

CURRENT_PATH=`pwd`

CONF_PATH="${CURRENT_PATH}/Config"
SCRIPTS_PATH="${CURRENT_PATH}/Scripts"
LOG_ROOT="${CURRENT_PATH}/log"
TOTAL_INSTALL_LOG_PATH="${LOG_ROOT}/TOTAL_INSTALL"

SQL_CONF="${CONF_PATH}/my.cnf"

if [ -e ${CURRENT_PATH}/ComFnc ];then
    . ${CURRENT_PATH}/ComFnc "${TOTAL_INSTALL_LOG_PATH}" 
else
    echo "${RED}Common functions script doesn't exist!!!"
    exit 1
fi

function askIPaddr()
{
    ## ip address will be used in following config
    # 1. /etc/my.cnf; 2. /etc/rsyncd.conf; 3. /etc/swift/proxy-server.conf; 4. /etc/swift/account,container,object-server.conf
    logMsg ques "Please input IP address for ${HostName}: " "IP address"
    logMsg info "This IP address will be used in several configuration files: 1. /etc/rsyncd.conf; 2. /etc/swift/proxy-server.conf; 3. /etc/swift/account,container,object-server.conf" "1. /etc/rsyncd.conf; 2. /etc/swift/proxy-server.conf; 3. /etc/swift/account,container,object-server.conf"
    echo -ne "$YELLOW(IP address)$NOR:"
    read IPaddr
    logMsg info "Your Input IP address was: ${IPaddr}" "${IPaddr}"
}

function findAndKillProcess(){
   process_basename="packagekitd"
   ret=`ps -e | grep ${process_basename}`
   if [ $? -eq 0 ];then
        procPid=`echo $ret | awk '{print$1}'`
        logMsg info "Found packageKit process - PID: ${procPid}, Stopping..." "${procPid}"
        kill -9 ${procPid}
        findAndKillProcess ${procPid}
   else
        if [ "$1" ];then
            logMsg success "packagekitd process - PID: ${procPid} has been killed" "${procPid}"
            sleep 2
            findAndKillProcess
        else
            logMsg info "No ${process_basename} porcess is found!" "${process_basename}"
        fi
    fi
}

function ckNetwork()
{
    logMsg info "Connection Checking.."
    ping www.baidu.com -c 5 2>&1 >/dev/null
    if [ $? -ne 0 ];then
	logMsg failure "Connection Check Failure, exiting..."
    fi
    logMsg info "Connection Check Passed."
}

function ckFirewall()
{
    logMsg info "Checking firewalld.service.." "firewalld.service"
    systemctl status firewalld.service
    if [ $? -ne 0 ]; then
	logMsg info "firewalld.service already stopped." "firewalld.service"
    else
	logMsg info "firewalld.service running, will stop it" "firewalld.service"
	systemctl stop firewalld.service
    logMsg info "Disable firewalld.service..." "firewalld.service"
    systemctl disable  firewalld.service
    systemctl status firewalld.service
    logMsg info "firewalld.service stop and disabled." "firewalld.service"
    fi
}

function setHosts()
{
    askIPaddr
    logMsg info "Add IPaddr and Hostname into /etc/hosts"
    echo "${IPaddr}    ${HostName}" >>/etc/hosts
    logMsg info "Set IP and Hostname Mapping Done"
}

function yumInstall()
{
    logMsg info "Installing yum-plugin-priorities.." "yum-plugin-priorities"
    yum install -y yum-plugin-priorities 2>${LOG_ROOT}/yumInstallError.log
    logMsg info "Installing epel-release.." "epel-release"
    yum install -y http://dl.fedoraproject.org/pub/epel/7/x86_64/e/epel-release-7-5.noarch.rpm 2>${LOG_ROOT}/yumInstallError.log
    logMsg info "Installing rdo-release.." "rdo-release"
    yum install -y http://rdo.fedorapeople.org/openstack-juno/rdo-release-juno.rpm 2>${LOG_ROOT}/yumInstallError.log
    logMsg info "Installing openstack-selinux.." "openstack-selinux"
    yum install -y openstack-selinux 2>${LOG_ROOT}/yumInstallError.log
    if [ ${OPT} -eq 3 ]; then
        logMsg info "Installing MySQL-python.." "MySQL-python"
        yum install -y MySQL-python 2>${LOG_ROOT}/yumInstallError.log
    else
        logMsg info "Installing mariadb mariadb-server MySQL-python.." "mariadb mariadb-server MySQL-python"
        yum install -y mariadb mariadb-server MySQL-python 2>${LOG_ROOT}/yumInstallError.log
        logMsg info "Installing rabbitmq-server.." "rabbitmq-server"
        yum install -y rabbitmq-server 2>${LOG_ROOT}/yumInstallError.log
        if [ $? -ne 0 ]; then
	    logMsg info "yum install rabbitmq-server package failed, will try again.." "rabbitmq-server"
	    yum install -y rabbitmq-server 2>${LOG_ROOT}/yumInstallError.log
	    if [ $? -ne 0 ]; then
            	logMsg failure "yum install rabbitmq-server package failed twice! please manual install it and try run the installer again."
	    fi
        fi
    fi
    logMsg info "yumInstall function come to the end" "yumInstall function"
}

function databaseConfig()
{
    logMsg info "Replace the template my.cnf with ${SQL_CONF}" "${SQL_CONF}"
    cp /etc/my.cnf /etc/my.cnf.bak -rf
    cp ${SQL_CONF} /etc/my.cnf -rf
    #logMsg info "Replace Done, add bind-address into /etc/my.cnf" "bind-address"
    #sed -i /^symbolic-links=0/a\bind-address=${IPaddr} /etc/my.cnf
    #cat /etc/my.cnf | grep bind-address
    #if [ $? -ne 0 ]; then
    #	logMsg failure "No bind-address Found in /etc/my.cnf" "bind-address"
    #fi
    logMsg info "Configure /etc/my.cnf Done" "/etc/my.cnf"
    logMsg info "enable and start mariadb.service" "mariadb.service"
    systemctl enable mariadb.service
    systemctl start mariadb.service
    if [ $? -ne 0 ]; then
	logMsg failure "Failed to start mariadb.service, exiting..." "mariadb.service"
    fi
    logMsg info "Config Done, run mysql_secure_installation" "mysql_secure_installation"
    logMsg info "mysql_secure_installation configure produces:" "mysql_secure_installation configure produces:"
    logMsg info "press Enter -> Set root password: Y -> rm anonymous: Y -> remote login disallow: Y -> remove test db: Y -> Reload: Y -> Done" "press Enter -> Set root password: Y -> rm anonymous: Y -> remote login disallow: Y -> remove test db: Y -> Reload: Y -> Done"
    ##mysql_secure_installation configure produces:
    #press Enter -> Set root password: Y -> rm anonymous: Y -> remote login disallow: Y -> remove test db: Y -> Reload: Y -> Done
    #
    mysql_secure_installation
    if [ $? -ne 0 ]; then
	logMsg failure "mysql_secure_installation config failed" "mysql_secure_installation"
    fi
    logMsg info "databaseConfig function Done" "databaseConfig"
}

function rabbitmqConfig()
{
    logMsg info "enable and start rabbitmq-server.service" "rabbitmq-server"
    systemctl enable rabbitmq-server.service
    systemctl start rabbitmq-server.service
    rabbitmqctl change_password guest guest
    logMsg info "rabbitmq-server config done" "rabbitmq-server"
}

##Keystone Setup
function keystoneDBConfig()
{
    logMsg info "Going to create keystone DB and grant privileges" "create keystone DB"
    rootHasPwd=0
    while [ $rootHasPwd -eq 0 ]
    do
    need_password=`mysql -u root test -e "show tables;" 2>/dev/null`
    if [ $? -ne 0 ];then
        logMsg info "Your MySQL database user root need a password to login, please input your password: " "root" "n"
        read -s  mysqlpwd
        ret=`mysql -u root -p${mysqlpwd} mysql -e "show tables;" 2>/dev/null`
        ret=$?
        if [ $ret -ne 0 ];then
            echo && logMsg info "Input password for root incorrect... please enter again."
            continue
        else
            echo && logMsg info "Input password for root correct."
            cmdMysql="-p${mysqlpwd}"
            rootHasPwd=1
            sleep 1
        fi
    else
        logMsg info "Default password for MySQL root user is blank."
        cmdMysql=""
        break
    fi
    done
    logMsg info "Creating Keystone DataBase schema."
    mysql -u root ${cmdMysql}<<EOF
CREATE DATABASE keystone;
GRANT ALL PRIVILEGES ON keystone.* TO 'keystone'@'localhost' IDENTIFIED BY 'keystone';
GRANT ALL PRIVILEGES ON keystone.* TO 'keystone'@'%' IDENTIFIED BY 'keystone';
EOF
    logMsg info "Function keystoneDBConfig Done" "keystoneDBConfig"
}

function keystonePkgInst()
{
    #hardcode keystone user's password as keystone
    logMsg info "Installing packages: openstack-keystone python-keystoneclient" "openstack-keystone python-keystoneclient"
    yum install -y openstack-keystone python-keystoneclient 2>${LOG_ROOT}/yumInstallError.log
    logMsg info "Install Done, updating keystone.conf" "keystone.conf"
    sed -i '/^\[DEFAULT\]/a\admin_token = admin' /etc/keystone/keystone.conf

    sed -i '/^\[database\]/a\connection = mysql\://keystone:keystone@ToBeReplaceHostName/keystone' /etc/keystone/keystone.conf
    sed -i s/ToBeReplaceHostName/${HostName}/g /etc/keystone/keystone.conf
    sed -i '/^\[token\]/a\driver = keystone.token.persistence.backends.sql.Token' /etc/keystone/keystone.conf
    sed -i '/^\[token\]/a\provider = keystone.token.providers.uuid.Provider' /etc/keystone/keystone.conf

    logMsg info "Configuring keystone: pki_setup" "keystone"
    keystone-manage pki_setup --keystone-user keystone --keystone-group keystone
    logMsg info "chown owner and group of /var/log/keystone" "chown owner and group"
    chown -R keystone:keystone /var/log/keystone
    logMsg info "Configuring /etc/keystone/ssl" "/etc/keystone/ssl"
    chown -R keystone:keystone /etc/keystone/ssl
    chmod -R o-rwx /etc/keystone/ssl
    logMsg info "Run keystone-manage db_sync" "db_sync"
    su -s /bin/sh -c "keystone-manage db_sync" keystone

    logMsg info "Enable and Start openstack-keystone.service." "openstack-keystone.service"
    systemctl enable openstack-keystone.service
    systemctl start openstack-keystone.service
    logMsg info "Enable and start openstack-keystone.service Done" "openstack-keystone.service"
    (crontab -l -u keystone 2>&1 | grep -q token_flush) || echo '@hourly /usr/bin/keystone-manage token_flush >/var/log/keystone/keystone-tokenflush.log 2>&1' >> /var/spool/cron/keystone
    logMsg info "function keystonePkgInst Done" "keystonePkgInst"
}

function keystoneAddInfo()
{
    logMsg info "Export Token and EndPoint"
    export OS_SERVICE_TOKEN=admin
    export OS_SERVICE_ENDPOINT=http://${HostName}:35357/v2.0
    logMsg info "Creating admin Tenant.." "admin Tenant"
    keystone tenant-create --name admin --description "Admin Tenant"
    if [ $? -ne 0 ]; then
    	logMsg failure "Keystone create admin Tenant Failed, normally this should NOT happen here, please check, exiting..."
    fi
    logMsg info "Creating admin user..." "admin user"
    keystone user-create --name admin --pass admin --email admin@swift.com
    logMsg info "Creating admin Role..." "admin Role"
    keystone role-create --name admin
    logMsg info "Adding admin tenant and user to admin Role" "admin"
    keystone user-role-add --tenant admin --user admin --role admin
    
    logMsg info "Creating _member_ Role" "_member_ Role"
    keystone role-create --name _member_
    logMsg info "Adding admin tenant and user to _member_ Role.." "_member_"
    keystone user-role-add --tenant admin --user admin --role _member_

    logMsg info "Creating demo tenant, user, Role" "demo"
    keystone tenant-create --name demo --description "Demo Tenant"
    keystone user-create --name demo --pass demo --email demo@swift.com
    keystone user-role-add --tenant demo --user demo --role _member_

    logMsg info "Creating service Tenant..." "service Tenant"
    keystone tenant-create --name service --description "Service Tenant"

    logMsg info "Creating keystone service..." "keystone service"
    keystone service-create --name keystone --type identity --description "OpenStack Identity"

    logMsg info "Creating keystone EndPoint.." "keystone EndPoint"
    keystone endpoint-create --service-id=$(keystone service-list | awk '/ identity / {print $2}') --publicurl=http://${HostName}:5000/v2.0 --internalurl=http://${HostName}:5000/v2.0 --adminurl=http://${HostName}:35357/v2.0
    logMsg info "Unset temporary environment variables: OS_SERVICE_TOKEN OS_SERVICE_ENDPOINT" "OS_SERVICE_TOKEN OS_SERVICE_ENDPOINT"
    unset OS_SERVICE_TOKEN OS_SERVICE_ENDPOINT
    logMsg info "Function keystoneAddInfo Done" "keystoneAddInfo"
}

##
## Swift Setup and Configured
##
function swiftSetupInit()
{
    #export environment variables
    logMsg info "Function swiftSetupInit start.." "swiftSetupInit"
    logMsg info "export environment variables: OS_USERNAME,OS_PASSWORD, OS_TENANT_NAME, OS_AUTH_URL" "OS_USERNAME,OS_PASSWORD, OS_TENANT_NAME, OS_AUTH_URL"
    export OS_USERNAME=admin
    export OS_PASSWORD=admin
    export OS_TENANT_NAME=admin
    export OS_AUTH_URL=http://${HostName}:35357/v2.0

    logMsg info "Create swift user" "swift"
    keystone user-create --name swift --pass swift
    logMsg info "Add user swift to service tenant and admin role" "swift"
    keystone user-role-add --user swift --tenant service --role admin
    logMsg info "Create OpenStack Object Storage Service." "OpenStack Object Storage"
    keystone service-create --name swift --type object-store --description "OpenStack Object Storage"
    logMsg info "Create OpenStack Swift Endpoint." "Swift"
    keystone endpoint-create --service-id=$(keystone service-list | awk '/ object-store / {print $2}') --publicurl=http://${HostName}:8080'/v1/AUTH_%(tenant_id)s' --internalurl=http://${HostName}:8080'/v1/AUTH_%(tenant_id)s' --adminurl=http://${HostName}:8080
    logMsg info "Function swiftSetupInit End.." "swiftSetupInit"
}

function proxyConfig()
{
    logMsg info "Function proxyConfig start.." "proxyConfig"
    logMsg info "yum install required packages: openstack-swift-proxy python-swiftclient python-keystoneauth-token memcached"  "openstack-swift-proxy python-swiftclient python-keystoneauth-token memcached"
    yum install -y openstack-swift-proxy python-swiftclient python-keystoneauth-token memcached 2>${LOG_ROOT}/yumInstallError.log

    logMsg info "updating /etc/swift/proxy-server.conf.." "/etc/swift/proxy-server.conf"
    mkdir -p /etc/swift/bak
    mv /etc/swift/swift.conf /etc/swift/bak
    cp ${CONF_PATH}/swift.conf /etc/swift -rf
    mv /etc/swift/proxy-server.conf /etc/swift/bak
    logMsg info "backup proxy-server.conf done, copy the template file /etc/swift/proxy-server.conf"
    cp ${CONF_PATH}/proxy-server.conf /etc/swift -rf
    logMsg info "Editing memcache_server, identity_uri,auth_rui.." "memcache_server, identity_uri,auth_rui"
    sed -i /SwiftAutoAddMemcacheServerBelow/a\memcache_servers=${IPaddr}:11211 /etc/swift/proxy-server.conf
    sed -i /SwiftAutoAddAuthtokenBelow/a\identity_uri=http://${HostName}:35357 /etc/swift/proxy-server.conf
    sed -i /SwiftAutoAddAuthtokenBelow/a\auth_uri=http://${HostName}:5000/v2.0 /etc/swift/proxy-server.conf

    logMsg info "Changing node_timeout setting..." "node_timeout"
    grep "# node_timeout = 10" /etc/swift/proxy-server.conf
    if [ $? -eq 0 ]; then
        sed -i 's/# node_timeout = 10/node_timeout = 300/g' /etc/swift/proxy-server.conf
    fi

    # logMsg info "Changing conn_timeout setting..." "node_timeout"
    # grep "# conn_timeout = 0.5" /etc/swift/proxy-server.conf
    # if [ $? -eq 0 ]; then
    #     sed -i 's/# conn_timeout = 0.5/conn_timeout = 30/g' /etc/swift/proxy-server.conf
    # fi
    logMsg info "Enable and start memcached.service.." "memcached.service"
    systemctl enable memcached.service
    systemctl start memcached.service
}

function swiftNodeConfig()
{
    logMsg info "Function swiftNodeConfig start..." "swiftNodeConfig"
    logMsg info "yum install packages: openstack-swift-account openstack-swift-container openstack-swift-object python-swiftclient python-keystoneauth-token memcached" "openstack-swift-account openstack-swift-container openstack-swift-object python-swiftclient python-keystoneauth-token memcached"
    yum install -y openstack-swift-account openstack-swift-container openstack-swift-object python-swiftclient python-keystoneauth-token memcached 2>${LOG_ROOT}/yumInstallError.log
    mkdir -p /etc/swift/bak

    if [ ${OPT} -eq 3 ];then
    	logMsg info "using default /etc/swift/swift.conf.." "/etc/swift/swift.conf"
    	cp ${CONF_PATH}/swift.conf.default /etc/swift/swift.conf -rf
    fi
    logMsg info "backup account,container, object-server.conf to /etc/swift/bak" "account,container, object-server.conf"
    mv /etc/swift/account-server.conf /etc/swift/bak
    mv /etc/swift/container-server.conf /etc/swift/bak
    mv /etc/swift/object-server.conf /etc/swift/bak
    logMsg info "updating bind_ip in account,container, object-server.conf" "account,container, object-server.conf"
    cp ${CONF_PATH}/account-server.conf /etc/swift/account-server.conf -rf
    sed -i /SwiftAutoAddBindIPBlow/a\bind_ip=${IPaddr} /etc/swift/account-server.conf
    cp ${CONF_PATH}/container-server.conf /etc/swift/container-server.conf -rf
    sed -i /SwiftAutoAddBindIPBlow/a\bind_ip=${IPaddr} /etc/swift/container-server.conf
    cp ${CONF_PATH}/object-server.conf /etc/swift/object-server.conf -rf
    sed -i /SwiftAutoAddBindIPBlow/a\bind_ip=${IPaddr} /etc/swift/object-server.conf
    ##
    # logMsg info "Changing sync_method to ssync.." "sync_method to ssync"
    # grep "# sync_method = rsync" /etc/swift/object-server.conf
    # if [ $? -eq 0 ]; then
    #     sed -i 's/# sync_method = rsync/sync_method = ssync/g' /etc/swift/object-server.conf
    # fi

    logMsg info "chown -R swift:swift /srv/node" "chown"
    mkdir -p /srv/node/vsnode
    chown -R swift:swift /srv/node
    logMsg info "chown -R swift:swift /var/cache/swift/" "chown"
    mkdir -p /var/cache/swift/
    chown -R swift:swift /var/cache/swift/

    logMsg info "updating /etc/rsyncd.conf.." "/etc/rsyncd.conf"
    mv /etc/rsyncd.conf /etc/rsyncd.conf.bak
    logMsg info "backup original rsyncd.conf file done, copy the template file.."
    cp ${CONF_PATH}/rsyncd.conf /etc/rsyncd.conf -rf
    logMsg info "Editing address = <ip> in rsyncd.conf" "rsyncd.conf"
    sed -i /SwiftAutoAddAddressBelow/a\address=${IPaddr} /etc/rsyncd.conf

    logMsg info "enable and start rsyncd.service.." "rsyncd.service"
    systemctl enable rsyncd.service
    systemctl start rsyncd.service

    logMsg info "Function swiftNodeConfig End." "swiftNodeConfig"
}

function ckSwiftDevice()
{
    #swift ring device need to be mounted at /srv/node, the device name i.e. sdb1 will be used when create ring
    logMsg info "swift ring device need to be mounted at /srv/node in xfs format, the device name i.e. sdb1" "/srv/node"
    logMsg ques "Please Input Device Name i.e. sdb1: " "Device Name i.e. sdb1"
    echo -ne "$YELLOW(Device Name)$NOR:"
    read DevName
    logMsg info "Your Input Device Name was: ${DevName}" "${DevName}"
    logMsg info "Going to check if /srv/node/${DevName} was mounted" "/srv/node/${DevName}"
    _ret=`mount | grep "/srv/node/${DevName}" | head -n 1`
    if [ -z "${_ret}" ]; then
	logMsg fail "Specified device /srv/node/${DevName} was NOT mounted, exiting..." "/srv/node/${DevName}"
    fi
    getDev=`mount | grep "/srv/node/${DevName}" | awk '{print $3}' | cut -f 4 -d '/' | head -n 1`
    if [ "${getDev}" != "${DevName}" ]; then
	logMsg fail "Specified device name: [${DevName}] was NOT the same as real mounted device: [${getDev}]" "NOT the same"
    fi
    logMsg info "Going to check if /srv/node/${DevName} was in XFS format" "XFS"
    fsType=`mount | grep "/srv/node/${DevName}" | awk '{print $5}'`
    #if [ "${fsType}" != "xfs" ]; then
    #	logMsg fail "Specified device /srv/node/${DevName} was NOT in XFS format, exiting..." "/srv/node/${DevName}"
    #fi
    logMsg info "swift device /srv/node/${DevName} check passed" "/srv/node/${DevName}"
}

function swiftRingConfig()
{
    #will go to /etc/swift create swift ring
    if [ -z "${DevName}" ]; then
	logMsg info "NO Device name specified yet, will use vsnode as device name." "vsnode"
	DevName="vsnode"
    fi
    if [ -z "${IPaddr}" ]; then
	logMsg info "NO IP address specified for device: ${DevName} yet" "${DevName}"
	askIPaddr
    fi
    logMsg info "Function swiftRingConfig start..." "swiftRingConfig"
    logMsg info "Go to /etc/swift location" "/etc/swift"
    cd /etc/swift
    echo You are in: `pwd`
    logMsg info "Remove old ring builder files" "ring builder"
    rm -rf account.builder container.builder object.builder account.builder.gz container.builder.gz object.builder.gz
    logMsg info "Create account, container, object builder. " "account, container, object builder."
    swift-ring-builder account.builder create 15 3 1
    swift-ring-builder container.builder create 15 3 1
    swift-ring-builder object.builder create 15 3 1
    logMsg info "add specified device: ${DevName} to builder.." "${DevName}"
    swift-ring-builder account.builder add z1-${IPaddr}:6002/${DevName} 100
    swift-ring-builder container.builder add z1-${IPaddr}:6001/${DevName} 100
    swift-ring-builder object.builder add z1-${IPaddr}:6000/${DevName} 100
    logMsg info "Verified what we had added to the builder.." "builder"
    swift-ring-builder account.builder
    swift-ring-builder container.builder
    swift-ring-builder object.builder
    logMsg info "rebalance the ring builder.." "rebalance"
    swift-ring-builder account.builder rebalance
    swift-ring-builder container.builder rebalance
    swift-ring-builder object.builder rebalance
    logMsg info "chown -R swift:swift /etc/swift" "chown -R swift:swift /etc/swift"
    chown -R swift:swift /etc/swift
    logMsg info "Function swiftRingConfig End." "swiftRingConfig"
}

function _opSetupInit()
{
    #use yum command install some required packages, so make sure Internet connection available
    #ckNetwork
    ckFirewall
    findAndKillProcess
    setHosts
    yumInstall
    databaseConfig
    rabbitmqConfig
}

function _opKeystoneSetup()
{
    # Keystone Setup
    keystoneDBConfig
    keystonePkgInst
    keystoneAddInfo
}

function _opSwiftSetup()
{
    # Swift Setup
    swiftSetupInit
    proxyConfig
    swiftNodeConfig
}

function _opSwiftRingSetup()
{
    ckSwiftDevice
    swiftRingConfig
}

function _opDashboardSetup()
{
    logMsg info "start function _opDashboardSetup.." "_opDashboardSetup"
    logMsg info "installing required packages: openstack-dashboard httpd mod_wsgi memcached pythonmemcached" "openstack-dashboard httpd mod_wsgi memcached pythonmemcached"
    yum install -y openstack-dashboard httpd mod_wsgi memcached python-memcached 2>${LOG_ROOT}/yumInstallError.log
    logMsg info "Start to configure the dashboard /etc/openstack-dashboard/local_settings.." "/etc/openstack-dashboard/local_settings"
    cp ${CONF_PATH}/local_settings /etc/openstack-dashboard/local_settings -rf
    reStr="OPENSTACK_HOST=\"${HostName}\""
    sed -i /SwiftAutoAddHOSTBlow/a\\$reStr  /etc/openstack-dashboard/local_settings
    logMsg info "Configure SELinux to permit the web server to connect to OpenStack services: setsebool -P httpd_can_network_connect on" "setsebool -P httpd_can_network_connect on"
    setsebool -P httpd_can_network_connect on
    if [ $? -ne 0 ]; then
	logMsg info "Failed Command: setsebool -P httpd_can_network_connect on" "setsebool -P httpd_can_network_connect on"
    else
	logMsg info "Command successful Done"
    fi
    logMsg info "Run:chown -R apache:apache /usr/share/openstack-dashboard/static" "chown -R apache:apache /usr/share/openstack-dashboard/static"
    chown -R apache:apache /usr/share/openstack-dashboard/static
    logMsg info "Enable and Start httpd.service" "httpd.service"
    systemctl enable httpd.service
    systemctl start httpd.service
    if [ $? -ne 0 ]; then
	logMsg failure "Service httpd.service start failure, exiting..."
    fi
    logMsg info "Dashboard configured done, after start all swift service, you can access via web browser: http://${HostName}/dashboard or http://${IPaddr}/dashboard" "http://${HostName}/dashboard or http://${IPaddr}/dashboard" 
}

function _opGenerateScript()
{
    logMsg info "Generating client environment script for admin, sript: admin-openrc.sh" "admin-openrc.sh"
    logMsg info "Copying ${SCRIPTS_PATH}/admin-openrc.sh to /root" "${SCRIPTS_PATH}/admin-openrc.sh"
    cp ${SCRIPTS_PATH}/admin-openrc.sh /root/admin-openrc.sh -rf
    logMsg info "adding OS_AUTH_URL to admin-openrc.sh" "OS_AUTH_URL"
    echo "export OS_AUTH_URL=http://${HostName}:5000/v2.0" >>/root/admin-openrc.sh
    logMsg info "Execute SWIFT CLI script to set environment variables: source /root/admin-openrc.sh" "source /root/admin-openrc.sh"
    source /root/admin-openrc.sh
    logMsg info "Copying swift start all script: startSwift.sh to /tmp" "startSwift.sh"
    cp ${SCRIPTS_PATH}/startSwift.sh /tmp/startSwift.sh -rf
    logMsg info "Copying swift stop all script: stopSwift.sh to /tmp" "stopSwift.sh"
    cp ${SCRIPTS_PATH}/stopSwift.sh /tmp/stopSwift.sh -rf
    if [ ${OPT} -eq 2 ]; then
	echo "swift-init proxy-server start" >>/tmp/startSwift.sh
	echo "swift-init proxy-server stop" >>/tmp/stopSwift.sh
    else
	echo "swift-init all start" >>/tmp/startSwift.sh
        echo "swift-init all stop" >>/tmp/stopSwift.sh
    fi 
    logMsg info "Copying swift ring create script: createRing.sh to /tmp" "createRing.sh"
    cp ${SCRIPTS_PATH}/createRing.sh /tmp/createRing.sh -rf
    logMsg info "Copying swift ring add script: addRing.sh to /tmp" "addRing.sh"
    cp ${SCRIPTS_PATH}/addRing.sh /tmp/addRing.sh -rf
    logMsg info "Copying swift ring rebalance script: rebalanceRing.sh to /tmp" "rebalanceRing.sh"
    cp ${SCRIPTS_PATH}/rebalanceRing.sh /tmp/rebalanceRing.sh -rf
    logMsg info "Function _opGenerateScript Done" "_opGenerateScript"
}

function rmKeystoneDB()
{
    logMsg info "Function rmKeystoneDB start.." "rmKeystoneDB"
    rootHasPwd=0
    while [ $rootHasPwd -eq 0 ]
    do
    need_password=`mysql -u root test -e "show tables;" 2>/dev/null`
    if [ $? -ne 0 ];then
        logMsg info "Your MySQL database user root need a password to login, please input your password: " "root" "n"
        read -s  mysqlpwd
        ret=`mysql -u root -p${mysqlpwd} mysql -e "show tables;" 2>/dev/null`
        ret=$?
        if [ $ret -ne 0 ];then
            echo && logMsg info "Input password for root incorrect... please enter again."
            continue
        else
            echo && logMsg info "Input password for root correct."
            cmdMysql="-p${mysqlpwd}"
            rootHasPwd=1
            sleep 1
        fi
    else
        logMsg info "Default password for MySQL root user is blank."
        cmdMysql=""
        break
    fi
    done

    logMsg info "Drop database: keystone" "keystone"
    ret=`mysql -u root -p${mysqlpwd} -e "drop database keystone;"`

    logMsg info "Delete keystone user info in User table of mysql DB" "keystone user info"
    ret=`mysql -u root -p${mysqlpwd} mysql -e "delete from user where User='keystone';"`
    logMsg info "Function rmKeystoneDB End" "rmKeystoneDB"
}

function rmYumPackages()
{
    # function call when uninstall openstack swift, remove related packages.
    logMsg info "removing swift related packages: openstack-swift-proxy openstack-swift-account openstack-swift-container openstack-swift-object python-swiftclient" "openstack-swift-proxy openstack-swift-account openstack-swift-container openstack-swift-object python-swiftclient"
    yum remove -y openstack-swift-proxy openstack-swift-account openstack-swift-container openstack-swift-object python-swiftclient 2>${LOG_ROOT}/yumRemoveError.log

    logMsg info "removing packages: python-keystoneauth-token" "python-keystoneauth-token"
    yum remove -y python-keystoneauth-token 2>${LOG_ROOT}/yumRemoveError.log

    logMsg info "removing packages: memcached" "memcached"
    yum remove -y memcached 2>${LOG_ROOT}/yumRemoveError.log

    logMsg info "removing keystone related  packages: openstack-keystone python-keystoneclient" "openstack-keystone python-keystoneclient"
    yum remove -y openstack-keystone python-keystoneclient 2>${LOG_ROOT}/yumRemoveError.log

    logMsg info "removing database related packages: rabbitmq-server MySQL-python" "rabbitmq-server MySQL-python"
    yum remove -y rabbitmq-server MySQL-python  2>${LOG_ROOT}/yumRemoveError.log

    logMsg info "removing package: openstack-selinux" "openstack-selinux"
    yum remove -y openstack-selinux 2>${LOG_ROOT}/yumRemoveError.log

    logMsg info "removing package: rdo-release-juno.." "rdo-release-juno"
    yum remove -y rdo-release-juno 2>${LOG_ROOT}/yumRemoveError.log

    logMsg info "removing packages:epel-release yum-plugin-priorities" "epel-release yum-plugin-priorities"
    yum remove -y epel-release yum-plugin-priorities 2>${LOG_ROOT}/yumRemoveError.log
    logMsg info "Yum remove openstack swift related packages Done" "openstack swift related packages"
}

function rmSwiftConfig()
{
    # remove swift related configuration files
    logMsg info "removing /etc/swift folder.." "/etc/swift"
    rm /etc/swift -rf
    logMsg info "recovering original: /etc/rsyncd.conf.." "/etc/rsyncd.conf"
    mv /etc/rsyncd.conf.bak /etc/rsyncd.conf
    logMsg info "stop and disable rsyncd.service.." "rsyncd.service"
    systemctl stop rsyncd.service
    systemctl disable rsyncd.service
    logMsg info "removing host mapping in /etc/hosts." "/etc/hosts"
    sed -i "/${HostName}/d" /etc/hosts

}

function rmDashboardConfig()
{
    rpm -qa | grep -E "dashboard|mod_wsgi"
    if [ $? -ne 0 ]; then
	logMsg info "NO openstack-dashboard configured" "openstack-dashboard"
	logMsg info "function rmDashboardConfig Done" "rmDashboardConfig"
    else
	logMsg info "stoping and disable httpd.service.." "httpd.service"
	systemctl stop httpd.service
	systemctl disable httpd.service
	logMsg info "Removing openstack-dashboard components..." "openstack-dashboard"
	yum remove -y openstack-dashboard httpd mod_wsgi python-memcached  2>${LOG_ROOT}/yumRemoveError.log	
	logMsg info "Removing /etc/openstack-dashboard dir" "/etc/openstack-dashboard"
	rm /etc/openstack-dashboard -rf
	logMsg info "function rmDashboardConfig Done" "rmDashboardConfig"
    fi
    
}

function main_allinone()
{
    # Configure OpenStack Proxy,Keystone and Swift Node in one
    # This require TLC install first
    _opSetupInit
    _opKeystoneSetup
    _opSwiftSetup
    # generate swift ring with /srv/node/vsnode
    # _opSwiftRingSetup
     swiftRingConfig
    _opDashboardSetup
    # generate sripts use for create swift ring or start/stop swift services
    _opGenerateScript
}

function main_controller()
{
    # Configure OpenStack Proxy and Keysotne as controller
    _opSetupInit
    _opKeystoneSetup
    # _opSwiftSetup without swiftNodeConfig
     swiftSetupInit
     proxyConfig
    _opDashboardSetup
    # generate sripts use for create swift ring or start/stop swift services 
    _opGenerateScript
}

function main_node()
{
   # Only Configure Swift Node
   # main_server
   #_opSetupInit  # try no need db config: databaseConfig
    #ckNetwork
    ckFirewall
    findAndKillProcess
    setHosts
    yumInstall
   # rabbitmqConfig
   # keystoneDBConfig
   # keystonePkgInst
   swiftNodeConfig 
   # yum remove -y openstack-swift-proxy openstack-keystone 2>${LOG_ROOT}/yumRemoveError.log
   # yum remove -y openstack-keystone 2>${LOG_ROOT}/yumRemoveError.log
   # remove keystone database
   # rmKeystoneDB 
}

function main_uninstall()
{
    # Uninstall OpenStack Swift
    findAndKillProcess
    rpm -qa | grep -E "openstack-swift-proxy|openstack-keystone"
    if [ $? -ne 0 ]; then
    	rmYumPackages
	rmSwiftConfig
    else
	rmYumPackages
	rmSwiftConfig
	rmKeystoneDB
    fi
    rmDashboardConfig
}

function startSWIFTInstaller()
{
    logMsg ques "${GREEN}Please Select SWIFT Installer Option: ${NOR}"
    echo -ne "  1): ${YELLOW} SWIFT All in one ${NOR}
        \r  2): ${YELLOW} SWIFT PROXY + Keystone ${NOR}
	\r  3): ${YELLOW} SWIFT Storage Node ${NOR}
	\r  4): ${YELLOW} Uninstall ${NOR}
        \rInput ${YELLOW}'C'${NOR} to cancel and exit.${YELLOW}(1/2/3/4/C)${NOR}: "
    read OPT
    case $OPT in
        1)
            logMsg info "You are selecting to install SWIFT ALL in one." "SWIFT ALL in one"
            main_allinone
            logMsg info "You can continue configure TLC Tape Auditor package, and then start vs service." "TLC Tape Auditor"
            logMsg info "Further, you can export Environment variables by command: source /root/admin-openrc.sh" "source /root/admin-openrc.sh"
            logMsg info "Then, you can now start all swift services by run: swift-init all start" "swift-init all start"
        ;;
        2)
            logMsg info "You are selecting to install SWIFT PROXY + Keystone." "SWIFT PROXY + Keystone"
            main_controller
            logMsg info "You Environment are ready for create Swift Ring." "Swift Ring"
            logMsg info "you can export Environment variables by command: source /root/admin-openrc.sh" "source /root/admin-openrc.sh"
	    logMsg info "Then, you can now start swift proxy services by run: swift-init proxy-server start" "swift-init proxy-server start"
	;;
        3)
            logMsg info "You are selecting to install OpenStack SWIFT Storage Node Only." "SWIFT Storage Node"
            main_node
            logMsg info "Now you can create swift ring from server and deploy it to this node." "swift ring from server and deploy"
	    logMsg info "Then, you can now start all swift services by run: swift-init all start" "swift-init all start"
        ;;
        4)
	    logMsg info "You are selecting to Uninstall SWIFT components" "Uninstall SWIFT"
	    if [ -e '/usr/bin/swift-init' ]; then
            	logMsg info "Try to stop all swift service: swift-init all stop" "swift-init all stop"
            	swift-init all stop
            	main_uninstall
            	logMsg info "Uninstall OpenStack Swift components Done." "OpenStack Swift components"
	    else
		logMsg info "It looks NOT SWIFT components installed, please manual check and try again." "NOT SWIFT components"
		exit 0
	    fi
	;;	    
        c|C|cancel|Cancel|CANCEL)
            logMsg warn "You've canceled the SWIFT Installer."
            exit 0
        ;;
        *)
	    echo -ne "  1): ${YELLOW} SWIFT All in one ${NOR}
	        \r  2): ${YELLOW} SWIFT PROXY + Keystone ${NOR}
	        \r  3): ${YELLOW} SWIFT Storage Node ${NOR}
	        \r  4): ${YELLOW} Uninstall ${NOR}
	        \rInput ${YELLOW}'C'${NOR} to cancel and exit.${YELLOW}(1/2/3/4/C)${NOR}: "
        ;;
    esac
}

startSWIFTInstaller
exit $?

