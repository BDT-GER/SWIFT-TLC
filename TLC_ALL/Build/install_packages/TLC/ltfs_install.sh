#!/bin/bash

function uninstHpLtfs()
{
    logMsg info "Removing HP LTFS SE..."
    rm -f /usr/local/bin/*ltfs* /usr/local/lib/libltfs* /usr/local/lib/ltfs/* /usr/local/etc/ltfs.conf* /usr/local/etc/vs_ltfs.conf*
    if [ $? == 0 ];then
    logMsg success "HP LTFS SE has been uninstalled."
    else
    logMsg fail "Failed to remove HP LTFS."
    fi
}

function chk_LTFS_Vendor(){
    if [ $operation != 'install' ];then
        ret=`ltfs -V 2>&1| grep "HP LTFS"`
        if [ $? = 0 ];then
            LTFS_DEFAULT_VENDOR='HP'
        setOpt "$CURRENT_PATH/version" "VENDOR" "HP"
        fi
    elif [ $operation = 'install' ];then
    logMsg ques "${GREEN}Which vendor of LTFS Edition to be installed?${NOR}"
    echo -ne "0): ${YELLOW} IBM LTFS SE Edition ${NOR}
        \r1): ${YELLOW} HP LTFS SE Edition${NOR}
        \rInput ${YELLOW}'C'${NOR} to cancel the installation.${YELLOW}(0/1/C)${NOR}: "
    exitFlag=false
    while [ $exitFlag != true ]
    do
        read ltfsVendor
        case $ltfsVendor in
            0 | IBM | ibm)
            exitFlag=true
            logMsg info "You are selecting to install the IBM LTFS SE Edition." "IBM LTFS SE"
            LTFS_DEFAULT_VENDOR="IBM"
            setOpt "$CURRENT_PATH/version" "VENDOR" "IBM"
            sleep 2
        ;;
            1 | HP | hp)
            exitFlag=true
            logMsg info "You are selecting to install the HP LTFS SE Edition." "HP LTFS SE"
            LTFS_DEFAULT_VENDOR="HP"
            setOpt "$CURRENT_PATH/version" "VENDOR" "HP"
            sleep 2
        ;;
        c | C | cancel | CANCEL )
            logMsg warn "You've canceled the installation."
            exit 0
        ;;
            *)
            logMsg warn "Unknown input $ltfsVendor" "${ltfsVendor}"
            echo -ne "0): ${YELLOW} IBM LTFS SE Edition ${NOR}
                \r1): ${YELLOW} HP LTFS SE Edition${NOR}
                \rInput ${YELLOW}'C'${NOR} to cancel the installation.${YELLOW}(0/1/C)${NOR}: "
        ;;
        esac

    done
    fi
}

function check_vendor()
{
    if [ "$2" ];then
        slctVendor=`tr '[A-Z]' '[a-z]' <<< $2`
        case $slctVendor in
        hp | HP )
                logMsg info "You are selecting to install the HP LTFS SE Edition." "HP LTFS SE"
                LTFS_DEFAULT_VENDOR="HP"
                setOpt "$CURRENT_PATH/version" "VENDOR" "HP"
                sleep 2

        ;;
        ibm | IBM )
                logMsg info "You are selecting to install the IBM LTFS SE Edition." "IBM LTFS SE"
                LTFS_DEFAULT_VENDOR="IBM"
                setOpt "$CURRENT_PATH/version" "VENDOR" "IBM"
                sleep 2

        ;;
        *)
        logMsg warn "There is no vendor of $2 you specified supported." "$2"
        chk_LTFS_Vendor
        ;;
        esac
    else
        chk_LTFS_Vendor
    fi
}

function installHpLtfs()
{
    logMsg info "Installing HP LTFS."
    cd $HPLTFS_RPM

    cd $TARPKG_ROOT
    if [ -e HPLTFS_BINARIES_x64.tar.gz ]; then
    logMsg info "HPLTFS Binraries exists." "HPLTFS"
    tar xvf HPLTFS_BINARIES_x64.tar.gz -C /
    else
    logMsg failure "HPLTFS_BINARIES_SLES11 doesn't exist" "HPLTFS_BINARIES_SLES11"
    fi
    ldconfig
    LTFS_CONF="/usr/local/etc/ltfs.conf"
    cp "${LTFS_CONF}" "/usr/local/etc/vs_ltfs.conf" 
}

function installIbmLtfsLE()
{
    echo ""
}

function uninstallIbmLtfsLE()
{
    echo ""
}
