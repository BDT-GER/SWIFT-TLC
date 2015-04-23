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
 * Factory.h
 *
 *  Created on: Apr 11, 2012
 *      Author: More Zeng
 */


#pragma once


#ifdef MORE_TEST

namespace bdt
{

    void
    CreateLogger(const fs::path & folderLogger);

    ofstream &
    LoggerDebug();

    ofstream &
    LoggerInfo();

    ofstream &
    LoggerWarn();

    ofstream &
    LoggerError();

    static boost::mutex mutexLogger;

#define LogIdent                                            \
        ( string(__PRETTY_FUNCTION__)                       \
            + ":" + boost::lexical_cast<string>(__LINE__)   \
            + ":" + boost::lexical_cast<string>(errno) )

#define LogTime                                             \
        boost::posix_time::to_simple_string(                \
            boost::posix_time::microsec_clock::local_time() )

#ifdef DEBUG

#define LogDebug(msg) \
        { \
            boost::lock_guard<boost::mutex> lock(mutexLogger); \
            string timestamp = LogTime; \
            LoggerDebug() << timestamp << " " << LogIdent << "\t" \
                << msg << endl; \
        }

#else

#define LogDebug(msg)

#endif

#define LogInfo(msg) \
        { \
            boost::lock_guard<boost::mutex> lock(mutexLogger); \
            string timestamp = LogTime; \
            LoggerDebug() << timestamp << " " << LogIdent << "\t" \
                << msg << endl; \
            LoggerInfo() << timestamp << " " << LogIdent << "\t" \
                << msg << endl; \
        }

#define LogWarn(msg) \
        { \
            boost::lock_guard<boost::mutex> lock(mutexLogger); \
            string timestamp = LogTime; \
            LoggerDebug() << timestamp << " " << LogIdent << "\t" \
                << msg << endl; \
            LoggerInfo() << timestamp << " " << LogIdent << "\t" \
                << msg << endl; \
            LoggerWarn() << timestamp << " " << LogIdent << "\t" \
                << msg << endl; \
        }

#define LogError(msg) \
        { \
            boost::lock_guard<boost::mutex> lock(mutexLogger); \
            string timestamp = LogTime; \
            LoggerDebug() << timestamp << " " << LogIdent << "\t" \
                << msg << endl; \
            LoggerInfo() << timestamp << " " << LogIdent << "\t" \
                << msg << endl; \
            LoggerWarn() << timestamp << " " << LogIdent << "\t" \
                << msg << endl; \
            LoggerError() << timestamp << " " << LogIdent << "\t" \
                << msg << endl; \
        }

#define EventInfo(eventID,msg)
#define EventWarn(eventID,msg)
#define EventError(eventID,msg)
#define EventCritical(eventID,msg)

}

#else

#include "../log/loggerManager.h"

using namespace ltfs_logger;

namespace bdt
{

string
GetName();

#define LogDebug(msg)   do {                                    \
                        int errnoOrigin = errno;                \
                        LgDebug("vfs", GetName()                \
                                << string(__PRETTY_FUNCTION__)  \
                                << " " << errno << " " << msg); \
                        errno = errnoOrigin;                    \
                        } while (false);
#define LogInfo(msg)    do {                                    \
                        int errnoOrigin = errno;                \
                        LgInfo("vfs", GetName()                 \
                                << " " << errno << " " << msg); \
                        errno = errnoOrigin;                    \
                        } while (false);
#define LogWarn(msg)    do {                                    \
                        int errnoOrigin = errno;                \
                        LgWarn("vfs", GetName()                 \
                                << " " << errno << " " << msg); \
                        errno = errnoOrigin;                    \
                        } while (false);
#define LogError(msg)   do {                                    \
                        int errnoOrigin = errno;                \
                        LgError("vfs", GetName()                \
                                << " " << errno << " " << msg); \
                        errno = errnoOrigin;                    \
                        } while (false);

#define EventInfo(eventId, msg)    CmnEvent("vfs", EVENT_LEVEL_INFO, eventId, msg)
#define EventWarn(eventId, msg)    CmnEvent("vfs", EVENT_LEVEL_WARNING, eventId, msg)
#define EventError(eventId, msg)   CmnEvent("vfs", EVENT_LEVEL_ERR, eventId, msg)
#define EventCritical(eventId, msg)   CmnEvent("vfs", EVENT_LEVEL_CRITICAL, eventId, msg)
}

#endif


#include "Exception.h"
#include "FileOperationInterface.h"
#include "Inode.h"
#include "Configure.h"
#include "Throttle.h"
#include "TapeManagerInterface.h"
#include "ScheduleInterface.h"
#include "../tape/TapeLibraryManager.h"

#include "BackendTask.h"


namespace bdt
{
    class CacheManager;
    class ReadManager;
    class MetaManager;
    class BackupTapeTask;


    class Factory
    {
    public:
        Factory();

        virtual
        ~Factory();


        static void
        StartBackendTasks();

        static void
        StopBackendTasks();


        static void
        SetService(const string & service)
        {
            service_ = service;
        }

        static string
        GetService()
        {
            return service_;
        }


        static void
        SetName(const string & name)
        {
            name_ = name;
        }

        static string
        GetName()
        {
            return name_;
        }

        static void
        SetUuid(const string & uuid)
        {
            uuid_ = uuid;
        }

        static string
        GetUuid()
        {
            return uuid_;
        }


        static void
        SetMetaFolder(const fs::path & folder)
        {
            folderMeta_ = folder;
        }

        static fs::path
        GetMetaFolder()
        {
            return folderMeta_;
        }


        static void
        SetCacheFolder(const fs::path & folder)
        {
            folderCache_ = folder;
        }

        static fs::path
        GetCacheFolder()
        {
            return folderCache_;
        }


        static void
        SetTapeFolder(const fs::path & folder)
        {
            folderTape_ = folder;
        }

        static fs::path
        GetTapeFolder()
        {
            return folderTape_;
        }


        static void
        CreateThrottle(int interval,long long valve);

        static void
        ReleaseThrottle();

        static Throttle *
        GetThrottle()
        {
            return throttle_.get();
        }


        static void
        CreateConfigure();

        static void
        ReleaseConfigure();

        static Configure *
        GetConfigure()
        {
            return configure_.get();
        }


        static void
        CreateMetaManager();

        static void
        ReleaseMetaManager();

        static MetaManager *
        GetMetaManager()
        {
            return meta_.get();
        }


        static void
        CreateTapeManager();

        static void
        ReleaseTapeManager();

        static void
        ResetTapeManager(TapeManagerInterface * tape)
        {
            tape_.reset(tape);
        }

        static TapeManagerInterface *
        GetTapeManager()
        {
            return tape_.get();
        }


        static void
        CreateCacheManager();

        static void
        ReleaseCacheManager();

        static CacheManager *
        GetCacheManager()
        {
            return cache_.get();
        }


        static void
        CreateReadManager();

        static void
        ReleaseReadManager();

        static ReadManager *
        GetReadManager()
        {
            return read_.get();
        }


        static void
        CreateSchedule();

        static void
        ReleaseSchedule();

        static void
        ResetSchedule(ScheduleInterface * schedule)
        {
            schedule_.reset(schedule);
        }

        static ScheduleInterface *
        GetSchedule()
        {
            return schedule_.get();
        }


        static void
        CreateTapeLibraryManager();

        static void
        ReleaseTapeLibraryManager();

        static void
        ResetTapeLibraryManager(tape::TapeLibraryManager * changer)
        {
            changer_.reset(changer);
        }

        static tape::TapeLibraryManager *
        GetTapeLibraryManager()
        {
            return changer_.get();
        }

        static BackupTapeTask * GetBackupTask()
        {
        	if(tasks_.size() > 0){
        		return (BackupTapeTask*)tasks_[0];
        	}
        	return NULL;
        }

        static int
        SocketServerHandle(const string & service);

        static int
        SocketClientHandle(const string & service);

        static bool
        GetRelativePathFromCachePath(
                const fs::path & pathCache, fs::path & path);

        static bool
        GetRelativePathFromMetaPath(
                const fs::path & pathMeta, fs::path & path);

        static bool
        GetMetaPathFromCachePath(const fs::path & pathCache, fs::path & path);

        static bool
        GetCachePathFromMetaPath(const fs::path & pathMeta, fs::path & path);

    private:
        static string service_;
        static string name_;
        static string uuid_;
        static fs::path folderMeta_;
        static fs::path folderCache_;
        static fs::path folderTape_;
        static auto_ptr<Configure> configure_;
        static auto_ptr<Throttle> throttle_;
        static auto_ptr<MetaManager> meta_;
        static auto_ptr<TapeManagerInterface> tape_;
        static auto_ptr<CacheManager> cache_;
        static auto_ptr<ReadManager> read_;
        static auto_ptr<ScheduleInterface> schedule_;
        static auto_ptr<tape::TapeLibraryManager> changer_;

        static vector<BackendTask *> tasks_;
        static auto_ptr<boost::thread_group> taskGroup_;

        static ofstream logDebug_;
        static ofstream logInfo_;
        static ofstream logWarn_;
        static ofstream logError_;
        static boost::mutex logMutex_;

        static string const pathSocket_;
    };

inline string
GetName()
{
    return Factory::GetName();
}

}
