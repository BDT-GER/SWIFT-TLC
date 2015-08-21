all: vfsserver vfsclient vfsserver-simulator vfsclient-simulator lfs_tool library_tool

FUSE_SOURCES = FuseBase.cpp FuseCallback.cpp
FUSE_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(FUSE_SOURCES))
FUSE_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(FUSE_SOURCES))
FUSE_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(FUSE_SOURCES))

REDIRECT_SOURCES = FuseRedirect.cpp FuseRedirectApp.cpp
REDIRECT_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(REDIRECT_SOURCES))
REDIRECT_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(REDIRECT_SOURCES))
REDIRECT_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(REDIRECT_SOURCES))

VFS_SOURCES = FuseBDT.cpp FuseBDTApp.cpp
VFS_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(VFS_SOURCES))
VFS_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(VFS_SOURCES))
VFS_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(VFS_SOURCES))

BDT_SOURCES = $(shell echo bdt/*.cpp)
BDT_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(BDT_SOURCES))
BDT_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(BDT_SOURCES))
BDT_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(BDT_SOURCES))

TAPE_SOURCES = $(shell echo tape/*.cpp)
TAPE_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(TAPE_SOURCES))
TAPE_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(TAPE_SOURCES))
TAPE_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(TAPE_SOURCES))

TAPE_SCSI_SOURCES = $(shell echo tape/scsi/*.cpp)
TAPE_SCSI_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(TAPE_SCSI_SOURCES))
TAPE_SCSI_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(TAPE_SCSI_SOURCES))
TAPE_SCSI_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(TAPE_SCSI_SOURCES))

TAPE_SIMULATOR_SOURCES = $(shell echo tape/simulator/*.cpp)
TAPE_SIMULATOR_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(TAPE_SIMULATOR_SOURCES))
TAPE_SIMULATOR_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(TAPE_SIMULATOR_SOURCES))
TAPE_SIMULATOR_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(TAPE_SIMULATOR_SOURCES))

LIB_CONFIG_SOURCES = $(shell echo lib/config/*.cpp)
LIB_CONFIG_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(LIB_CONFIG_SOURCES))
LIB_CONFIG_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(LIB_CONFIG_SOURCES))
LIB_CONFIG_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(LIB_CONFIG_SOURCES))

LIB_COMMON_SOURCES = $(shell echo lib/common/*.cpp)
LIB_COMMON_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(LIB_COMMON_SOURCES))
LIB_COMMON_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(LIB_COMMON_SOURCES))
LIB_COMMON_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(LIB_COMMON_SOURCES))

LIB_LTFS_SOURCES = $(shell echo lib/ltfs_library/*.cpp)
LIB_LTFS_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(LIB_LTFS_SOURCES))
LIB_LTFS_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(LIB_LTFS_SOURCES))
LIB_LTFS_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(LIB_LTFS_SOURCES))

LIB_DB_SOURCES = $(shell echo lib/database/*.cpp)
LIB_DB_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(LIB_DB_SOURCES))
LIB_DB_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(LIB_DB_SOURCES))
LIB_DB_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(LIB_DB_SOURCES))

LIB_LTFS_SIMULATOR_SOURCES = $(shell echo lib/ltfs_library/simulator/*.cpp)
LIB_LTFS_SIMULATOR_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(LIB_LTFS_SIMULATOR_SOURCES))
LIB_LTFS_SIMULATOR_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(LIB_LTFS_SIMULATOR_SOURCES))
LIB_LTFS_SIMULATOR_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(LIB_LTFS_SIMULATOR_SOURCES))

LTFS_MANAGEMENT_SOURCES = $(shell echo ltfs_management/*.cpp)
LTFS_MANAGEMENT_DEBUG = $(patsubst %.cpp,debug/%.o,$(LTFS_MANAGEMENT_SOURCES))
LTFS_MANAGEMENT_RELEASE = $(patsubst %.cpp,release/%.o,$(LTFS_MANAGEMENT_SOURCES))
LTFS_MANAGEMENT_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(LTFS_MANAGEMENT_SOURCES))

LTFS_FORMAT_SOURCES = $(shell echo ltfs_format/*.cpp)
LTFS_FORMAT_DEBUG = $(patsubst %.cpp,debug/%.o,$(LTFS_FORMAT_SOURCES))
LTFS_FORMAT_RELEASE = $(patsubst %.cpp,release/%.o,$(LTFS_FORMAT_SOURCES))
LTFS_FORMAT_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(LTFS_FORMAT_SOURCES))

SOCKET_SOURCES = $(shell echo socket/*.cpp)
SOCKET_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(SOCKET_SOURCES))
SOCKET_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(SOCKET_SOURCES))
SOCKET_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(SOCKET_SOURCES))

LOG_SOURCES = $(shell echo log/*.cpp)
LOG_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(LOG_SOURCES))
LOG_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(LOG_SOURCES))
LOG_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(LOG_SOURCES))

TINY_XML_SOURCES = $(shell echo tinyxml/*.cpp)
TINY_XML_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(TINY_XML_SOURCES))
TINY_XML_OBJECTS_RELEASE = $(patsubst %.cpp,release/%.o,$(TINY_XML_SOURCES))
TINY_XML_OBJECTS_SIMULATOR = $(patsubst %.cpp,simulator/%.o,$(TINY_XML_SOURCES))

UTIILITY_SOURCES = $(shell echo utility/*.cpp)
UTIILITY_OBJECTS_DEBUG = $(patsubst %.cpp,debug/%.o,$(UTIILITY_SOURCES))

CPPFLAGS = -DWITH_OPENSSL -D_FILE_OFFSET_BITS=64 -DBOOST_REGEX_NO_EXTERNAL_TEMPLATES -Wall -Wno-unused-local-typedefs -I /root/log4cplus/include -I /root/mysql-connector/include -I /usr/include/python2.7
LDFLAGS = -Wl,-rpath /usr/VS/lib -L /usr/VS/lib -pthread -lcrypto -lrt -ldl -lboost_filesystem -lboost_thread -lboost_date_time -lboost_regex -lboost_iostreams -lboost_system  -lssl -llog4cplus -lxmlrpc -lxmlrpc_util -lxmlrpc_server -lxmlrpc_packetsocket -lxmlrpc++ -lxmlrpc_server++ -lxmlrpc_server_pstream++ -lxmlrpc_client -lxmlrpc_client++ -lmysqlcppconn -lfuse -lpython2.7
LIC_LDFLAGS = -Wl,-rpath /usr/VS/lib -L /usr/VS/lib -pthread -lrt -ldl -lboost_filesystem -lboost_thread -lboost_date_time -lboost_regex -lboost_iostreams -lboost_system -llog4cplus -lxmlrpc -lxmlrpc_util -lxmlrpc_server -lxmlrpc_packetsocket -lxmlrpc++ -lxmlrpc_server++ -lxmlrpc_server_pstream++ -lxmlrpc_client -lxmlrpc_client++ -lfuse 


bdt-redirect: $(REDIRECT_OBJECTS_RELEASE) $(FUSE_OBJECTS_RELEASE)
	g++ -o $@ $^ -Wl,-rpath /usr/VS/lib -L /usr/VS/lib -pthread -lfuse -lrt -ldl -lboost_filesystem -lboost_thread -lboost_date_time -lboost_regex -lboost_iostreams

bdt-redirect-debug: $(REDIRECT_OBJECTS_DEBUG) $(FUSE_OBJECTS_DEBUG)
	g++ -o $@ $^ -Wl,-rpath /usr/VS/lib -L /usr/VS/lib -pthread -lfuse -lrt -ldl -lboost_filesystem -lboost_thread -lboost_date_time -lboost_regex -lboost_iostreams

bdt-redirect-simulator: $(REDIRECT_OBJECTS_SIMULATOR) $(FUSE_OBJECTS_SIMULATOR)
	g++ -o $@ $^ -Wl,-rpath /usr/VS/lib -L /usr/VS/lib -pthread  -lrt -ldl -lboost_filesystem -lboost_thread -lboost_date_time -lboost_regex -lboost_iostreams -lfuse

debug/libbdtltfs_tape.a: $(TAPE_OBJECTS_DEBUG)
	ar rcs $@ $^

release/libbdtltfs_tape.a: $(TAPE_OBJECTS_RELEASE)
	ar rcs $@ $^

simulator/libbdtltfs_tape.a: $(TAPE_OBJECTS_SIMULATOR)
	ar rcs $@ $^

debug/libbdtltfs_tape_simulator.a: $(TAPE_SIMULATOR_OBJECTS_DEBUG)
	ar rcs $@ $^

release/libbdtltfs_tape_simulator.a: $(TAPE_SIMULATOR_OBJECTS_RELEASE)
	ar rcs $@ $^

simulator/libbdtltfs_tape_simulator.a: $(TAPE_SIMULATOR_OBJECTS_SIMULATOR)
	ar rcs $@ $^
	
vfsserver: release/FuseBDTServer.o $(BDT_OBJECTS_RELEASE) release/libbdtltfs_tape.a $(LIB_CONFIG_OBJECTS_RELEASE) $(LIB_COMMON_OBJECTS_RELEASE) $(LIB_LTFS_OBJECTS_RELEASE) $(LIB_DB_OBJECTS_RELEASE) $(LTFS_MANAGEMENT_RELEASE) $(LTFS_FORMAT_RELEASE) $(SOCKET_OBJECTS_RELEASE) $(LOG_OBJECTS_RELEASE) $(TINY_XML_OBJECTS_RELEASE) 
	g++ -o $@ $^ $(LDFLAGS) -lboost_iostreams 

vfsserver-debug: debug/FuseBDTServer.o $(BDT_OBJECTS_DEBUG) debug/libbdtltfs_tape.a $(LIB_CONFIG_OBJECTS_DEBUG) $(LIB_COMMON_OBJECTS_DEBUG) $(LIB_LTFS_OBJECTS_DEBUG) $(LIB_DB_OBJECTS_DEBUG) $(LTFS_MANAGEMENT_DEBUG) $(LTFS_FORMAT_DEBUG) $(SOCKET_OBJECTS_DEBUG) $(LOG_OBJECTS_DEBUG) $(TINY_XML_OBJECTS_DEBUG)  
	g++ -o $@ $^ $(LDFLAGS) -lboost_iostreams 

vfsclient: $(VFS_OBJECTS_RELEASE) $(BDT_OBJECTS_RELEASE) $(FUSE_OBJECTS_RELEASE) release/libbdtltfs_tape.a $(LIB_CONFIG_OBJECTS_RELEASE) $(LIB_COMMON_OBJECTS_RELEASE) $(LIB_LTFS_OBJECTS_RELEASE) $(LIB_DB_OBJECTS_RELEASE) $(LTFS_MANAGEMENT_RELEASE) $(LTFS_FORMAT_RELEASE) $(SOCKET_OBJECTS_RELEASE) $(LOG_OBJECTS_RELEASE) $(TINY_XML_OBJECTS_RELEASE) 
	g++ -o $@ $^ $(LDFLAGS)

vfsclient-debug: $(VFS_OBJECTS_DEBUG) $(BDT_OBJECTS_DEBUG) $(FUSE_OBJECTS_DEBUG) debug/libbdtltfs_tape.a $(LIB_CONFIG_OBJECTS_DEBUG) $(LIB_COMMON_OBJECTS_DEBUG) $(LIB_LTFS_OBJECTS_DEBUG) $(LIB_DB_OBJECTS_DEBUG) $(LTFS_MANAGEMENT_DEBUG) $(LTFS_FORMAT_DEBUG) $(SOCKET_OBJECTS_DEBUG) $(LOG_OBJECTS_DEBUG) $(TINY_XML_OBJECTS_DEBUG) 
	g++ -o $@ $^ $(LDFLAGS) 

vfsserver-simulator: simulator/FuseBDTServer.o $(BDT_OBJECTS_SIMULATOR) simulator/libbdtltfs_tape.a $(LIB_CONFIG_OBJECTS_SIMULATOR) $(LIB_COMMON_OBJECTS_SIMULATOR) $(LIB_LTFS_OBJECTS_SIMULATOR) $(LIB_LTFS_SIMULATOR_OBJECTS_SIMULATOR) $(LIB_DB_OBJECTS_SIMULATOR) $(LTFS_MANAGEMENT_SIMULATOR)  $(LTFS_FORMAT_SIMULATOR) $(SOCKET_OBJECTS_SIMULATOR) $(LOG_OBJECTS_SIMULATOR)  $(TINY_XML_OBJECTS_SIMULATOR)  
	g++ -o $@ $^ $(LDFLAGS)

vfsclient-simulator:$(VFS_OBJECTS_SIMULATOR) $(BDT_OBJECTS_SIMULATOR) $(FUSE_OBJECTS_SIMULATOR) simulator/libbdtltfs_tape.a $(LIB_CONFIG_OBJECTS_SIMULATOR) $(LIB_COMMON_OBJECTS_SIMULATOR) $(LIB_LTFS_OBJECTS_SIMULATOR) $(LIB_LTFS_SIMULATOR_OBJECTS_SIMULATOR) $(LIB_DB_OBJECTS_SIMULATOR) $(LTFS_MANAGEMENT_SIMULATOR) $(LTFS_FORMAT_SIMULATOR) $(SOCKET_OBJECTS_SIMULATOR) $(LOG_OBJECTS_SIMULATOR)  $(TINY_XML_OBJECTS_SIMULATOR) 
	g++ -o $@ $^ $(LDFLAGS) 
	
lfs_tool: debug/utility/lfs_tool.o $(LIB_LTFS_OBJECTS_DEBUG) $(LOG_OBJECTS_DEBUG) $(BDT_OBJECTS_DEBUG) $(LIB_CONFIG_OBJECTS_DEBUG) $(LIB_COMMON_OBJECTS_DEBUG) $(LIB_LTFS_OBJECTS_DEBUG) $(LIB_DB_OBJECTS_DEBUG) $(LTFS_MANAGEMENT_DEBUG) $(LTFS_FORMAT_DEBUG) $(SOCKET_OBJECTS_DEBUG) $(LOG_OBJECTS_DEBUG) $(TINY_XML_OBJECTS_DEBUG)
	g++ -o $@ $^ $(LDFLAGS)
	
library_tool: debug/utility/library_tool.o $(LIB_LTFS_OBJECTS_DEBUG) $(LOG_OBJECTS_DEBUG) $(BDT_OBJECTS_DEBUG) $(LIB_CONFIG_OBJECTS_DEBUG) $(LIB_COMMON_OBJECTS_DEBUG) $(LIB_LTFS_OBJECTS_DEBUG) $(LIB_DB_OBJECTS_DEBUG) $(LTFS_MANAGEMENT_DEBUG) $(LTFS_FORMAT_DEBUG) $(SOCKET_OBJECTS_DEBUG) $(LOG_OBJECTS_DEBUG) $(TINY_XML_OBJECTS_DEBUG)
	g++ -o $@ $^ $(LDFLAGS)

release/%.o: %.cpp
	mkdir release/bdt -p
	mkdir release/tape -p
	mkdir release/tape/simulator -p
	mkdir release/tape/scsi -p	
	mkdir release/lib/config -p
	mkdir release/lib/common -p
	mkdir release/lib/ltfs_library -p
	mkdir release/lib/ltfs_library/simulator -p
	mkdir release/lib/database -p
	mkdir release/ltfs_format -p
	mkdir release/ltfs_management -p
	mkdir release/socket -p
	mkdir release/log -p
	mkdir release/tinyxml -p
	mkdir release/utility -p
	g++ -g -O2 -c $(CPPFLAGS) $< -o $@

debug/%.o: %.cpp
	mkdir debug/bdt -p
	mkdir debug/tape -p
	mkdir debug/tape/simulator -p
	mkdir debug/tape/scsi -p
	mkdir debug/lib/config -p
	mkdir debug/lib/common -p
	mkdir debug/lib/ltfs_library -p
	mkdir debug/lib/ltfs_library/simulator -p
	mkdir debug/lib/database -p
	mkdir debug/ltfs_format -p
	mkdir debug/ltfs_management -p
	mkdir debug/socket -p
	mkdir debug/log -p
	mkdir debug/tinyxml -p
	mkdir debug/utility -p
	g++ -DDEBUG -g -O0 -c $(CPPFLAGS) $< -o $@

simulator/%.o: %.cpp
	mkdir simulator/bdt -p
	mkdir simulator/tape -p
	mkdir simulator/tape/simulator -p
	mkdir simulator/tape/scsi -p
	mkdir simulator/lib/config -p
	mkdir simulator/lib/common -p
	mkdir simulator/lib/ltfs_library -p
	mkdir simulator/lib/ltfs_library/simulator -p
	mkdir simulator/lib/database -p
	mkdir simulator/ltfs_format -p
	mkdir simulator/ltfs_management -p
	mkdir simulator/socket -p
	mkdir simulator/log -p
	mkdir simulator/tinyxml -p
	g++ -DDEBUG -DSIMULATOR -g -O0 -c $(CPPFLAGS) $< -o $@
	
clean:
	rm -rf debug release simulator
	rm -f $(FUSE_OBJECTS_RELEASE) $(FUSE_OBJECTS_DEBUG) $(FUSE_OBJECTS_SIMULATOR) 
	rm -f $(REDIRECT_OBJECTS_RELEASE) $(REDIRECT_OBJECTS_DEBUG) $(REDIRECT_OBJECTS_SIMULATOR) 
	rm -f $(VFS_OBJECTS_RELEASE) $(VFS_OBJECTS_DEBUG) $(VFS_OBJECTS_SIMULATOR)
	rm -f $(BDT_OBJECTS_RELEASE) $(BDT_OBJECTS_DEBUG) $(BDT_OBJECTS_SIMULATOR)   
	rm -f $(TAPE_OBJECTS_RELEASE) $(TAPE_OBJECTS_DEBUG) $(TAPE_OBJECTS_SIMULATOR)
	rm -f $(TAPE_SIMULATOR_OBJECTS_RELEASE) $(TAPE_SIMULATOR_OBJECTS_DEBUG) $(TAPE_SIMULATOR_OBJECTS_SIMULATOR) 
	rm -f $(TAPE_SCSI_OBJECTS_RELEASE) $(TAPE_SCSI_OBJECTS_DEBUG) $(TAPE_SCSI_OBJECTS_SIMULATOR) 
	rm -f $(LIB_LTFS_OBJECTS_RELEASE) $(LIB_LTFS_OBJECTS_DEBUG) $(LIB_LTFS_OBJECTS_SIMULATOR) 
	rm -f $(LIB_LTFS_SIMULATOR_OBJECTS_RELEASE) $(LIB_LTFS_SIMULATOR_OBJECTS_DEBUG) $(LIB_LTFS_SIMULATOR_OBJECTS_SIMULATOR)  
	rm -f $(SOCKET_OBJECTS_RELEASE) $(SOCKET_OBJECTS_DEBUG) $(SOCKET_OBJECTS_SIMULATOR)  
	rm -f $(LOG_OBJECTS_RELEASE) $(LOG_OBJECTS_DEBUG) $(LOG_OBJECTS_SIMULATOR)  
	rm -f $(TINY_XML_OBJECTS_RELEASE) $(TINY_XML_OBJECTS_DEBUG) $(TINY_XML_OBJECTS_SIMULATOR)
	rm -f $(UTIILITY_OBJECTS_DEBUG)
	rm -f bdt-* vfsclient* vfsserver* lfs_tool library_tool

