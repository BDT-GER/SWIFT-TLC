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
 * Factory.cpp
 *
 *  Created on: Apr 11, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
//#include "TapeManagerLE.h"
//#include "TapeManagerSE.h"
#include "TapeManagerProxy.h"
#include "TapeManagerStop.h"
#include "MetaManager.h"
//#include "ScheduleTape.h"
//#include "SchedulePriorityTape.h"
#include "ScheduleProxy.h"
#include "CacheManager.h"
#include "ReadManager.h"

#ifdef MORE_TEST
#else
#include "BackupTapeTask.h"
#endif
//#include "CacheMonitorTask.h"


namespace bdt
{

    ofstream loggerDebug("/dev/zero",ios::out|ios::app);
    ofstream loggerInfo("/dev/zero",ios::out|ios::app);
    ofstream loggerWarn("/dev/zero",ios::out|ios::app);
    ofstream loggerError("/dev/zero",ios::out|ios::app);


    ofstream &
    LoggerDebug()
    {
        return loggerDebug;
    }


    ofstream &
    LoggerInfo()
    {
        return loggerInfo;
    }

    ofstream &
    LoggerWarn()
    {
        return loggerWarn;
    }


    ofstream &
    LoggerError()
    {
        return loggerError;
    }


    void
    CreateLogger(const fs::path & folderLogger)
    {
        if ( ! fs::exists(folderLogger) ) {
            fs::create_directory(folderLogger);
        }

        if ( ! fs::is_directory(folderLogger) ) {
            return;
        }

        fs::path logger;

        logger = folderLogger / "debug.log";
        loggerDebug.close();
        loggerDebug.open( logger.string().c_str(), ios::out|ios::app );

        logger = folderLogger / "info.log";
        loggerInfo.close();
        loggerInfo.open( logger.string().c_str(), ios::out|ios::app );

        logger = folderLogger / "warn.log";
        loggerWarn.close();
        loggerWarn.open( logger.string().c_str(), ios::out|ios::app );

        logger = folderLogger / "error.log";
        loggerError.close();
        loggerError.open( logger.string().c_str(), ios::out|ios::app );
    }


    Factory::Factory()
    {
    }


    Factory::~Factory()
    {
    }


    string Factory::service_;
    string Factory::name_;
    string Factory::uuid_;
    fs::path Factory::folderMeta_;
    fs::path Factory::folderCache_;
    fs::path Factory::folderTape_;
    auto_ptr<Configure> Factory::configure_;
    auto_ptr<Throttle> Factory::throttle_;
    auto_ptr<MetaManager> Factory::meta_;
    auto_ptr<TapeManagerInterface> Factory::tape_;
    auto_ptr<CacheManager> Factory::cache_;
    auto_ptr<ReadManager> Factory::read_;
    auto_ptr<ScheduleInterface> Factory::schedule_;
    auto_ptr<tape::TapeLibraryManager> Factory::changer_;

    vector<BackendTask *> Factory::tasks_;
    auto_ptr<boost::thread_group> Factory::taskGroup_;

    string const Factory::pathSocket_ = "/tmp/.socket.VS.";


    void
    Factory::CreateConfigure()
    {
        assert( NULL == configure_.get() );
        configure_.reset( new Configure() );
    }


    void
    Factory::CreateThrottle(int interval,long long valve)
    {
        assert( NULL == throttle_.get() );
        throttle_.reset( new Throttle(interval,valve) );
    }


    void
    Factory::CreateMetaManager()
    {
        assert( NULL == meta_.get() );
        meta_.reset( new MetaManager() );
        meta_->LoadHandlers();
    }


    void
    Factory::CreateTapeManager()
    {
        assert( NULL == tape_.get() );
        //tape_.reset( new TapeManagerLE() );
        //tape_.reset( new TapeManagerSE() );
        tape_.reset( new TapeManagerStop(new TapeManagerProxy()) );
    }


    void
    Factory::CreateCacheManager()
    {
        assert( NULL == cache_.get() );
        cache_.reset( new CacheManager() );
    }


    void
    Factory::CreateReadManager()
    {
        assert( NULL == read_.get() );
        read_.reset( new ReadManager() );
    }


    void
    Factory::CreateSchedule()
    {
        assert( NULL == schedule_.get() );
        //schedule_.reset( new ScheduleTape(10,500,3000) );
        //schedule_.reset( new SchedulePriorityTape() );
        schedule_.reset( new ScheduleProxy() );
    }


    void
    Factory::CreateTapeLibraryManager()
    {
        assert( NULL == changer_.get() );
#ifdef MORE_TEST
        changer_.reset( new tape::TapeLibraryManager() );
#endif
    }


    void
    Factory::StartBackendTasks()
    {
        if ( NULL != taskGroup_.get() ) {
            return;
        }

        taskGroup_.reset(new boost::thread_group());

#ifdef MORE_TEST
#else
        tasks_.push_back(new BackupTapeTask());
        taskGroup_->create_thread(
                boost::bind(&BackendTask::Start,tasks_[0]) );
#endif

//        tasks_.push_back(new CacheMonitorTask());
//        taskGroup_->create_thread(
//                boost::bind(&BackendTask::Start,tasks_[1]) );
    }


    void
    Factory::StopBackendTasks()
    {
        taskGroup_->interrupt_all();
        taskGroup_->join_all();
        taskGroup_.reset();

        for_each( tasks_.begin(), tasks_.end(),
                boost::bind(
                    operator delete,
                    _1 ) );
        tasks_.clear();
    }


    int
    Factory::SocketServerHandle(const string & service)
    {
        struct sockaddr_un addr;
        int handle = socket(AF_UNIX,SOCK_STREAM,0);
        if ( handle == -1 ) {
            LogError(service << " socket error");
            return handle;
        }
        memset(&addr,0,sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy( addr.sun_path,
                (pathSocket_ + service).c_str(),
                sizeof(addr.sun_path) - 1 );
        unlink(addr.sun_path);
        if ( -1 == ::bind( handle, (struct sockaddr *)&addr, sizeof(addr) ) ) {
            close(handle);
            LogError(service << " bind error");
            return -1;
        }
        if ( -1 == chmod(addr.sun_path,S_IRWXU) ) {
            close(handle);
            LogError(service << " chmod error");
            return -1;
        }
        if ( -1 == listen(handle,5) ) {
            close(handle);
            LogError(service << " listen error");
            return -1;
        }
        return handle;
    }


    int
    Factory::SocketClientHandle(const string & service)
    {
        int handle = socket(AF_UNIX, SOCK_STREAM, 0);
        if ( -1 == handle ) {
            LogError(service << " socket error");
            return -1;
        }
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy( addr.sun_path,
                (pathSocket_ + service).c_str(),
                sizeof(addr.sun_path) - 1 );
        if ( -1 == connect(handle, (struct sockaddr*)&addr, sizeof(addr)) ) {
            close(handle);
            LogError(service << " connect error");
            return -1;
        }
        return handle;
    }

    static inline bool
    GetRelativePath(
            const fs::path & path,
            const fs::path & folder,
            fs::path & pathRelative)
    {
        string namePath = path.string();
        string nameFolder = folder.string();
        if ( namePath.size() <= nameFolder.size() ) {
            return false;
        }
        //if ( namePath[nameFolder.size()] != fs::slash<char>::value ) {
        if ( namePath[nameFolder.size()] != '/' ) {
            return false;
        }
        if ( namePath.substr(0,nameFolder.size()) != nameFolder ) {
            return false;
        }
        pathRelative = namePath.substr(nameFolder.size());
        return true;
    }

    bool
    Factory::GetRelativePathFromCachePath(
            const fs::path & pathCache, fs::path & path)
    {
        return GetRelativePath(pathCache, folderCache_ / service_, path);
    }

    bool
    Factory::GetRelativePathFromMetaPath(
            const fs::path & pathMeta, fs::path & path)
    {
        return GetRelativePath(pathMeta, folderMeta_ / service_, path);
    }


    bool
    Factory::GetMetaPathFromCachePath(
            const fs::path & pathCache, fs::path & path)
    {
        fs::path pathRelative;
        if ( ! GetRelativePathFromCachePath(pathCache,pathRelative) ) {
            return false;
        }

        path = folderMeta_ / service_ / pathRelative;
        return true;
    }

    bool
    Factory::GetCachePathFromMetaPath(
            const fs::path & pathMeta, fs::path & path)
    {
        fs::path pathRelative;
        if ( ! GetRelativePathFromMetaPath(pathMeta,pathRelative) ) {
            return false;
        }

        path = folderCache_ / service_ / pathRelative;
        return true;
    }

}
