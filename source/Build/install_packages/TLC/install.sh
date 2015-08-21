#!/bin/bash 
RED="\e[0;31m"
NOR="\e[0;0m"
GREEN="\e[0;32m"
YELLOW="\e[0;33m"

PKG_MAIN_VER="0"
PKG_SUB_VER="9"
PKG_BUILD_NUM="10"
pkgVer="${PKG_MAIN_VER}.${PKG_SUB_VER}.${PKG_BUILD_NUM}"
CURRENT_PATH=`pwd`
LTFS_SOURCE="${CURRENT_PATH}/VS"
LTFS_SOURCE_UDB="${LTFS_SOURCE}/db"
LTFS_SOURCE_CFG="${LTFS_SOURCE}/Config"
LTFS_SOURCE_SCRIPTS="${LTFS_SOURCE}/Scripts"
LTFS_SOURCE_BIN="${LTFS_SOURCE}/bin"

DISKCACHEPATH="/opt/VS/vsCache/diskCache"
METACACHEPATH="/opt/VS/vsCache/meta"
LTFS_INSTALL_ROOT="/usr/VS"
LTFS_INSTALL_CFG="/etc/vs"
LTFS_INSTALL_SCRIPTS="${LTFS_INSTALL_ROOT}/scripts"
LTFS_INSTALL_UDB="${METACACHEPATH}/db"
LTFS_TMP_FOLDER="${LTFS_INSTALL_ROOT}/tmp"
VFS_ROOT="${LTFS_INSTALL_ROOT}/vfs"
BIN_ROOT="${LTFS_INSTALL_ROOT}/bin"
LINTAPE_RPMS_ON_INSTROOT="${LTFS_INSTALL_ROOT}/lintape"
LINTAPE_RPM_SRC="lin_tape-2.9.6-1.src.rpm"
REBUILT_LINTAPE_RPM_NAME="lin_tape-2.9.6-1.x86_64.rpm"
LINTAPED_RPM="lin_taped-2.9.6-rhel6.x86_64.rpm"
VFS_TARGET_PERMISSION_CTRL_FOLDER="/srv/"
VFS_TARGET_FOLDER="/srv/node/"
VFS_BIN_NAME="vfsserver"
VFS_CLN_BIN_NAME="vfsclient"
LTFS_TAPE_MOUNT_POINT="/opt/VS/vsMounts"
PYTHON_LIB_HOME="/usr/local/lib64/python2.6/site-packages"

THIRD_PARTY="${CURRENT_PATH}/3rd_Party"
LIB="${THIRD_PARTY}/lib"
RPMS="${THIRD_PARTY}/RPMs"
ZYPPER_REPO_TEMP="${RPMS}/repo"
HPLTFS_RPM="${RPMS}/rpm4HpLtfs"
IBM_LINTAPE_RPM="${RPMS}/rpm4IbmSe/lin_tape"
IBM_SE_RPM="${RPMS}/rpm4IbmSe/SE"
TARPKG_ROOT="${THIRD_PARTY}/Tars"



LOG_ROOT="${CURRENT_PATH}/log"
TOTAL_INSTALL_LOG_PATH="${LOG_ROOT}/TOTAL_INSTALL"
if [ -e ${LTFS_SOURCE_SCRIPTS}/ComFnc ];then
    . ${LTFS_SOURCE_SCRIPTS}/ComFnc "${TOTAL_INSTALL_LOG_PATH}" "${pkgVer}"
else
    echo "${RED}Common functions script doesn't exist!!!"
    exit 1
fi
LOG4CPLUS_CFG_SRC="log4cplus.cfg"
LTFS_CFG_SRC="vs_conf.xml"
DISKSPACE_MONITOR_CFG_SRC="diskSpaceMonitor.cfg"
LOGROTATE_CFG_SRC="vs"
LOGROTATE_CFG_DEST="/etc/logrotate.d"
LTFS_DEFAULT_VENDOR='IBM'
CRONFILE="/etc/crontab"
if [ $? -eq 0 ];then
    IS_SINGLEBOX="YES"
    ADMIN_USER_NAME="admin"
else
    IS_SINGLEBOX="NO"
    ADMIN_USER_NAME="vsadmin" 
fi
ADMIN_GROUP_NAME="vsadmin"
USRES_GROUP_NAME="vsusers"
LTFSGROUPS="$ADMIN_GROUP_NAME $USRES_GROUP_NAME"

. ./ltfs_install.sh 

function printLine()
{
    lineWide=`stty size | awk '{print$2}'`
    for ((i=1; i<=$lineWide;i++))
    do
            echo -en "${YELLOW}#"
    done
    echo -en "\n"
}

function welcomeInfo()
{
    winWide=`stty size | awk '{print$2}'`
    ltfsHeadWide=`expr $[($winWide - 117)/2]`
    welHeadWide=`expr $[($winWide - 38)/2]`
    inHeadWide=`expr $[($winWide - 40)/2]`
    upHeadWide=`expr $[($winWide - 42)/2]`
    uninHeadWide=`expr $[($winWide - 36)/2]`
    if [ $winWide -lt 117 ];then
        resize -c -s 50 137 > /dev/null
        ltfsHeadWide=10
	welHeadWide=49
	inHeadWide=48
	upHeadWide=47
	uninHeadWide=50
    fi

    for ((i=0;i<=$ltfsHeadWide;i++))
    do
        lWide="${lWide} "
    done

    for ((i=0;i<=$welHeadWide;i++))
    do
        welWide="${welWide} "
    done


    welcomeMsg="${welWide}WELCOME TO ValueStor INSTALLATION WIZARD\n"
    ValueStor="
${lWide}VVVVVV                VVVVV     SSSSSSSSSSSSSSSSSS
${lWide} V   V               V   V      S                S
${lWide}  V   V             V   V       S    SSSSSSSSSSSSS         ttttt
${lWide}   V   V           V   V        S    S                     t   t                       rrrr  rrrr
${lWide}    V   V         V   V         S    SSSSSSSSSSSSS  tttttttt   tttttttt    oooooooo    r  rrr  r
${lWide}     V   V       V   V          S                S  t                 t   o        o   r  rr  r
${lWide}      V   V     V   V           SSSSSSSSSSSSS    S  tttttttt   tttttttt  o  oooooo  o  r  rrr
${lWide}       V   V   V   V                        S    S         t   t        o   o    o   o r  rr
${lWide}        V   VVV   V             SSSSSSSSSSSSS    S         t   ttt       o  oooooo  o  r  r
${lWide}         V   V   V              S                S         t     t        o        o   r  r
${lWide}          VVVVVVV               SSSSSSSSSSSSSSSSSS         ttttttt         oooooooo    rrrr
"
    printLine
    echo -e "${GREEN}${ValueStor}"
    printLine
    echo -e "${GREEN}${welcomeMsg}"


    if [ "$operation" = "install" ];then
	for ((i=0;i<=$inHeadWide;i++))
	do
       	    inWide="${inWide} "
	done
	inMsg="${GREEN}${inWide}You are about to ${YELLOW}'fresh install'${GREEN} VStor${NOR}\n"
	echo -e "$inMsg" 
	printLine
    elif [ "$operation" = "upgrade" ];then
	for ((i=0;i<=$upHeadWide;i++))
        do
            upWide="${upWide} "
        done
	upMsg="$GREEN${upWide}You are about to $YELLOW'upgrade install'$GREEN VStor$NOR\n"
	echo -e "$upMsg"
	printLine
    elif [ "$operation" = "uninstall" ];then
        for ((i=0;i<=$uninHeadWide;i++))
        do
            uninWide="${uninWide} "
        done
	uninMsg="${GREEN}${uninWide}You are about to ${YELLOW}'UNINSTALL'${GREEN} VStor${NOR}\n"
	echo -e "$uninMsg"
	printLine
    fi
}

function exitOnfailed(){
    if [ $? != 0 ];then
	if [ $1 ];then
	    logMsg fail "$1"
	else
	    logMsg fail "Operation failed!"
	fi
    fi
}


function installIbmLtfsSE_OLD()
{
    RPM_PKG_IBM_LTFS_SE=""
    cd $IBM_SE_RPM
    if [ -e $RPM_PKG_IBM_LTFS_SE ];then
        logMsg "Installing IBM ltfs SE" "IBM ltfs SE"
        cmdInst="rpm -Uvh $RPM_PKG_IBM_LTFS_SE"
        instResult=`$cmdInst 2>&1`
        if [ $? -ne 0 ];then
            i=`echo $instResult | grep 'already installed'`
            if [ $? = 0 ];then
                logMsg ignore 'ltfs was installed, no need to install again' 'ltfs'
            else
                logMsg failure "$instResult"
            fi
        else
            logMsg success 'ltfs has been installed.' 'ltfs'
        fi
    else
        logMsg failure "rpm package: $RPM_PKG_IBM_LTFS_SE doesn't exist." "$RPM_PKG_IBM_LTFS_SE"
    fi
}

function installIbmLtfsSE()
{
    cd $TARPKG_ROOT
    if [ -e IBMLTFS_SE_CENTOS7.tar.gz ]; then
        logMsg info "IBMLTFS SE Binraries exists." "IBMLTFS SE"
        tar xvf IBMLTFS_SE_CENTOS7.tar.gz -C /
    else
        logMsg failure "IBMLTFS_BINARIES_SLES11 doesn't exist" "IBMLTFS_SE_BINARIES_CENTOS7"
    fi
    if [ -e /etc/ltfs.conf ];then
	logMsg info "Creating symbol link from /etc/ltfs.conf to /usr/local/etc/ltfs.conf"
	rm -rf /usr/local/etc/ltfs.conf
	ln -s /etc/ltfs.conf /usr/local/etc/ltfs.conf
    fi
}


function uninstallIbmLtfsSE()
{
    logMsg info "Removing IBM LTFS SE..."
    rm -rf /usr/local/bin/*ltfs* /usr/local/lib64/libltfs* /usr/local/lib64/ltfs/* /etc/ltfs.conf* /usr/local/etc/ltfs.conf /opt/IBM/ltfs /usr/local/share/doc/ltfssde*
    if [ $? == 0 ];then
        logMsg success "IBM LTFS SE has been uninstalled."
    else
        logMsg fail "Failed to remove IBM LTFS SE."
    fi
}

function uninstallIbmLtfsSE_OLD()
{
    RPMSUNINST="ltfs lin_taped lin_tape"
    logMsg info "Uninstalling RPMs."
    for pack in $RPMSUNINST
    do
            logMsg info "Uninstalling RPM: $pack." "$pack"
        cmdUninstStr="rpm -e $pack"
        result=`$cmdUninstStr 2>&1`
        if [ $? -ne 0 ];then
        rt=`echo $result | grep 'is not installed'`
        if [ $? = 0 ]; then
            logMsg info "$pack is not installed, no need to uninstall." "$pack"
            continue
        fi
        logMsg failure "$pack uninstall failed." "$pack"
        exit 1
        else
        logMsg success "$pack has been uninstalled." "$pack"
        fi
    done
}

function buildLinTape()
{
    if [ -e $LINTAPE_RPM_SRC ];then    
        logMsg info "Rebuilding lin_tape" "lin_tape"
        rpm -q gcc
        if [ $? -ne 0 ];then
           yum install -y gcc
        fi
        rpm -q glibc-devel
        if [ $? -ne 0 ];then
           yum install -y glibc-devel
        fi
        rpm -q kernel-devel
        if [ $? -ne 0 ];then
           yum install -y kernel-devel
        fi
        rpm -q rpm-build
        if [ $? -ne 0 ];then
           yum install -y rpm-build
        fi
        rpmbuild --rebuild $LINTAPE_RPM_SRC > $LOG_ROOT/lin_tape_rebuild.log
        if [ $? -ne 0 ];then
            logMsg failure "Rebuild lin_tape failure!!!" "lin_tape"
        else
            logMsg success "Rebuild lin_tape success." "lin_tape"
            if [ -e /root/rpmbuild/RPMS/x86_64/$REBUILT_LINTAPE_RPM_NAME ];then
                logMsg info "Installing rebuilt lin_tape" "lin_tape"
                cmdInstLinTape="rpm -Uvh /root/rpmbuild/RPMS/x86_64/$REBUILT_LINTAPE_RPM_NAME"
                instLinTapeResult=`$cmdInstLinTape 2>&1`
                  if [ $? -ne 0 ];then
                    i=`echo $instLinTapeResult | grep 'already installed'`
                    if [ $? = 0 ];then
                        logMsg ignore "lin_tape was installed, no need to install again" "lin_tape"
                    else
                        logMsg failure "$instLinTapeResult"
                    fi
                else
                    logMsg success "lin_tape has been installed." "lin_tape"
                fi
            fi
        fi  
    else
          logMsg failure "$LINTAPE_RPM_SRC doesn't exist" "$LINTAPE_RPM_SRC"
    fi
}

function installLinTape()
{
    cd ${IBM_LINTAPE_RPM}
    if [ ! -e "${LINTAPE_RPMS_ON_INSTROOT}" ];then
    	mkdir $LINTAPE_RPMS_ON_INSTROOT -p
    fi
    cp ${LINTAPE_RPM_SRC} ${LINTAPE_RPMS_ON_INSTROOT} -arf
    cp ${LINTAPED_RPM} ${LINTAPE_RPMS_ON_INSTROOT} -arf
    echo `uname -r` > ${LINTAPE_RPMS_ON_INSTROOT}/lintapeOnKernel
    if [ -e $REBUILT_LINTAPE_RPM_NAME ];then
        logMsg info "Installing rebuilt lin_tape" "lin_tape"
        cmdInstLinTape="rpm -Uvh $REBUILT_LINTAPE_RPM_NAME"
        instLinTapeResult=`$cmdInstLinTape 2>&1`
        if [ $? -ne 0 ];then
            i=`echo $instLinTapeResult | grep 'already installed'`
            if [ $? -eq 0 ];then
                logMsg ignore "lin_tape was installed, no need to install again" "lin_tape"
            else
    		    rpm -e lin_tape
    	    	#exitOnfailed "Unable to uninstall lin_tape."
      		    buildLinTape 2>$LOG_ROOT/err.log
            fi
        else
	        modprobe lin_tape
      	    RETVAL=$?
            if [ $RETVAL -eq 0 ];then
    	    	logMsg success "lin_tape has been installed." "lin_tape"
    	    else
	        	logMsg warn "Failed to modprobe lin_tape, will try to rebuild the lin_tape source."
		        logMsg info "Removing the previous installed lin_tape"
    		    rpm -e lin_tape
    	    	#exitOnfailed "Unable to uninstall lin_tape."
      		    buildLinTape 2>$LOG_ROOT/err.log
	        fi
        fi
    fi

    logMsg info "Installing lin_taped" "lin_taped"
    if [ -e $LINTAPED_RPM ];then
        cmdInstLinTaped="rpm -Uvh $LINTAPED_RPM"
        instLinTapedResult=`$cmdInstLinTaped 2>&1`
        if [ $? -ne 0 ];then
            i=`echo $instLinTapedResult | grep 'already installed'`
            if [ $? = 0 ];then
                logMsg ignore 'lin_taped was installed, no need to install again' 'lin_taped'
            else
                echo "$instLinTapedResult"
                logMsg failure "$instLinTapedResult"
            fi
        else
            logMsg success "lin_taped has been installed" "lin_taped"
        fi
    else
        logMsg failure "rpm package: lin_taped doesn't exist" "lin_taped"
    fi
}

function disableUnsupportModule()
{
    unSupModCfg="/etc/modprobe.d/unsupported-modules"
    unSupModCfg_bak="$unSupModCfg.org"
    if [ -e $unSupModCfg ];then
        line=`cat $unSupModCfg  | grep -n "allow_unsupported_modules" | grep -v "#" | awk -F : '{print$1}'`
        value=`cat $unSupModCfg  | grep -n "allow_unsupported_modules" | grep -v "#" | awk -F : '{print$2}' | awk -F " " '{print$2}'`
        if [ $value = 0 ];then
            mv $unSupModCfg $unSupModCfg_bak
            sed "$line c \allow_unsupported_modules 1" $unSupModCfg_bak >> $unSupModCfg
            rm $unSupModCfg_bak
        fi
    fi
}

function installPyDevPkgs()
{
    logMsg info "Installing the develop python packages."
    pyDevPkgs="setuptools-0.6c11 htj-suds-htj-4f5326e"
    if [ "$operation" = "install" ];then
        for pyPkg in $pyDevPkgs
        do
            cd $TARPKG_ROOT
            if [ -e $pyPkg.tar.gz ]; then
                logMsg info "Package $pyPkg exists" $pyPkg
                tar xvf $pyPkg.tar.gz > $LOG_ROOT/Install_$pyPkg.log
                cd $pyPkg
                if [ -e setup.py ]; then
		    logMsg info "Install script found, start to install package $pyPkg." "$pyPkg"
	            python setup.py install > $LOG_ROOT/Install_$pyPkg.log
	        else
	            logMsg failure "The install script 'setup.py' can't be found, please check if the package $pyPkg is complete." "$pyPkg"
	        fi
	        if [ $? != 0 ]; then
	            logMsg failure "Failed to install $pyPkg." "$pyPkg"
	        else
	            logMsg success "Package $pyPkg has been installed." "$pyPkg"
	            cd .. && rm $pyPkg -rf
          	fi
            else
	        logMsg failure "$pyPkg.tar.gz doesn't exist." "$pyPkg.tar.gz"
            fi
	done
    fi
}

function zyppInst()
{
    yum install -y net-snmp-agent-libs lsscsi 
    if [ -e '/usr/bin/mysql' ];then
        echo "MySql already installed."
    else
        yum install -y mariadb-server mariadb mariadb-libs
    fi
    rpm -q fuse-libs
    if [ $? -ne 0 ];then
        yum install -y fuse-libs
    fi
    rpm -q xmlrpc-c-client++
    if [ $? -ne 0 ];then
        yum install -y xmlrpc-c-client++
    fi
    rpm -q boost
    if [ $? -ne 0 ];then
        yum install -y boost
    fi
}

function cpLib2InstDir()
{
    logMsg info "Copying lib." "lib"
    if [ -e $LIB ]; then
        logMsg info "Lib folder exists"
	mkdirWithPerm $LTFS_INSTALL_ROOT 705
        \cp $LIB $LTFS_INSTALL_ROOT/lib -arf
	exitOnfailed "Failed to copied lib, please check if the install package is complete."
        logMsg success "Lib copied"
    else
        logMsg failure "Lib folder doesn't exist."
    fi
}

function mkdirWithPerm(){
    # $1 : Folder path to be created.
    # $2 : Permission to be given for the created folder.
    if [ -d $1 ];then
	logMsg info "Folder $1 already exists, updating permssion" "$1"
    else
	logMsg info "Folder $1 doesn't exist, creating it and set the permission"
	mkdir $1 -p
	exitOnfailed "Failed to create folder '$1'."
	logMsg suc "Folder $1 has been created" "$1"
    fi
	chmod $2 $1
	exitOnfailed "Faild to set permission"
	logMsg suc "Set permission for folder $1 done!" "$1"
}

function makeDirStrc(){

    folders="$VFS_TARGET_PERMISSION_CTRL_FOLDER:711:root:root
	     $VFS_TARGET_FOLDER:755:root:root
	     $VFS_ROOT:700:root:root
	     $BIN_ROOT:700:root:root
	     $DISKCACHEPATH:711:root:root
	     $METACACHEPATH:711:root:root
	     $LTFS_TAPE_MOUNT_POINT:750:root:root"
    for f in $folders
    do
        path=`echo $f | awk -F ":" '{print$1}'`
        perm=`echo $f | awk -F ":" '{print$2}'`
	owner=`echo $f | awk -F ":" '{print$3}'`
	owngroup=`echo $f | awk -F ":" '{print$4}'`
        mkdirWithPerm $path $perm
	chown $owner:$owngroup $path
    done
}

function installVfs()
{
    BINS4VFS="$VFS_BIN_NAME $VFS_CLN_BIN_NAME"
    logMsg info "Installing VFS" "VFS"
    cpLib2InstDir 

    cd $LTFS_SOURCE_BIN
    BinNotExist=0
    for vfsBin in $BINS4VFS
    do
        logMsg info "Check if $vfsBin exists."
        if [ ! -e $vfsBin ];then
    	    logMsg info "$vfsBin doesn't exist."
	    BinNotExist=1
	else
	    logMsg info "$vfsBin exists."
	fi
    done

    if [ ! $BinNotExist == 1 ]; then
        if [ "$operation" != "upgrade" ]; then
	    logMsg info "Creating VStor directories structure."
       	    makeDirStrc 
	elif [ "$operation" == "upgrade" ];then
            if [ -e $LTFS_INSTALL_ROOT/lib ];then
                logMsg info "VFS required libaries exists." "VFS"
		logMsg info "Updating to the latest lib"
                if [ -e $LTFS_INSTALL_ROOT/lib.org ];then
                    rm -rf $LTFS_INSTALL_ROOT/lib.org
                fi			
		mv $LTFS_INSTALL_ROOT/lib $LTFS_INSTALL_ROOT/lib.org
		\cp $LIB $LTFS_INSTALL_ROOT/lib -arf
		rm -rf $LTFS_INSTALL_ROOT/lib.org
            else
                if [ -e $LIB ]; then
		    logMsg info "Lib folder exists."
                    \cp $LIB $LTFS_INSTALL_ROOT/lib -arf
                    if [ $? = 0 ];then
                        logMsg success "Lib copied."
                    else
                        logMsg failure "Failed to copied lib,Please check if the install package is complete."
                    fi
                else
                    logMsg failure "Lib folder doesn't exist."
                fi
            fi
	    makeDirStrc
        fi
            logMsg info "Copying VFS needed binraries to $VFS_ROOT" "VFS"
	    for BIN in $BINS4VFS
	    do
	        logMsg info "Copying binrary: $BIN to $VFS_ROOT" "$BIN"
                \cp $BIN $VFS_ROOT/$BIN -arf
		chmod 700 $BIN $VFS_ROOT/$BIN
	    done	
    else
        logMsg fail "At lease one of the VFS Binraries doesn't exist, please verify if the package is complete."
    fi

}

function cpBins2Dest(){
    logMsg info "Copying binraries to $BIN_ROOT" "$BIN_ROOT"
    BINS="lfs_tool library_tool"
    for bin in $BINS
    do
        if [ $LTFS_SOURCE_BIN ];then
            if [ ! -d $BIN_ROOT ];then
                mkdirWithPerm $path 700 
                chown root:root $BIN_ROOT
            fi
            logMsg info "Copying samba plugin library..." "samba"
            \cp $LTFS_SOURCE_BIN/$bin $BIN_ROOT/ -rf
            if [ $? == 0 ];then
                logMsg success "Copy binrary $bin done!" "$bin"
            else
                logMsg fail "Failed to copy binrary $bin." "$bin"
            fi
        else
            logMsg fail "Binraries can not be found in the package, please verify if the package is complete."
        fi
    done
}

function chkXfs()
{
    if [ "$operation" = "install" ];then
        xfsVols=`mount | grep 'xfs' | awk '{print$3}'`
        foundDiskCacheXfsVol=1
        foundMetaCacheXfsvol=1
        if [ "$xfsVols" ];then
            for xfsVol in $xfsVols
            do
              if [ $xfsVol == $DISKCACHEPATH ];then
                  foundDiskCacheXfsVol=0
              elif [ $xfsVol == $METACACHEPATH ];then
                  foundMetaCacheXfsvol=0
              fi
            done
            if [ $foundDiskCacheXfsVol = "0" ];then
                logMsg success "Found an XFS volume was mounted at $DISKCACHEPATH, will use this directory as the diskCache" "$xfsVols"
            else
                xfsQues $DISKCACHEPATH "Disk"
            fi
            if [ $foundMetaCacheXfsvol = "0" ];then
                logMsg success "Found an XFS volume was mounted at $METACACHEPATH, will use this directory as the metaCache" "$xfsVols"
            else
                xfsQues $METACACHEPATH "Meta"
            fi
        else
            logMsg info "There is no xfs volume."
            xfsQues $DISKCACHEPATH "Disk"
            xfsQues $METACACHEPATH "Meta"
        fi
    fi
}

function xfsQues()
{
    logMsg ques "Seems there is no XFS volume mounted to ${1}, if you confirm, installer will create this folder to be used as the VSTor ${2}Cache place." "$1"
    echo -ne "${YELLOW}Yes/No${NOR}(Input ${YELLOW}'Yes'${NOR} to coninue or Input ${YELLOW}'No'${NOR} to CANCEL the Installation): "
    loopFlag=true
    while $loopFlag
    do
        read YesNo
        case "$YesNo" in
             y* | Y* | "")
                loopFlag=false
                if [ -d $1 ];then
                   logMsg info "$1 folder exists, no need to recreate it." "$1"
                else
                   logMsg info "$1 doesn't exist, will create it." "$1"
                   mkdir $1 -p
                   if [ $? = 0 ];then
                      logMsg success "$1 has been created, it will be mounted as the VStor Cache." "$1"
                   else
                      logMsg fail "Failed to create $1." "$1"
                   fi
                fi
             ;;
             n* | N* | c | C)
                logMsg info "You have Canceled the installation, you can manually attached/mount a disk/volume and format it as ${YELLOW}'xfs'${NOR} on $1 then restart the installation of the VStor" "$1"
                 exit 0
            ;;
            *)
                logMsg warn "Unknown option ${YesNo}" "${YesNo}"
                echo -ne "${YELLOW}Yes/No${NOR}(Input ${YELLOW}'Yes'${NOR} to coninue or Input ${YELLOW}'No'${NOR} to CANCEL the Installation): "    
            ;;
        esac
    done
}

function createMyqlDBandUser(){
    mysqlSrcState=`systemctl status mariadb.service`
    if [ $? -eq 3 ];then
        systemctl restart mariadb.service
    fi
    rootHasPwd=0
    while [ $rootHasPwd -eq 0 ]
    do
    need_password=`mysql -u root test -e "show databases;" 2>/dev/null`
    if [ $? -ne 0 ];then
        logMsg info "Your MySQL database user root need a password to login, please input your password: " "root" "n"
        read -s  mysqlpwd
        ret=`mysql -u root -p${mysqlpwd} mysql -e "show tables;" 2>/dev/null`
        ret=$?
        if [ $ret -ne 0 ];then
            echo && logMsg info "Input password incorrect... please enter again."
            continue
        else
            echo && logMsg info "Input password correct."
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
    is_vsadmin_exist=`mysql -u root ${cmdMysql} mysql -e "select User from user where User='vsadmin'" | grep -v User`
    if [ ! $is_vsadmin_exist ];then
        TO_CREATE_VSADMIN="CREATE USER 'vsadmin'@'localhost' IDENTIFIED BY 'hello123';"
    else
        TO_CREATE_VSADMIN=""
    fi
    logMsg info "Creating DataBases schema."
    mysql -u root ${cmdMysql}<<EOF
CREATE DATABASE Media CHARACTER SET utf8;
CREATE DATABASE CatalogDb CHARACTER SET utf8;
GRANT ALL PRIVILEGES ON Media.* TO 'vsadmin'@'localhost';
GRANT ALL PRIVILEGES ON CatalogDb.* TO 'vsadmin'@'localhost';
EOF
}

function addDefaultAdmin()
{
	createMyqlDBandUser
}


function cpScps2Dest()
{
	if [ -e "$LTFS_INSTALL_ROOT/scripts" ];then
        echo ""
	else
        mkdir -p  "$LTFS_INSTALL_ROOT/scripts"
	fi
	
	logMsg info "Copying lib4py26..."
	mkdir ${PYTHON_LIB_HOME}/vs -p
	mkdir ${PYTHON_LIB_HOME}/vs/conf -p
	if [ -e "$LTFS_SOURCE_SCRIPTS/VS_Notification/" ];then
	    cp "$LTFS_SOURCE_SCRIPTS/VS_Notification/conf/logging.conf" "${PYTHON_LIB_HOME}/vs/conf/" -arf
	else
	    logMsg failure "$LTFS_SOURCE_SCRIPTS/VS_Notification/ doesn't exist"
	fi

    if [ -e "$LTFS_SOURCE_SCRIPTS/VS_Notification/conf/rsyslog.conf" ];then
        logMsg info "Copying rsyslog configure file to /etc/rsyslog.conf" "rsyslog"
        mv /etc/rsyslog.conf /etc/rsyslog.conf.bk
        cp "$LTFS_SOURCE_SCRIPTS/VS_Notification/conf/rsyslog.conf" "/etc/rsyslog.conf" -rf
    else
        logMsg failure "$LTFS_SOURCE_SCRIPTS/VS_Notification/conf/rsyslog.conf doesn't exist" 
    fi 

    logMsg info "Copying scripts..." 
    scpts="watchdog"
    for scpt in $scpts
    do
	logMsg info "Copy $scpt to $LTFS_INSTALL_SCRIPTS"
	\cp "$LTFS_SOURCE_SCRIPTS/$scpt" "$LTFS_INSTALL_SCRIPTS/$scpt" -arf
    done

    LTFS_LOG_PATH="/var/log/vs"
    if [ -e "/var/log/vs" ];then
	logMsg info "The default log folder: $LTFS_LOG_PATH  exists, no needs to create it." "$LTFS_LOG_PATH"
	#chown root:www /var/log/vs
	chmod 770 $LTFS_LOG_PATH 
    else
	logMsg info "The default log folder: $LTFS_LOG_PATH doesn't exist, will create it." "$LTFS_LOG_PATH"
	mkdir "/var/log/vs"
	touch "/var/log/vs/event_handler.log" 
	#chown root:www /var/log/vs 
	chmod 770 $LTFS_LOG_PATH
    fi
    systemctl restart rsyslog

}

function chkInstVer()
{
    versionFile="/usr/VS/version"
    if [ -e $versionFile ];then
        logMsg info "Found version file"
        INST_MAINVER=`cat $versionFile | grep MAIN_VERSION | cut -f2 -d'='`
        INST_SUBVER=`cat $versionFile | grep SUB_VERSION | cut -f2 -d'='`
        INST_BUILDNUM=`cat $versionFile | grep BUILD_NUMBER | cut -f2 -d'='`
        INST_VERSION="$INST_MAINVER.$INST_SUBVER.$INST_BUILDNUM"
        logMsg info "Installed Version: $INST_VERSION" "$INST_VERSION"
        logMsg info "Package Version: $pkgVer" "$pkgVer"
        if [ $PKG_MAIN_VER -eq $INST_MAINVER ];then
            #logMsg info "Same Main Version, check sub version"
            if [ $PKG_SUB_VER == $INST_SUBVER ];then
                #logMsg info "Same Sub Version, Check build number"
                if [ $PKG_BUILD_NUM -eq $INST_BUILDNUM ];then
		    k=1
		    while [ $k == 1 ]
		    do
                        logMsg ques "Same version build was installed on this system, do you want to re-install it? (y/n):" "(y/n)" "n"
                        read yn
                        case "$yn" in
                            y | Y | YES | yes)
                                logMsg info "You've choose to re-install the VStor"
				k=0
                                operation='upgrade'
                            ;;
                            n | N | no | NO )
                                logMsg info "You've cancel the installation"
                                exit 0
                            ;;
	                    *)
			        logMsg info "Please only input y/n" "y/n"
                        esac
		    done
		elif [ $PKG_BUILD_NUM -lt $INST_BUILDNUM ];then
		    logMsg info "Package version is lower than the installed one, no need to install again."
		    exit 0
                else
	            askQues
                fi
            else
                askQues 
            fi
        else
	    askQues
        fi
    else
	if [ "$operation" == "upgrade" ];then
            logMsg info "Version file doesn't exist, VStor didn't installed, please don't use upgrade option to install the package."
            exit 1
	fi 
    fi
}

function askQues()
{
    if [ "$operation" == "install" ];then
        loop=1
        while [ $loop == 1 ]
        do
            logMsg info "Build number mismatch, need to be upgrade."
            logMsg ques "But you are choosing an fresh install option, do you want to install the package as upgrade? (y/n)" "(y/n)"
            read opt
            case "$opt" in
    	        y | Y | YES | yes)	
    	            logMsg info "You've choose to re-install the VStor"
                    loop=0
                    operation='upgrade'
	        ;;
    	        n | N | no | NO )
                    logMsg info "You've cancel the installation"
                    exit 0
                ;;
                *)
                    logMsg info "Please only input y/n" "y/n"
            esac
        done
    fi
}

function chkonSrv()
{
    if [ "$operation" = "install" ];then
        if [ -e $LTFS_SOURCE_SCRIPTS/vs ];then
            logMsg info "Copying startup script to /etc/init.d/." "/etc/init.d/"
            \cp $LTFS_SOURCE_SCRIPTS/vs /etc/init.d/vs -arf
	    \cp $LTFS_SOURCE_SCRIPTS/vs /etc/rc.d/init.d/vs -arf
            chmod 744 /etc/init.d/vs
	    chmod 744 /etc/rc.d/init.d/vs
            chkconfig --add vs
            chkconfig --level 35 vs on
        else
            logMsg fail "VStor service manage script doesn't exist" "VStor"
        fi
    elif [ "$operation" = "upgrade" ];then
        if [ -e /etc/init.d/vs ];then
	    diff $LTFS_SOURCE_SCRIPTS/vs /etc/init.d/vs > /dev/null
	    if [ $? -ne 0 ];then
		logMsg info "Update vs service script" "vs"
		\cp $LTFS_SOURCE_SCRIPTS/vs /etc/init.d/vs -ar
		\cp $LTFS_SOURCE_SCRIPTS/vs /etc/rc.d/init.d/vs -ar
	    else
		logMsg info "No update for vs service script" "vs"
	    fi
	else
	    logMsg warn "vs service script doesn't exist, will copy a new one"
	    \cp $LTFS_SOURCE_SCRIPTS/vs /etc/init.d/vs -ar
	    \cp $LTFS_SOURCE_SCRIPTS/vs /etc/rc.d/init.d/vs -ar
	fi 
    fi

    if [ -e $LTFS_SOURCE_SCRIPTS/chkIsDirty ];then
        logMsg info "Copying $LTFS_SOURCE_SCRIPTS/chkIsDirty to /etc/init.d"
        \cp $LTFS_SOURCE_SCRIPTS/chkIsDirty /etc/init.d/chkIsDirty -arf
        chmod 744 /etc/init.d/chkIsDirty
        if [ "$operation" = "install" ];then
            chkconfig --add chkIsDirty
	    if [ $? -eq 0 ];then
		/etc/init.d/chkIsDirty start
	    fi
        fi
    else
        logMsg faild "$LTFS_SOURCE_SCRIPTS/chkIsDirty doesn't exit"
    fi
}

function cpCfg2Dest()
{
    logMsg info "Copying configure file..."
    mkdirWithPerm $LTFS_INSTALL_CFG 750
    cfgFilesList="${LOG4CPLUS_CFG_SRC}:${LTFS_INSTALL_CFG} $LTFS_CFG_SRC:${LTFS_INSTALL_CFG}  $LOGROTATE_CFG_SRC:$LOGROTATE_CFG_DEST"
    for cf in $cfgFilesList
    do
	srcCfgPath=`echo $cf | awk -F ":" '{print$1}'`
	destCfgPath=`echo $cf | awk -F ":" '{print$2}'`
	
	if [ -e "${LTFS_SOURCE_CFG}/${srcCfgPath}" ];then
            logMsg info "Copying default configure file ${srcCfgPath} to ${destCfgPath}" 
            if [ ! -e "${destCfgPath}/${srcCfgPath}" ];then
                cp "${LTFS_SOURCE_CFG}/${srcCfgPath}" "${destCfgPath}/${srcCfgPath}" -rf
            else
                logMsg info "${destCfgPath}/${srcCfgPath} exists, no need to replace it."
            fi
        else
            logMsg failure "cpCfg2Dest(): ${LTFS_SOURCE_CFG}/${srcCfgPath} doesn't exist, please check if the package is complete."
        fi
    done 
}

function stopVfs(){

    logMsg info "Checking if the VFS was mounted." "VFS"
    rc=`ps -e | grep $VFS_BIN_NAME`
    if [ $? = 0 ]; then
        logMsg info "VFS service is running, stopping it." "VFS"
        logMsg info "Umounting shares"
        python "$LTFS_INSTALL_SCRIPTS/umount_share.pyc" -a
        if [ $? == 0 ];then
            logMsg info "Shares umounted."
        else
            logMsg fail "Failed to unmount shares."
        fi
        if [ -e $VFS_ROOT/vfs.pid ];then
            logMsg info "Found vfs.pid."
            PID=`cat $VFS_ROOT/vfs.pid`
        else
            logMsg info "PID file didn't found, check via ps command"
            PID=`ps -e | grep $$VFS_BIN_NAME | awk '{print $1}'`
        fi
        kill -n TERM $PID
        if [ $? -ne 0 ];then
            logMsg failure "Failed to umount VFS, please check." "VFS"
        else
            waitTillProcEnd
            logMsg success "VFS umounted." "VFS"
        fi
    else
        logMsg info "VFS didn't mount." "VFS"
    fi
}

function cpUdb2instRoot()
{
    MYSQL_DB_FILEPATH="/var/lib/mysql/"
    for g in $LTFSGROUPS
    do
        chkgrp=`cat /etc/group | grep $g | awk -F ":" '{print$1'} | grep $g`
        if [ $? == 0 ];then
            logMsg info "User group ${g} exists, no need to create again" "${g}"
        else
            logMsg info "User group doesn't exist, will creat it" "${g}"
            groupadd ${g}
        fi
    done

    if [ "$operation" = "install" ];then
        if [ ! -e ${METACACHEPATH} ];then
            mkdir ${METACACHEPATH} -p
        fi
    fi
}

function chkLibConnect(){
    SCSI_DEV=`lsscsi -g | grep mediumx`
    if [ $? = 0 ];then
        LIB_CNT_STATE='YES'
    else
        LIB_CNT_STATE='NO'
    fi
}

function callHook(){
    logMsg info "Calling hook script."
    if [ "$operation" = "upgrade" ];then

        if [ -e $LTFS_SOURCE_SCRIPTS/hook.sh ];then
            sh $LTFS_SOURCE_SCRIPTS/hook.sh $CURRENT_PATH
        fi
    fi
}

function setBackSyslogng(){
    mv /etc/rsyslog.conf.bk /etc/rsyslog.conf
    systemctl restart rsyslog
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

function chkAndUmountTape(){
    logMsg info "Checking if any tape mounting."
    mounttab="/etc/mtab"
    mntpots=`cat ${mounttab} | grep "^ltfs.*" | awk '{print$2}'`
    if [ ! -z "$mntpots" ];then
        for mp in ${mntpots}
        do
            logMsg info "Umounting ${mp}" "${mp}"
            fusermount -u ${mp}
            if [ $? -eq 0 ];then
                logMsg info "Umount ${mp} successes!" "${mp}"
            else
                logMsg warn "Unable to umount ${mp}, will do one more time umount."
                fusermount -u ${mp}
                if [ $? -eq 0 ];then
                    logMsg success "Umount ${mp} successes!" "${mp}"
                else
                    logMsg fail "Failed to umount ${mp}, please manually unmount the tape before continue the installation." "${mp}"
                fi
            fi
        done
    else
        logMsg info "No tape is mounted."
    fi
}


function setWatchDogCron(){
    logMsg info "Setting watchdog cron."
    isCronExist=`cat $CRONFILE | grep /usr/VS/scripts/watchdog`
    if [ $? -ne 0 ];then
         echo "#*/1 * * * * root sh /usr/VS/scripts/watchdog" >> ${CRONFILE}
    fi
}

function unsetWatchDogCron(){
    logMsg info "Removing watchdog cron."
    sed -i '\:[ \t]*/usr/VS/scripts/watchdog:d' ${CRONFILE}
}

function removeMysqlDB(){
    MySQL_DB="Media CatalogDb"
    for db in ${MySQL_DB}
    do
    logMsg info "Deleting msyql database ${db}" "${db}"
    #mysql -u vsadmin -phello123 -e "drop database ${db}"
    mysql -u vsadmin  -e "drop database ${db}"
    done
}

function autoLoadSgModule()
{
    if [ -e /etc/rc.modules ];then
    	grep "modprobe sg" /etc/rc.modules
	if [ $? -ne 0 ];then
            echo "modprobe sg" >> /etc/rc.modules
	    modprobe sg
    	fi
    else
	touch /etc/rc.modules
	chmod +x /etc/rc.modules
	echo "modprobe sg" >> /etc/rc.modules
	modprobe sg
    fi
}

function main()
{
    chkXfs 2>$LOG_ROOT/err.log
    disableUnsupportModule 2>$LOG_ROOT/err.log
    zyppInst 2>$LOG_ROOT/err.log
    ###installPyDevPkgs 2>$LOG_ROOT/err.log
    if [ $LTFS_DEFAULT_VENDOR = 'HP' ];then
        installHpLtfs 2>$LOG_ROOT/err.log
    elif [ $LTFS_DEFAULT_VENDOR = 'IBM' ];then
        installLinTape 2>$LOG_ROOT/err.log
        installIbmLtfsSE 2>$LOG_ROOT/err.log
    elif [ $LTFS_DEFAULT_VENDOR = 'IBMLE' ];then
        installLinTape 2>$LOG_ROOT/err.log
        installIbmLtfsLE 2>$LOG_ROOT/err.log
    fi
    #cpSmbPlugIn 2>$LOG_ROOT/err.log
    logMsg info "Creating VStor root directory."
    mkdirWithPerm $LTFS_INSTALL_ROOT 705
    mkdirWithPerm $LTFS_TMP_FOLDER 705
    cpUdb2instRoot 2>$LOG_ROOT/err.log

    if [ "$operation" = "install" ];then
        logMsg info "Installation operation, generating the required certificates."
        #gentCrts 2>$LOG_ROOT/err.log
        addDefaultAdmin
    elif [ "$operation" = "upgrade" ];then
        logMsg info "upgrade"
    fi

    logMsg info "Copying source to ${LTFS_INSTALL_ROOT}." "${LTFS_INSTALL_ROOT}"
    
    installVfs
    if [ $? -eq 0 ]; then
        logMsg success "VFS installation has been done!" "VFS"
    fi

    cpBins2Dest
    #if [ $LTFS_DEFAULT_VENDOR = 'HP' ];then
    #    installTapeVerifyToolHP 
    #elif [ $LTFS_DEFAULT_VENDOR = 'IBM' ];then
    #    installTapeVerifyToolIBM 
    #elif [ $LTFS_DEFAULT_VENDOR = 'IBMLE' ];then
    #    installTapeVerifyToolIBM 
    #fi
    cpCfg2Dest
    cpScps2Dest
    #setDiskSpaceMonitorCron
    #setSessionCronManager
    chkonSrv
    callHook

    setWatchDogCron
    cp $CURRENT_PATH/version $LTFS_INSTALL_ROOT/ -rf
}


case "$1" in
    install|INSTALL|i|"")
        operation="install"
	clear
	welcomeInfo
	chkInstVer
	chkAndUmountTape
    check_vendor
    autoLoadSgModule
    main
	if [ $? = 0 ];then
            logMsg success "${YELLOW}CONGRATULATIONS!!${GREEN} The Fresh Installation has been completely done! Installed version: ${pkgVer}" "${pkgVer}"
	    chkLibConnect
	    if [ $LIB_CNT_STATE == 'YES' ];then
	        logMsg info "Please check everything ready then start the VStor service with command: /etc/init.d/vs start" "/etc/init.d/vs start"
            elif [ $LIB_CNT_STATE == 'NO' ];then
	        logMsg info "It seems that you don't have a Tape Library connected, please connect a tape libary then start the VStor service with command: /etc/init.d/vs start" "/etc/init.d/vs start"	
	    fi
	fi   
    ;;
    upgrade|UPGRADE|up)
        operation="upgrade"
	clear
	welcomeInfo
	chkInstVer
	chk_LTFS_Vendor
    autoLoadSgModule

        logMsg info "Stopping services."
	if [ -s /etc/init.d/vs ];then
	    service vs stop
	    if [ $? != 0 ];then
	        logMsg fail "Stopping vs service failed, please check!"
	    fi
	else
	   logMsg fail "Service vs script doesn't exists." "vs"
	fi

        main
        if [ $? = 0 ];then
            logMsg success "${YELLOW}CONGRATULATIONS!!${GREEN} The Upgrade Installation has been completely done! Installed version: ${pkgVer}" "${pkgVer}"
	    chkLibConnect
            if [ $LIB_CNT_STATE == 'YES' ];then
                logMsg info "Please check everything ready then start the VStor service with command: /etc/init.d/vs start" "/etc/init.d/vs start"
            elif [ $LIB_CNT_STATE == 'NO' ];then
                logMsg info "It seems that you don't have a Tape Library connected, please connect a tape libary then start the VStor service with command: /etc/init.d/vs start" "/etc/init.d/vs start"
            fi
        fi
    ;;
    uninstall|UNINSTALL)
	clear
	operation="uninstall"
	chk_LTFS_Vendor
	welcomeInfo	
        logMsg ques "ARE YOU REALLY SURE TO UNINSTALL VStor?" "UNINSTALL"
	echo -ne "$YELLOW(Y/N)$NOR:"
        read ANSWER

        case $ANSWER in
            [Yy]*)
                logMsg info "You've confirmed to uninstall VStor, uninstallation started."
		if [ ! -e /usr/VS ];then
		    logMsg info "It seems that you didn't install VStor, uninstall is not needed."
		    exit 0
		fi 
       
	        logMsg info "Stopping VStor service: vs" "vs"
	        if [ -e /etc/init.d/vs ];then
                    /etc/init.d/vs stop
		    if [ $? != 0 ];then
			exit 1
		    fi
	        fi

		logMsg info "Stopping service chkIsDirty."
		if [ -s /etc/init.d/chkIsDirty ];then
		    /etc/init.d/chkIsDirty stop
	   	    exitOnfailed
		else
		    logMsg fail "/etc/init.d/chkIsDirty doesn't exit."
		fi	
		unsetWatchDogCron
		removeMysqlDB

                logMsg info "Removing the VS directories" "VS"
                sources="/usr/VS/ ${PYTHON_LIB_HOME}/vs /usr/lib64/samba/vfs/ltfstor.so /usr/lib64/samba/vfs/streams_ltfstor.so /var/log/vs/ $LOGROTATE_CFG_DEST/vs /var/lib/dav/"
                for s in $sources
                do
                    if [ -e $s ];then
                        rm -rf $s
                        if [ $? != 0 ];then
                            logMsg failure "Failed to remove $s, you may need to remove it manually." "$s"
                        else
                            logMsg success "$s has been removed." "$s"
                        fi
                    else
                        logMsg info "$s doesn't not exist." "$s"
                    fi
                done

		logMsg info "Uninstalling LTFS."
		if [ $LTFS_DEFAULT_VENDOR = 'IBM' ];then
            uninstallIbmLtfsSE
		elif [ $LTFS_DEFAULT_VENDOR = 'HP' ];then
		    uninstHpLtfs
		elif [ $LTFS_DEFAULT_VENDOR = 'IBMLE' ];then
            uninstallIbmLtfsLE
		fi 
 	
		setBackSyslogng

		logMsg info "Removing managing service: vs" "vs"
		chkconfig --del vs
		if [ -e "/etc/rc.d/rc5.d/S27vs" ];then 
    		    rm "/etc/rc.d/rc5.d/S27vs"
		elif [ -e "/etc/rc.d/rc3.d/S27vs" ];then
		    rm "/etc/rc.d/rc3.d/S27vs"
		fi 
		if [ -e "/etc/init.d/vs" ];then
		    rm "/etc/init.d/vs"
		fi
		if [ -e "/etc/rc.d/init.d/vs" ]; then
		    rm "/etc/rc.d/init.d/vs"
		fi

		logMsg info "Removing service chkIsDirty." "chkIsDirty" 
		chkconfig --del chkIsDirty
		if [ -e "/etc/init.d/chkIsDirty" ];then
		    rm "/etc/init.d/chkIsDirty"
		fi

		logMsg info "Removing Cache Folders." "Cache"
                cacheFolders="${DISKCACHEPATH}/*
			      ${METACACHEPATH}/*
                              ${LTFS_TAPE_MOUNT_POINT}
                              ${LTFS_INSTALL_CFG}
                              ${VFS_TARGET_FOLDER}/vsnode"
                              #${VFS_TARGET_PERMISSION_CTRL_FOLDER}"
                for cf in $cacheFolders
                do
                    if [ -e $cf ];then
                        logMsg info "Removing folder $cf" "$cf"
                        rm -rf $cf
                        if [ $? == 0 ];then
                            logMsg suc "Folder $cf has been removed." "$cf"
                        else
                            logMsg warn "Failed to remove folder $cf, please check and remove it manually."
                        fi
                    fi
                done

		if [ $? = 0 ];then
			logMsg success "VS has been uninstalled." "VS"
		fi
            ;;

            [Nn]*)
                logMsg info "You've canceled the unstallation."
                exit 0
	    ;;
	
	    *)
		logMsg info "Unknow Option, exiting."
		exit 0
	    ;;
        esac
    ;;

    "-v" | "version" |"VERSION")
        echo -e "VStor Build version: ${YELLOW}${pkgVer}${NOR}"
    ;;

    *)
        echo $"Usage: $0 {install | upgrade | uninstall | version}"
        exit 2
esac
exit $?

