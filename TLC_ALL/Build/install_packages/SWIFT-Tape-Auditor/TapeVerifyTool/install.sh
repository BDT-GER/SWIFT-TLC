#!/bin/bash
RED="\e[0;31m"
NOR="\e[0;0m"
GREEN="\e[0;32m"
YELLOW="\e[0;33m"
TOTAL_INSTALL_LOG_PATH=install-`date +%Y_%m_%d-%H:%M:%S`.log

function logMsg()
{
    #$1 Log Type: INFO | SUCCESS | FAILURE | LOGONLY
    #$2 Log Message
    #$3 keyword string to highlight
    #Usage: logMsg suc "Samba Service is running" "Samba"
    case $1 in
        INFO|info)
        msgHeadCol=$GREEN
        logType="[INFO]"
        msgBodyCol=$NOR
    ;;
        IGNORE|ignore)
        msgHeadCol=$GREEN
        logType="[IGNORE]"
        msgBodyCol=$NOR
    ;;
        SUCCESS|success|suc)
        msgHeadCol=$GREEN
        logType="[SUCCESS]"
        msgBodyCol=$GREEN
    ;;
        FAILURE|failure|fail)
        msgHeadCol=$RED
        logType="[FAILURE]"
        msgBodyCol=$RED
    ;;
        WARNING|warning|warn)
        msgHeadCol=$RED
        logType="[WARNING]"
        msgBodyCol=$NOR
    ;;
        QUESTION|question|ques)
        msgHeadCol=$CYAN
        logType="[QUESTION]"
        msgBodyCol=$NOR
    ;;
        LOGONLY|logonly|log)
        msgHeadCol=$GREEN
        logType="[LOGONLY]"
        msgBodyCol=$NOR
    esac
        msg=$2
        if [ "$3" ]; then
                msg=${2/$3/"$YELLOW'$3'$msgBodyCol"}
        fi
    if [ "$4" == "n" ];then
        echoOpt="-ne"
    else
        echoOpt="-e"
    fi
    if [ ! -z $TOTAL_INSTALL_LOG_PATH ];then
            if [ $logType = "[LOGONLY]" ];then
                echo `date` - $logType: $2 >> $TOTAL_INSTALL_LOG_PATH
            else
                echo "$echoOpt" "${msgHeadCol}${logType}${msgBodyCol} $msg $NOR" && echo `date` - $logType: $2 >> $TOTAL_INSTALL_LOG_PATH
            fi
    else
        echo "$echoOpt" "${msgHeadCol}${logType}${msgBodyCol} $msg $NOR"
    fi
        if [ $logType = "[FAILURE]" ];then
        if [ ! -z $pkgVer ];then
                echo -e "Package Version: ${YELLOW}${pkgVer}${NOR}"
        fi
                exit 1
        fi
}
    


BIN_ROOT="/usr/share/vs-tape-verify-tool"

case "$1" in 
    install|INSTALL|i|"")
        logMsg ques "${GREEN}Which platform of TapeVerify Tool to be installed?${NOR}"
        echo -ne "0): ${YELLOW} TapeVerify Tool for IBM platform ${NOR}
            \r1): ${YELLOW} TapeVerify Tool for HP platform${NOR}
            \rInput ${YELLOW}'C'${NOR} to cancel the installation.${YELLOW}(0/1/C)${NOR}: "
        exitFlag=false
        while [ $exitFlag != true ]
        do
            read platForm
            case $platForm in
                0 | IBM | ibm)
                exitFlag=true
                logMsg info "You are selecting to install TapeVerify Tool for IBM platform." "IBM TapeVerify Tool"
                VENDER="IBM"
                sleep 2
            ;;
                1 | HP | hp)
                exitFlag=true
                logMsg info "You are selecting to install TapeVerify Tool for HP platform." "HP TapeVerify Tool"
                VENDER="HP"
                sleep 2
            ;;
            c | C | cancel | CANCEL )
                logMsg warn "You've canceled the installation."
                exit 0
            ;;
                *)
                logMsg warn "Unknown input $platForm" "${platForm}"
                echo -ne "0): ${YELLOW} TapeVerify Tool for IBM platform ${NOR}
                    \r1): ${YELLOW} TapeVerify Tool for HP platform${NOR}
                    \rInput ${YELLOW}'C'${NOR} to cancel the installation.${YELLOW}(0/1/C)${NOR}: "
            ;;
            esac    
        done
        if [ ! -e $BIN_ROOT ];then
            mkdir -p $BIN_ROOT
        fi
        tar xvf ./TapeVerifyTool"${VENDER}".tar.gz -C $BIN_ROOT/
        if [ $? -ne 0 ];then
            logMsg warn "Failed to install TapeVerify Tool. Please check if ValueStor is installed before installing TapeVerify Tool."
            exit 1
        else
            logMsg info "TapeVerify Tool installed."
        fi 
    ;;
    uninstall|UNINSTALL)
        rm -rf ${BIN_ROOT} 
        logMsg info "TapeVerify Tool uninstalled." 
    ;;
    esac    

exit 0
