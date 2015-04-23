TESTS = Test
check_PROGRAMS = $(TESTS)

test_source_Main = \
bdt-ltfs_test.cpp

test_source_File = \
FileOperationBitmapTest.cpp \
FileDigestTest.cpp \
FileOperationTest.cpp \
ExtendedAttributeTest.cpp

test_source_Schedule = \
PriorityTapeGroupTest.cpp \
ScheduleElevator.cpp \
FileOperationScheduleTest.cpp \
PriorityTapeTask.cpp \
PriorityTapeTest.cpp \
ScheduleTask.cpp \
ScheduleTapeTest.cpp \
SchedulePriorityTapeTest.cpp

test_source_Tape = \
TapeManagerTest.cpp 

test_source_Cache = \
CacheManagerTest.cpp

test_source_Meta = \
MetaManagerTest.cpp \
ReadTaskTest.cpp \
ReadManagerTest.cpp \
InodeHandlerTest.cpp

test_source_CIFS = \
CIFSWaitTest.cpp

test_source_Misc = \
FactoryTest.cpp \
ConfigureTest.cpp \
ThrottleTest.cpp \
ScheduleNone.cpp

test_source_TODO = \
FileOperationCIFSTest.cpp \
FileOperationPriorityTest.cpp \
FileOperationReadWriteCacheTest.cpp \
ServiceDiagnoseTest.cpp \
ReadCacheManagerTest.cpp \
TapeManagerSETest.cpp \
TapeManagerLETest.cpp \
ReadCacheInformationTest.cpp \
FileOperationTapeStatusTest.cpp \
FileOperationReadCacheTest.cpp

test_source_Obsolete = \
ResourceManagerTest.cpp \
MetaManagerTapeStatusTest.cpp \
MetaManagerCatalogTest.cpp \
FileOperationMetaTest.cpp \
FileCollectionTest.cpp

Test_SOURCES = \
    $(test_source_Main) \
    $(test_source_Misc) \
    $(test_source_CIFS) \
    $(test_source_Meta) \
    $(test_source_Cache) \
    $(test_source_File) \
    ResourceTapeSimulator.cpp \
    ReadTapeFileSimulator.cpp \
    ../Inode.cpp ../Factory.cpp ../TapeManager.cpp ../TapeManagerLE.cpp \
    ../TapeManagerSE.cpp ../MetaManager.cpp \
    ../CacheManager.cpp \
    ../FileOperationCIFS.cpp ../PriorityTapeGroup.cpp \
    ../FileOperation.cpp ../FileOperationTape.cpp \
    ../ExtendedAttribute.cpp \
    ../CIFSWait.cpp ../PriorityTape.cpp \
    ../SchedulePriorityTape.cpp \
    ../Configure.cpp ../FileDigest.cpp \
    ../ScheduleProxy.cpp ../ScheduleProxyServer.cpp ../TapeManagerProxy.cpp \
    ../TapeManagerProxyServer.cpp ../SocketServer.cpp ../Throttle.cpp \
    ../ScheduleAccount.cpp \
    ../ReadTask.cpp ../ReadManager.cpp \
    ../FileOperationDelay.cpp \
    ../FileOperationPriority.cpp \
    ../MetaDatabase.cpp \
    ../InodeHandler.cpp ../Bitmap.cpp ../FileOperationBitmap.cpp \
    ../TapeManagerStop.cpp ../FileOperationInodeHandler.cpp

Test_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -I/root/boost/include -I/root/xmlrpc/include -D_FILE_OFFSET_BITS=64 -DMORE_TEST -Wno-unused-local-typedefs
Test_LDFLAGS = $(CPPUNIT_LIBS) -Wl,-rpath /usr/VS/lib -L /usr/VS/lib -ldl -lboost_system -lboost_filesystem -lboost_thread -lboost_date_time -lboost_regex -lcrypto -lssl -lxmlrpc -lxmlrpc_util -lxmlrpc_xmlparse -lxmlrpc_xmltok -lxmlrpc_server -lxmlrpc_packetsocket -lxmlrpc++ -lxmlrpc_server++ -lxmlrpc_server_pstream++ -lxmlrpc_client -lxmlrpc_client++
Test_LDADD = ../../debug/libbdtltfs_tape.a ../../debug/libbdtltfs_tape_simulator.a
