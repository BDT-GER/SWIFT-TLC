#/bin/bash

OBJECT_SERVER_CONF="/etc/swift/object-server.conf"
AUDITOR_PY="/usr/lib/python2.7/site-packages/swift/obj/auditor.py"
AUDITOR_OBJECT_PATH="/usr/lib/python2.7/site-packages/swift/obj/"
NEW_EGG_NAME="vs_auditor#vs_object"

case "$1" in
    install|INSTALL|i|"")
    yum install -y patch

    if [ -e ${AUDITOR_OBJECT_PATH}/vs_server.py ]; then 
        echo "##########################################"
        echo "The SWIFT-Tape-Auditor should have been installed. If you want to install this one, Please uninstall the existing one first."
        echo "##########################################"
        exit 1
    fi

    /usr/bin/python setup.py install
    if [ $? -ne 0 ]; then
    echo "Failed to setup swift vs backend."
        exit 1
    fi

    grep "use = egg:"${NEW_EGG_NAME} ${OBJECT_SERVER_CONF}
    if [ $? -ne 0 ]; then
        sed -i 's/use = egg:swift#object/use = egg:'${NEW_EGG_NAME}'/g' ${OBJECT_SERVER_CONF}
    fi


    cp -f ${AUDITOR_PY} ${AUDITOR_PY}.org
    patch -p1 < auditor.patch ${AUDITOR_PY}
    if [ $? -ne 0 ]; then    
        echo "Failed to patch auditor.py."
        exit 1
    fi
   
    python ./vs_swift_conf.py ${OBJECT_SERVER_CONF} install
    if [ $? -ne 0 ]; then    
        echo "Failed to setup swift vs config."
        exit 1
    fi
    # install TapeVerifyTool
    cd ./TapeVerifyTool && ./install.sh
    if [ $? -ne 0 ]; then    
        echo "Failed to install TapeVerifyTool."
        exit 1
    fi
    echo "SWIFT-Tape-Auditor installed."
    exit 0
    ;;
    uninstall|UNINSTALL)
    grep "use = egg:"${NEW_EGG_NAME} ${OBJECT_SERVER_CONF}
    if [ $? -eq 0 ]; then
        sed -i 's/use = egg:'${NEW_EGG_NAME}'/use = egg:swift#object/g' ${OBJECT_SERVER_CONF}
    fi

    cp -f ${AUDITOR_PY}.org ${AUDITOR_PY}
    rm -f /*.py ${AUDITOR_OBJECT_PATH}/vs_diskfile.py ${AUDITOR_OBJECT_PATH}/vs_server.py
    rm -f /usr/lib/python2.7/site-packages/vs_auditor-0.1-py2.7.egg
    python ./vs_swift_conf.py ${OBJECT_SERVER_CONF} uninstall
    # uninstall TapeVerifyTool
    cd ./TapeVerifyTool && ./install.sh uninstall
    if [ $? -ne 0 ]; then    
        echo "Failed to uninstall TapeVerifyTool."
        exit 1
    fi
    echo "SWIFT-Tape-Auditor uninstalled."
    exit 0
    ;;
    *)
    echo $"Usage: $0 {install | uninstall}"
    exit 2
esac
exit 0
