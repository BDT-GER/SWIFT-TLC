TESTS = BDT_TAPE_Test
check_PROGRAMS = $(TESTS)

BDT_TAPE_Test_SOURCES = bdt-tape_test.cpp \
    TapeLibraryManagerTest.cpp TapeLibraryManagerTest.h \
    ObserverTest.cpp ObserverTest.h \
    ObserverListenerSub.cpp ObserverListenerSub.h \
    ChangerTest.cpp ChangerTest.h \
    DriveTest.cpp DriveTest.h

BDT_TAPE_Test_LDADD = ../../debug/libbdtltfs_tape.a \
    ../../debug/libbdtltfs_tape_simulator.a

BDT_TAPE_Test_CXXFLAGS = $(CPPUNIT_CFLAGS)
BDT_TAPE_Test_LDFLAGS = $(CPPUNIT_LIBS) -ldl -lboost_filesystem -lboost_thread -lboost_date_time
