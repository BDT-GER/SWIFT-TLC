TESTS = LTFS_SIMULATOR_Test
check_PROGRAMS = $(TESTS)

LTFS_SIMULATOR_Test_SOURCES = Simulator_Test.cpp \
	CfgfileParserTest.cpp CfgfileParserTest.h \
	SimulatorTest.cpp SimulatorTest.h \
	../CfgfileParser.cpp ../CfgfileParser.h \
	../Simulator.cpp ../Simulator.h \
	../SimChanger.cpp ../SimChanger.h \
	../SimDrive.cpp ../SimDrive.h \
	../SimSlot.cpp ../SimSlot.h \
	../SimMailSlot.cpp ../SimMailSlot.h \
	../SimTape.cpp ../SimTape.h \
	../stdafx.h \
	../../../../log/loggerManager.cpp ../../../../log/loggerManager.h

LTFS_SIMULATOR_Test_CXXFLAGS = $(CPPUNIT_CFLAGS) -DDO_UNIT_TEST -DDO_AUTO_TEST \
    -I/root/boost/include \
    -I/root/xmlrpc/include \
    -I/root/log4cplus/include \
    -I /root/xmlrpc/include \
    -I /root/mysql-connector/include
LTFS_SIMULATOR_Test_LDFLAGS = $(CPPUNIT_LIBS) -Wl,-rpath /usr/VS/lib \
    -L /usr/VS/lib \
    -ldl \ 
    -lboost_filesystem \
    -lboost_thread \
    -lboost_date_time \
    -lboost_regex  \
    -lboost_iostreams \
    -llog4cplus 
