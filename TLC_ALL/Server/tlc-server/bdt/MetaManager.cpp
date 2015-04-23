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
 * MetaManager.cpp
 *
 *  Created on: Feb 29, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "MetaManager.h"
#include "MetaDatabase.h"
#include "CacheManager.h"
#include "InodeHandler.h"
#include "FileOperation.h"


namespace bdt
{

    static int CheckInterval = 10;

    MetaManager::MetaManager()
    : folder_(Factory::GetMetaFolder() / Factory::GetService()),
      database_(new MetaDatabase()),
      cache_(Factory::GetCacheManager()),
      check_(boost::posix_time::second_clock::local_time())
    {
        if (fs::exists(folder_)) {
            if (fs::is_directory(folder_)) {
            } else {
                throw PathNotExistException(folder_.string());
            }
        } else {
            if ( ! fs::create_directory(folder_) ) {
                throw PathNotExistException(folder_.string());
            }
        }

        checkHandlersThread_.reset( new boost::thread(
                &MetaManager::CheckHandlersTask, this ) );
    }


    MetaManager::~MetaManager()
    {
        checkHandlersThread_->interrupt();
        checkHandlersThread_->join();

        SaveHandlers();
    }


    Inode *
    MetaManager::GetInode(const fs::path & path)
    {
        fs::path fullpath = folder_ / path;
        if (fs::exists(fullpath)) {
            return new Inode(fullpath);
        } else {
            return NULL;
        }
    }


    bool
    MetaManager::CreateFile(const fs::path & path, mode_t mode)
    {
        fs::path fullpath = folder_ / path;
        if (fs::exists(fullpath)) {
            errno = EEXIST;
            return false;
        } else {
            if ( ! CheckFreeCapacity() ) {
                errno = ENOSPC;
                return false;
            }
            int handle = ::creat(fullpath.string().c_str(),mode);
            if ( handle >= 0 ) {
                close(handle);
                return true;
            } else {
                return false;
            }
        }
    }


    bool
    MetaManager::CreateFolder(const fs::path & path, mode_t mode)
    {
        fs::path fullpath = folder_ / path;
        if (fs::exists(fullpath)) {
            errno = EEXIST;
            return false;
        } else {
            if ( ! CheckFreeCapacity() ) {
                errno = ENOSPC;
                return false;
            }
            return ( 0 == ::mkdir(fullpath.string().c_str(),mode) );
        }
    }


    bool
    MetaManager::DeleteInode(const fs::path & path)
    {
        fs::path pathname = folder_ / path;

        boost::lock_guard<boost::mutex> lock(mutex_);

        auto_ptr<Inode> inode(GetInode(path));

        if ( NULL == inode.get() ) {
            LogWarn(path << " is deleted already");
            return true;
        }

        if ( fs::is_directory(pathname) ) {
            bool ret = ( 0 == ::rmdir(pathname.string().c_str()) );
            if ( ! database_->DeleteFolder(path) ) {
                LogError(path << " fails to delete from database");
            }
            return ret;
        }

        unsigned long long number;
        if ( ! inode->GetNumber(number) ) {
            number = 0;
        }

        long backup;
        if ( ! inode->GetBackup(backup) ) {
            backup = 0;
        }

        if ( 0 != ::unlink(pathname.string().c_str()) ) {
            return false;
        }

        if ( backup ) {
            if ( ! database_->DeleteFile(number) ) {
                LogError(path << " fails to delete from database");
            }
        }

        MapHandlerType::iterator i = handlersOpen_.find(path);
        if ( i != handlersOpen_.end() ) {
            if ( ! i->second->InvokeAction(InodeHandler::ActionDelete) ) {
                LogWarn(path << " fails for delete action");
            }
            if ( i->second->Handle() ) {
                delete i->second;
            } else {
                handlersDelete_.push_back(i->second);
            }
            handlersOpen_.erase(i);
        } else {
            if ( ! cache_->DeleteFile(number) ) {
                LogError(path << " fails to be deleted");
            }
        }

        return true;
    }


    bool
    MetaManager::RenameInode(const fs::path & from, const fs::path & to)
    {
        fs::path pathFrom = folder_ / from;
        fs::path pathTo = folder_ / to;

        boost::lock_guard<boost::mutex> lock(mutex_);

        if ( 0 != ::rename(
                pathFrom.string().c_str(),
                pathTo.string().c_str() ) ) {
            return false;
        }

        if ( fs::is_directory(pathTo) ) {
            if ( ! database_->RenameFolder(from,to) ) {
                LogError(from << " fails to rename to " << to
                        << " in database");
            }
        } else {
            auto_ptr<Inode> inode( GetInode(to) );
            long backup;
            if ( inode->GetBackup(backup) && backup ) {
                unsigned long long number;
                if ( inode->GetNumber(number) ) {
                    if ( ! database_->RenameFile(number,to) ) {
                        LogError(number << " fails to rename to " << to
                            << " in database");
                    }
                }
            }
        }

        if ( fs::is_directory(pathTo) ) {
            string prefix = from.string() + "/";
            int prefixLen = prefix.size();
            MapHandlerType changes;
            vector<fs::path> deletes;
            BOOST_FOREACH(MapHandlerType::value_type & pair, handlersOpen_) {
                if ( pair.first.string().substr(0,prefixLen) != prefix ) {
                    continue;
                }
                fs::path pathNew = to / pair.first.string().substr(prefixLen);
                if ( ! pair.second->InvokeAction(
                        InodeHandler::ActionRename, pathNew.string() ) ) {
                    LogWarn(pair.first << " fails for rename action to "
                            << pathNew);
                }
                deletes.push_back(pair.first);
                changes.insert( MapHandlerType::value_type(
                        pathNew, pair.second ) );
            }
            BOOST_FOREACH(const fs::path & path, deletes) {
                handlersOpen_.erase(path);
            }
            BOOST_FOREACH(MapHandlerType::value_type & pair, changes) {
                handlersOpen_.insert( MapHandlerType::value_type(
                    pair.first, pair.second ) );
            }
            return true;
        } else {
            MapHandlerType::iterator i = handlersOpen_.find(to);
            if ( i != handlersOpen_.end() ) {
                if ( ! i->second->InvokeAction(InodeHandler::ActionDelete) ) {
                    LogWarn(to << " fails for delete action in rename");
                }
                if ( i->second->Handle() ) {
                    delete i->second;
                } else {
                    handlersDelete_.push_back(i->second);
                }
                handlersOpen_.erase(i);
            }

            i = handlersOpen_.find(from);
            if ( i == handlersOpen_.end() ) {
                return true;
            }
            if ( ! i->second->InvokeAction(
                    InodeHandler::ActionRename, to.string() ) ) {
                LogWarn(from << " fails for rename action to " << to);
            }
            InodeHandler * handler = i->second;
            handlersOpen_.erase(i);
            handlersOpen_.insert(MapHandlerType::value_type(to,handler));
            return true;
        }
    }


    bool
    MetaManager::GetActiveStat(const fs::path & path,struct stat & stat)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        auto_ptr<FileOperationInterface> file;

        MapHandlerType::iterator i = handlersOpen_.find(path);
        if ( i == handlersOpen_.end() ) {
            return false;
        }
        file.reset(i->second->GetFileOperation(O_RDONLY));
        return file->GetStat(stat);
    }

    FileOperationInterface *
    MetaManager::GetFileOperation(
            const fs::path & path,
            int flags)
    {
        auto_ptr<Inode> inode( GetInode(path) );
        if ( inode.get() == NULL ) {
            LogError(path << " does not exist");
            return NULL;
        }

        if ( ! inode->IsFile() ) {
            LogError(path << " is not a file");
            return NULL;
        }

        if ( database_->IsFilterFile(path) ) {
            try {
                return new FileOperation(inode->Path(),flags);
            } catch ( const std::exception & e ) {
                LogWarn(inode->Path() << " " << e.what());
                return NULL;
            }
        }

        boost::lock_guard<boost::mutex> lock(mutex_);

        MapHandlerType::iterator i = handlersOpen_.find(path);
        if ( i == handlersOpen_.end() ) {
            i = handlersOpen_.insert( MapHandlerType::value_type(
                    path, new InodeHandler(path) ) ).first;
        }
        return i->second->GetFileOperation(flags);
    }


    bool
    MetaManager::CheckFreeCapacity()
    {
        static off_t leastSize = Factory::GetConfigure()->GetValueSize(
                Configure::MetaFreeLeastSize );

        off_t usedCapacity,freeCapacity;
        if ( GetCapacity(usedCapacity,freeCapacity) ) {
            if ( freeCapacity < leastSize ) {
                LogWarn(freeCapacity);
                errno = ENOSPC;
                return false;
            } else {
                return true;
            }
        } else {
            errno = ENOSPC;
            return false;
        }
    }


    bool
    MetaManager::GetCapacity(off_t & usedCapacity, off_t & freeCapacity)
    {
        struct statfs stat;
        if ( 0 != statfs(folder_.string().c_str(),&stat) ) {
            LogError(folder_);
            return false;
        }

        freeCapacity = (off_t)stat.f_bsize * stat.f_bfree;
        usedCapacity = (off_t)stat.f_bsize
                * (stat.f_blocks - stat.f_bfree);
        return true;
    }


    void
    MetaManager::GetBackupList(vector<BackupItem> & list)
    {
        list.clear();

        boost::lock_guard<boost::mutex> lock(mutex_);

        vector<fs::path> removes;
        BOOST_FOREACH(MapHandlerType::value_type & pair, handlersOpen_) {
            if ( pair.second->Handle() ) {
                delete pair.second;
                removes.push_back(pair.first);
                continue;
            }
            BackupItem item;
            if ( pair.second->NeedBackup(item.number,item.size,item.time) ) {
                item.path = pair.first;
                list.push_back(item);
            }
        }
        BOOST_FOREACH(const fs::path & path, removes) {
            handlersOpen_.erase(path);
        }

        CheckHandlers(false);

        return;
    }


    bool
    MetaManager::Backup(
            const fs::path & path,
            FileOperationInterface * file,
            const string & tape,
            fs::path & pathNew,
            bool & writeTape)
    {
        boost::unique_lock<boost::mutex> lock(mutex_);

        MapHandlerType::iterator i = handlersOpen_.find(path);
        if ( i == handlersOpen_.end() ) {
            return false;
        }
        BackupItem item;
        if ( ! i->second->NeedBackup(item.number,item.size,item.time) ) {
            return false;
        }

        lock.unlock();

        return i->second->InvokeBackup(file,tape,pathNew,writeTape);
    }


    void
    MetaManager::CheckHandlers(bool checkOpenHandlers)
    {
        boost::posix_time::ptime now(
                boost::posix_time::second_clock::local_time() );
        if ( (now - check_).total_seconds() < CheckInterval ) {
            return;
        }

        for ( size_t i = 0; i < handlersDelete_.size(); ) {
            if ( handlersDelete_[i]->Handle() ) {
                delete handlersDelete_[i];
                handlersDelete_[i] = * handlersDelete_.rbegin();
                handlersDelete_.pop_back();
            } else {
                ++ i;
            }
        }

        if ( ! checkOpenHandlers ) {
            check_ = boost::posix_time::second_clock::local_time();
            return;
        }

        vector<fs::path> removes;
        BOOST_FOREACH(MapHandlerType::value_type & pair, handlersOpen_) {
            if ( pair.second->Handle() ) {
                delete pair.second;
                removes.push_back(pair.first);
            }
        }
        BOOST_FOREACH(const fs::path & path, removes) {
            handlersOpen_.erase(path);
        }

        check_ = boost::posix_time::second_clock::local_time();
        return;
    }


    void
    MetaManager::CheckHandlersTask()
    {
        while (true) {
            try {
                boost::this_thread::sleep( boost::posix_time::seconds(
                        CheckInterval ) );
                boost::lock_guard<boost::mutex> lock(mutex_);
                CheckHandlers(true);
            } catch ( const boost::thread_interrupted & e ) {
                break;
            }
        }
    }


    bool
    MetaManager::IsFileInUse(const fs::path & path)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        MapHandlerType::iterator i = handlersOpen_.find(path);

        if ( i == handlersOpen_.end() ) {
            return false;
        }

        return i->second->IsInUse();
    }


    bool
    MetaManager::ReleaseFile(const fs::path & path)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        MapHandlerType::iterator i = handlersOpen_.find(path);
        if ( i != handlersOpen_.end() ) {
            return false;
        }

        auto_ptr<Inode> inode(GetInode(path));
        if ( NULL == inode.get() ) {
            return false;
        }

        unsigned long long number;
        if ( ! inode->GetNumber(number) ) {
            return false;
        }

        return cache_->DeleteFile(number);
    }


    fs::path
    MetaManager::SaveHandlersPathname()
    {
        return folder_ / (Factory::GetService() + ".save");
    }


    bool
    MetaManager::SaveHandlers()
    {
        ofstream output(
                SaveHandlersPathname().string().c_str(),
                ios::out|ios::trunc);

        BOOST_FOREACH(const MapHandlerType::value_type & pair,handlersOpen_) {
            if ( pair.second->Persist() ) {
                output << pair.first.string() << endl;
            }
        }

        return true;
    }


    bool
    MetaManager::LoadHandlers()
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        if ( ! fs::is_regular_file(SaveHandlersPathname()) ) {
            ScanHandlers(folder_);
            return true;
        }

        ifstream input(SaveHandlersPathname().string().c_str());

        char pathBuffer[PATH_MAX];
        while ( input.getline(pathBuffer,sizeof(pathBuffer)) ) {
            fs::path path = pathBuffer;
            MapHandlerType::iterator i = handlersOpen_.find(path);
            if ( i != handlersOpen_.end() ) {
                LogWarn(path << " exists already");
                continue;
            }

            try {
                auto_ptr<InodeHandler> handler(new InodeHandler(path));
                handlersOpen_.insert( MapHandlerType::value_type(
                        path, handler.release() ) );
            } catch ( const std::exception & e ) {
                LogWarn(e.what());
            }
        }

        try {
            fs::remove(SaveHandlersPathname());
        } catch ( const std::exception & e ) {
            LogWarn(e.what());
        }

        return true;
    }


    void
    MetaManager::ScanHandlers(const fs::path & folder)
    {
        fs::directory_iterator end;
        for ( fs::directory_iterator i(folder); i != end; ++ i ) {
            if ( fs::is_directory(i->status()) ) {
                ScanHandlers(i->path());
                continue;
            }
            if ( ! fs::is_regular_file(i->status()) ) {
                continue;
            }
            const string pathFull = i->path().string();
            fs::path path = pathFull.substr(folder_.string().size());
            if ( ! database_->IsBackupFile(path) ) {
                continue;
            }
            auto_ptr<InodeHandler> handler;
            handler.reset(new InodeHandler(path));
            if ( ! handler->Handle() ) {
                handlersOpen_.insert( MapHandlerType::value_type(
                        path, handler.release() ) );
            }
        }
    }

}

