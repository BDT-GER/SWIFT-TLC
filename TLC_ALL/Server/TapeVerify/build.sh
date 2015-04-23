#!/bin/bash
rm -rf lib Makefile
VENDER="IBM"
case "$1" in  
    ibm|IBM)
        VENDER="IBM"
        ;;
    hp|HP)
        VENDER="HP"
        ;;
    clean|CLEAN)
        rm -rf *.o lib *.gz TapeVerifyTool
        exit 0
        ;;
    esac
cp Makefile.${VENDER} Makefile
cp -r lib_${VENDER} lib
make clean
make
if [ $? != 0 ]; then
    echo "Failed to make."
    exit 1;
fi
TAR_PACKAGE="TapeVerifyTool"${VENDER}".tar.gz";
tar zcf ${TAR_PACKAGE} lib  TapeVerifyTool

if [ $? != 0 ]; then
    echo "Failed build ${VENDER} package."
    exit 1;
fi
exit 0
