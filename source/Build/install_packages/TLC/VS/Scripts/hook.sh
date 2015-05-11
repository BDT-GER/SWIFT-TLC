#!/bin/bash

PRODUCT_NAME="VS"
PRODUCT_NAME_LOWCASE="vs"
INST_ROOT_PATH=$1
LTFS_SOURCE="${INST_ROOT_PATH}/${PRODUCT_NAME}"
LTFS_SOURCE_UDB="${LTFS_SOURCE}/db"
LTFS_SOURCE_CFG="${LTFS_SOURCE}/Config"
LTFS_SOURCE_SCRIPTS="${LTFS_SOURCE}/Scripts"
LTFS_SOURCE_GUI="${LTFS_SOURCE}/RMU"
LTFS_SOURCE_BIN="${LTFS_SOURCE}/bin"

DISKCACHEPATH="/opt/${PRODUCT_NAME}/ltfsCache/diskCache"
METACACHEPATH="/opt/${PRODUCT_NAME}/ltfsCache/meta"
LTFS_INSTALL_ROOT="/usr/${PRODUCT_NAME}"
LTFS_INSTALL_GUI="${LTFS_INSTALL_ROOT}/RMU"
LTFS_INSTALL_CFG="${LTFS_INSTALL_ROOT}/conf"
LTFS_INSTALL_SCRIPTS="${LTFS_INSTALL_ROOT}/scripts"
LTFS_INSTALL_UDB="${METACACHEPATH}/db"
VFS_ROOT="${LTFS_INSTALL_ROOT}/vfs"
BIN_ROOT="${LTFS_INSTALL_ROOT}/bin"
VFS_TARGET_PERMISSION_CTRL_FOLDER="/opt/${PRODUCT_NAME}/Storage/"
VFS_TARGET_FOLDER="/opt/${PRODUCT_NAME}/Storage/VFS"
VFS_BIN_NAME="vfsserver"
VFS_CLN_BIN_NAME="vfsclient"
LTFS_TAPE_MOUNT_POINT="/opt/${PRODUCT_NAME}/ltfsMounts"
DIRECT_ACCESS_PATH="/opt/${PRODUCT_NAME}/DirectTapeAccess"
CATALOG_PATH="$METACACHEPATH/.catalog"
PYTHON_LIB_HOME="/usr/local/lib64/python2.6/site-packages"

CRTS_PATH_KEY="/etc/apache2/ssl.key/"
CRTS_PATH_CRT="/etc/apache2/ssl.crt/"
THIRD_PARTY="${INST_ROOT_PATH}/3rd_Party"
LIB="${THIRD_PARTY}/lib"
RPMS="${THIRD_PARTY}/RPMs"
ZYPPER_REPO_TEMP="${RPMS}/repo"
HPLTFS_RPM="${RPMS}/rpm4HpLtfs"
IBM_LINTAPE_RPM="${RPMS}/rpm4IbmSe/lin_tape"
IBM_SE_RPM="${RPMS}/rpm4IbmSe/SE"
SMB_RPMS="${RPMS}/rpm4smb"
SMB_PLUGIN="${THIRD_PARTY}/Plugins/plugin4smb"
TARPKG_ROOT="${THIRD_PARTY}/Tars"

LOG_ROOT="${INST_ROOT_PATH}/log"
TOTAL_INSTALL_LOG_PATH="${LOG_ROOT}/TOTAL_INSTALL"

SMB_CFG_DEST="/etc/samba/smb4${PRODUCT_NAME_LOWCASE}.conf"
SMB_CFG_SRC="$LTFS_SOURCE_CFG/smb4${PRODUCT_NAME_LOWCASE}.conf"
APACHE_CFG_SRC="$LTFS_SOURCE_CFG/apache2"
APACHE_CFG_DEST="/etc/sysconfig/apache2"
APACHE_CFG_VH_SRC="$LTFS_SOURCE_CFG/${PRODUCT_NAME}_VirtualHost.conf"
APACHE_CFG_VH_DEST="/etc/apache2/vhosts.d/${PRODUCT_NAME}_VirtualHost.conf"
LOG4CPLUS_CFG_SRC="log4cplus.cfg"
LTFS_CFG_SRC="${PRODUCT_NAME_LOWCASE}_conf.xml"
DISKSPACE_MONITOR_CFG_SRC="diskSpaceMonitor.cfg"
LOGROTATE_CFG_SRC="${PRODUCT_NAME_LOWCASE}"
LOGROTATE_CFG_DEST="/etc/logrotate.d"
ADMIN_USER_NAME="${PRODUCT_NAME_LOWCASE}admin"
ADMIN_GROUP_NAME="${PRODUCT_NAME_LOWCASE}admin"
USRES_GROUP_NAME="${PRODUCT_NAME_LOWCASE}users"
LTFSGROUPS="$ADMIN_GROUP_NAME $USRES_GROUP_NAME"
LTFS_DEFAULT_VENDOR='IBM'
CRONFILE="/etc/crontab"
#########################################################################################

function getPkgVersion(){
    versionFile=$INST_ROOT_PATH/version
    if [ -e $INST_ROOT_PATH/version ];then
        PKG_MAINVER=`cat $versionFile | grep MAIN_VERSION | cut -f2 -d'='`
        PKG_SUBVER=`cat $versionFile | grep SUB_VERSION | cut -f2 -d'='`
        PKG_BUILDNUM=`cat $versionFile | grep BUILD_NUMBER | cut -f2 -d'='`
        PKG_VERSION="$PKG_MAINVER.$PKG_SUBVER.$PKG_BUILDNUM"
    fi
}

function getInstallVersion(){
    instVerFile=$LTFS_INSTALL_ROOT/version
    if [ -e $instVerFile ];then
	INST_MAINVER=`cat $instVerFile | grep MAIN_VERSION | cut -f2 -d'='`
	INST_SUBVERSION=`cat $instVerFile | grep SUB_VERSION | cut -f2 -d'='`
	INST_BUILDNUM=`cat $instVerFile | grep BUILD_NUMBER | cut -f2 -d'='`
	INST_VERSION="$INST_MAINVER.$INST_SUBVERSION.$INST_BUILDNUM"
    fi    
}
getPkgVersion
getInstallVersion

if [ -e ${LTFS_SOURCE_SCRIPTS}/ComFnc ];then
    . ${LTFS_SOURCE_SCRIPTS}/ComFnc $TOTAL_INSTALL_LOG_PATH $PKG_VERSION
fi

#####################################################################################################
#  ADD FUNCTION WHICH WILL BE HOOKED HERE

####################################################################
#   EXAMPLE FUNCTION
function runTest(){
    logMsg info 'This is a test'
}
#    EXAMPLE FUNCTION END
###################################################################

function installOfficialSamba(){
    smbPkgs2bUninst="samba"
    for smbPkg in $smbPkgs2bUninst
    do
        result=`rpm -qa | grep ^samba.[0-9].[0-9].[0-9]`
        if [ $? = 0 ];then
            logMsg info "$smbPkg was installed, will removed it." "$smbPkg"
            zypper -n remove $smbPkg
        else
            logMsg info "$smbPkg didn't install, no need to remove it" "$smbPkg"
        fi
    done
    rpm -ivh $SMB_RPMS/"OfficialSmbPkg/samba-3.6.3-0.18.3.x86_64.rpm"

}

function dbConver(){
    DB_LIBRARY="$LTFS_INSTALL_UDB/library.db"
    DB_SYSCON="$LTFS_INSTALL_UDB/sysconf.db"
    DB_EVENT="$LTFS_INSTALL_UDB/event.db"
    DB_${PRODUCT_NAME}_TABLE="$DB_LIBRARY:Cartridges:DualCopy:VARCHAR(10) $DB_LIBRARY:Cartridges:TapeUUID:VARCHAR(36) $DB_LIBRARY:Cartridges:LastMountTime:NUMERIC $DB_SYSCON:Shares:SHARE_DUAL_COPY:BOOL"
    for dnt in $DB_${PRODUCT_NAME}_TABLE
    do
        db=`echo $dnt | awk -F ":" '{print$1}'`
        table=`echo $dnt | awk -F ":" '{print$2}'`
        field=`echo $dnt | awk -F ":" '{print$3}'`
        valType=`echo $dnt | awk -F ":" '{print$4}'`
        if [ -e $db ];then
            chkFiledExist=`sqlite3 ${db} ".schema ${table}" | grep "${field}"`
            if [ $? -ne 0 ];then
                logMsg info "${db}: Field ${field} doesn't exists in table ${table}, adding..."
                addNewField=`sqlite3 $db "ALTER TABLE ${table} ADD COLUMN ${field} ${valType}"`
		verTableExist=`sqlite3 $db ".schema Version" | grep MinorVersion`
		ret=$?
		if [ $ret -eq 1 ];then
		    logMsg info "Version table not exists in $db, adding table..." "$db"
		    addNewTable=`sqlite3 $db "CREATE TABLE "Version" ("MajorVersion" INTEGER PRIMARY KEY, "MinorVersion" INTEGER DEFAULT 0, "Comment" VARCHAR(128) DEFAULT NULL);"`
		    addDBInitVersion=`sqlite3 $db "INSERT INTO "Version" VALUES(1,0,'Original');"`
		fi
                if [ $db == $DB_SYSCON ];then
                    sqlite3 $db "UPDATE ${table} SET ${field}='0'"
                fi
		if [ $db == $DB_LIBRARY ];then
		    if [ ${field} == DualCopy ];then
			sqlite3 $db "UPDATE ${table} SET ${field}=''"
		    elif [ ${field} == TapeUUID ];then
			sqlite3 $db "UPDATE ${table} SET TapeUUID=''"
		    elif [ ${field} == LastMountTime ];then
			sqlite3 $db "UPDATE ${table} SET LastMountTime=0"
		    fi
		fi
            fi
        else
            logMsg fail "DB ${db} doesn't exist!"
        fi
    done 
    if [ -e $DB_EVENT ];then
	addNewTable=`sqlite3 $DB_EVENT "CREATE TABLE "Version" ("MajorVersion" INTEGER PRIMARY KEY, "MinorVersion" INTEGER DEFAULT 0, "Comment" VARCHAR(128) DEFAULT NULL);"`
	addDBInitVersion=`sqlite3 $DB_EVENT "INSERT INTO "Version" VALUES(1,0,'Original');"`
    fi 
}


######################################################################################################
function execFncIf(){
    # Use this function to call other function(s) if the given condition is met
    # USage: execFncIf FunctionName "condition"
    # An Example: execFncIf runTest "$PKG_BUILDNUM > 58"
    if [ $2 ];then
	$1
    else
	logMsg logonly "Given condition $2 is not met, will not run." "$2"
    fi
}


function execFncOnPkgVersion(){
    # use this function to call another fucntion on specified package version.
    # USAGE: execFncOnVersion functionName fullVersion
    # An Example: execFncOnVersion runTest "1.3e.58"
    if [ $PKG_VERSION == $2 ];then
	$1
    else
	logMsg logonly "Given condition $2 not met, will not run" "$2"
    fi
}

######################################################################################################

#CALL HOOD BELOW
#execFncOnVersion runTest "1.3e.58"
#execFncOnVersion installOfficialSamba '1.3e.59'
#execFncIf dbConver "$INST_MAINVER.$INST_SUBVERSION < 2.5"
if [ 2 == $INST_MAINVER ];then
    if [ 4 == $INST_SUBVERSION ];then
        if (("10" <= "$INST_BUILDNUM")) ;then
            if (( "$INST_BUILDNUM" <= "25")) ;then
                dbConver   
            fi
        fi
    fi
fi
#cp ${LTFS_SOURCE_CFG}/log4cplus.cfg $LTFS_INSTALL_CFG/log4cplus.cfg
#cp $LTFS_SOURCE_SCRIPTS/LTFS_Notification/conf/syslog-ng.conf /etc/syslog-ng/syslog-ng.conf
