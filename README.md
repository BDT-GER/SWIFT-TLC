# SWIFT-TLC
OpenStack SWIFT Object Storage Tape Library Connector

What is it?
-----------
Modern cloud storage architectures like OpenStack SWIFT Object Storage do not natively support Tape Storage Media and Tape Libraries as storage targets.
This SWIFT-TLC software solves this limitation. SWIFT-TLC allows using a tape library as storage target for an SWIFT object storage node. It is possible to extend existing SWIFT cluster by adding tape storage or building new SWIFT tape based object storages as well.
The aim of the TLC is to make all specific behavior of a tape library completely transparent to SWIFT (that’s why we call it connector). This ensures that there are no SWIFT modifications necessary to add tape storage to a SWIFT Cluster. The SWIFT TLC storage node uses exactly the same interface (Rest API) as a standard SWIFT storage node. 
All SWIFT features like ring controlled data placement, SWIFT policies, replication, auditing are fully supported.

Get started:
------------
Evaluation
----------
    Download the installer packages (*.gz) from folder /. This packages can be installed on CentOS
    Follow the QuickStart guide (SWIFT_TLC_QuickStart.docx) to get it running

Source Code
-----------
    Folder /TLC_ALL/Server contains all source code files
    Run the script build_package.sh to build the binaries

More Information:
-----------------
  If you are interested in more detailed technical information please refer these documents:
    SWIFT Technical Overview document,
    SWIFT Tape Auditor Overview Document

Get involved:
-------------
We hope by offering our SWIFT TLC as Open Source software that native nonproprietary tape storage support becomes a part of the OpenStack ecosystem.
Everybody is highly invited to participate in this project and can join the development team.

Contact:
--------
Andreas Kleber,
andreas.kleber@bdt.de,
+49 (151) 148650 65

