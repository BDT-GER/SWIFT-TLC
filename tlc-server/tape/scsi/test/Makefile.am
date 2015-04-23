TESTS = BDT_TAPE_Test
check_PROGRAMS = $(TESTS)

BDT_TAPE_Test_SOURCES = bdt-tape_scsi_test.cpp \
    TapeLibraryManagerTest.cpp TapeLibraryManagerTest.h \
    ChangerTest.cpp ChangerTest.h \
    DriveTest.cpp DriveTest.h \
    CartridgeTest.cpp CartridgeTest.h \
    SlotTest.cpp SlotTest.h \
    TestBase.cpp TestBase.h \
    ../../TapeLibraryManager.h ../TapeLibraryManager.cpp \
    ../../Changer.h ../Changer.cpp \
    ../../Drive.h ../Drive.cpp \
    ../../Slot.h ../Slot.cpp \
    ../../Cartridge.h ../Cartridge.cpp \
    ../../Error.h ../../Error.cpp \
    ../cmn.h ../cmn.cpp \
    ../../../lib/ltfs_library/LtfsChanger.h ../../../lib/ltfs_library/LtfsChanger.cpp \
    ../../../lib/ltfs_library/LtfsDrive.h ../../../lib/ltfs_library/LtfsDrive.cpp \
    ../../../lib/ltfs_library/LtfsTape.h ../../../lib/ltfs_library/LtfsTape.cpp \
    ../../../lib/ltfs_library/LtfsScsiDevice.h ../../../lib/ltfs_library/LtfsScsiDevice.cpp \
    ../../../lib/ltfs_library/LtfsMailSlot.h ../../../lib/ltfs_library/LtfsMailSlot.cpp \
    ../../../lib/ltfs_library/LtfsSlot.h ../../../lib/ltfs_library/LtfsSlot.cpp \
    ../../../lib/ltfs_library/LtfsError.h ../../../lib/ltfs_library/LtfsError.cpp \
    ../../../lib/ltfs_library/CmnFunc.h ../../../lib/ltfs_library/CmnFunc.cpp \
    ../../../ltfs_management/TapeLibraryMgr.h ../../../ltfs_management/TapeLibraryMgr.cpp \
    ../../../ltfs_management/TapeDbManager.h ../../../ltfs_management/TapeDbManager.cpp \
    ../../../lib/database/sql_Database.h ../../../lib/database/sql_Database.cpp
    
BDT_TAPE_Test_CXXFLAGS = -I /root/boost/include $(CPPUNIT_CFLAGS) -DDEBUG -DBOOST_REGEX_NO_EXTERNAL_TEMPLATES
BDT_TAPE_Test_LDFLAGS = $(CPPUNIT_LIBS)  -ldl -Wl,-rpath /usr/LTFStor/lib \
    -lboost_filesystem  -lboost_system -lboost_thread \
    -lboost_date_time -lboost_regex  -lboost_iostreams \
    -lsqlite3 \
    -L /usr/LTFStor/lib 
