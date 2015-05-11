/* Copyright (c) 2012 BDT Media Automation GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * FuseBDTServer.cpp
 *
 *  Created on: Oct 26, 2012
 *      Author: More Zeng
 */


#include "bdt/stdafx.h"
#include "bdt/ServiceServer.h"
#include "bdt/TapeManagerProxyServer.h"
#include "bdt/ScheduleProxyServer.h"
#include "bdt/CacheManager.h"
#include "bdt/CacheMonitorServer.h"

#include "socket/ltfsTaskManagement.h"

using namespace bdt;
namespace bdt
{
    void
    Factory::ReleaseCacheManager()
    {
        cache_.reset();
    }
}


static const char* vfsser_pid_path = "/var/run/";

static bool running = true;


static void
SigExit(int sig)
{
    running = false;
}


static void
UsageOutput(const char * base)
{
    cerr << base << " $TapeFolder $MetaFolder $CacheFolder" << endl;
}

int main(int argc, char *argv[])
{
    if ( argc < 4 ) {
        UsageOutput(argv[0]);
        return 1;
    }

    string tapeFolder(argv[1]);
    string metaFolder(argv[2]);
    string cacheFolder(argv[3]);
    fs::path folderTape(fs::system_complete(fs::path(tapeFolder)));
    fs::path folderMeta(fs::system_complete(fs::path(metaFolder)));
    fs::path folderCache(fs::system_complete(fs::path(cacheFolder)));
    if ( ! fs::is_directory(folderTape) ) {
        cerr << "$TapeFolder: " << tapeFolder
                << " does not exist" << endl;
        return 2;
    }
    if ( ! fs::is_directory(folderMeta) ) {
        cerr << "$MetaFolder: " << metaFolder
                << " does not exist" << endl;
        return 2;
    }
    if ( ! fs::is_directory(folderCache) ) {
        cerr << "$CacheFolder: " << cacheFolder
                << " does not exist" << endl;
        return 2;
    }

#define BDT_DAEMON
#ifdef  BDT_DAEMON
    pid_t pid = fork();
    if ( pid < 0 ) {
        return -1;
    } else if ( pid != 0 ) {
        return 0;
    }

    setsid();
    chdir("/");
    umask(0);

#endif
    signal(SIGTERM,SigExit);

    //save pid
    char cmd[128];
    char vfsserver_pid_file[256];
    char *programname = strrchr(argv[0], '/');
    sprintf(vfsserver_pid_file, "%s%s.pid",vfsser_pid_path, programname);
    sprintf(cmd, "echo %d > %s", getpid(), vfsserver_pid_file);
    if(0 != ::system(cmd))
    {
    	LogError("Failed to write pid to " << vfsserver_pid_file);
    }


    Factory::SetMetaFolder(folderMeta);
    Factory::SetCacheFolder(folderCache);
    Factory::SetTapeFolder(folderTape);
#ifdef MORE_TEST
    CreateLogger("/tmp/bdt-server");
#endif
    Factory::CreateConfigure();
#ifdef MORE_TEST
    Factory::GetConfigure()->Refresh("/tmp/bdt.config");
    Factory::CreateTapeLibraryManager();
#endif

    auto_ptr<TapeManagerProxyServer> tape;
    tape.reset(new TapeManagerProxyServer());

    auto_ptr<ScheduleProxyServer> schedule;
    schedule.reset(new ScheduleProxyServer());

    auto_ptr<CacheMonitorServer> purge;
    purge.reset(new CacheMonitorServer());

    using namespace ltfs_management;
    // lock the libraries
    TapeLibraryMgr::Instance()->PreventChangerMediaRemoval(true);


	using namespace ltfs_soapserver;
	SystemHwMode sysHwMode = TapeLibraryMgr::Instance()->GetSystemHwMode();
	if (sysHwMode == SYSTEM_HW_NEED_DIAGNOSE || sysHwMode == SYSTEM_HW_DIAGNOSING) {
		LogInfo("Dirty shutdown, share and task will not be started.");
	}else if(sysHwMode == SYSTEM_HW_NO_LICENSE){
		LogInfo("No valid license, share and task will not be started.");
	}
	else {
		//mount all shares
		int mountRet = TapeLibraryMgr::Instance()->MountAllShare();
		if (mountRet != 0)
		{
			LogError("Failed to mount all samba shares:" << mountRet);
		}
		//Load Queue
		ltfs_soapserver::TaskManagement::GetInstance()->LoadTaskQueue();
	}

	// add or update tape group for openstack swift node
	if(!TapeLibraryMgr::Instance()->AddUpdateTapeGroupForSwift()){
		LogError("Failed to and/update tape group for swift node.");
	}

	EventInfo("Service_Start", "System service started.");

    while(running) {
        sleep(1);
#ifdef BDT_DAEMON
#else
        int command;
        string service;
        cin >> command >> service;

        int handle = Factory::SocketClientHandle(
                ServiceServer::Service + service );
        if ( handle < 0 ) {
            cerr << "GetHandle: " << service << endl;
            continue;
        }

        xmlrpc_c::clientXmlTransport_pstream transport(
                xmlrpc_c::clientXmlTransport_pstream::constrOpt()
                .fd(handle));
        xmlrpc_c::client_xml client(&transport);
        xmlrpc_c::paramList params;

        string method;
        switch ( command ) {
        case 0:
            method = ServiceServer::ReleaseFile;
            {
                string pathname;
                cin >> pathname;
                params.add(xmlrpc_c::value_string(pathname));
            }
            break;
        case 1:
            method = ServiceServer::ReleaseInode;
            {
                string pathInode;
                cin >> pathInode;
                params.add(xmlrpc_c::value_string(pathInode));
            }
            break;
        case 2:
            method = ServiceServer::ReleaseTape;
            {
                string tape;
                cin >> tape;
                params.add(xmlrpc_c::value_string(tape));
            }
            break;
        case 3:
            method = ServiceServer::Import;
            {
                int action;
                string tape, pathTape, pathMeta;
                cin >> action >> tape >> pathTape;
                params.add(xmlrpc_c::value_int(action));
                params.add(xmlrpc_c::value_string(tape));
                params.add(xmlrpc_c::value_string(pathTape));
                params.add(xmlrpc_c::value_string(""));
            }
            break;
        case 4:
            method = ServiceServer::GetCacheCapacity;
            {
                params.add(xmlrpc_c::value_boolean(true));
            }
            break;
        case 5:
            method = ServiceServer::SetThrottle;
            {
                int interval;
                long long valve;
                cin >> interval >> valve;
                params.add(xmlrpc_c::value_int(interval));
                params.add(xmlrpc_c::value_i8(valve));
            }
            break;
        default:
            method = ServiceServer::ReleaseTape;
            params.add(xmlrpc_c::value_string("00000000"));
            break;
        }
        cout << "Method: " << method << endl;

        xmlrpc_c::rpc rpc(method,params);
        xmlrpc_c::carriageParm_pstream carriage;

        try {
            rpc.call(&client,&carriage);
        } catch ( std::exception const & e ) {
            LogError(e.what());
        }
        if ( ! rpc.isSuccessful() ) {
            xmlrpc_c::fault fault = rpc.getFault();
            LogError(fault.getCode() << ":" << fault.getDescription());
        } else {
            try {
                long long ret = xmlrpc_c::value_i8(rpc.getResult());
                cout << "Return: " << ret << endl;
            } catch ( std::exception const & e ) {
            }
            try {
                bool ret = xmlrpc_c::value_boolean(rpc.getResult());
                cout << "Return: " << (ret ? "true" : "false") << endl;
            } catch ( std::exception const & e ) {
            }
        }

        close(handle);
#endif
    }
    EventWarn("Service_Stop", "Stopping system service.");

//    purge.reset();

    TapeLibraryMgr::Instance()->PreventChangerMediaRemoval(false);
    LogInfo("TapeLibraryMgr::UnmountAllShare() started.");
    TapeLibraryMgr::Instance()->UnmountAllShare();
    LogInfo("TapeLibraryMgr::Destroy() started.");
    TapeLibraryMgr::Destroy();
    LogInfo("TapeLibraryMgr::Destroy() finished.");

    LogDebug("Stopping CacheMonitorServer");
    purge.reset(NULL);;
    LogDebug("Stopping ScheduleProxyServer");
    schedule.reset(NULL);
    LogDebug("Stopping TapeManagerProxyServer");
    tape.reset(NULL);

    return 0;
}
