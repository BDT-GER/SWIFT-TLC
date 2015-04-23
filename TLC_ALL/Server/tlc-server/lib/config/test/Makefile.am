TESTS = LTFS_CONFIG_Test
check_PROGRAMS = $(TESTS)

LTFS_CONFIG_Test_SOURCES = CfgManager_Test.cpp \
	CfgManagerTest.cpp CfgManagerTest.h \
	../CfgManager.cpp ../CfgManager.h \
	../CfgParser.cpp ../CfgParser.h \
	../CfgSerialization.cpp ../CfgSerialization.h \
	../CfgDataDetail.h ../stdafx.h \
	../../../log/loggerManager.cpp ../../../log/loggerManager.h \
	../../../tinyxml/tinyxml.cpp ../../../tinyxml/tinyxml.h \
	../../../tinyxml/tinystr.cpp ../../../tinyxml/tinystr.h \
	../../../tinyxml/tinyxmlerror.cpp ../../../tinyxml/tinyxmlparser.cpp

LTFS_CONFIG_Test_CXXFLAGS = -I /root/boost/include -I /root/log4cplus/include $(CPPUNIT_CFLAGS)  -DDO_UNIT_TEST -DDO_AUTO_TEST
LTFS_CONFIG_Test_LDFLAGS = $(CPPUNIT_LIBS) -Wl,-rpath /usr/LTFStor/lib -ldl -lboost_filesystem -lboost_system -lboost_thread -lboost_date_time -lboost_regex  -lboost_iostreams -llog4cplus -L /usr/LTFStor/lib
