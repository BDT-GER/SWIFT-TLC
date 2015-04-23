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
 * ReadManager.cpp
 *
 *  Created on: Dec 02, 2014
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "ReadManager.h"
#include "ReadTask.h"
#include "CacheManager.h"
#include "MetaDatabase.h"
#include "FileOperationBitmap.h"
#include "FileOperationPriority.h"
#include "FileOperationDelay.h"


namespace bdt
{

    int PreReadTimeout = 10 * 1000;


    ReadManager::ReadManager()
    : cache_(Factory::GetCacheManager()), database_(new MetaDatabase())
    {
        PreReadTimeout = Factory::GetConfigure()->GetValueSize(
              Configure::FileIdleTime );
        PreReadTimeout = (PreReadTimeout + 3) * 1000;
    }


    ReadManager::~ReadManager()
    {
        BOOST_FOREACH(ReadMap::value_type & pair, items_) {
            LogInfo(pair.first << " is cancelled for read");
            delete pair.second.task;
        }
        BOOST_FOREACH(PreReadMap::value_type & pair, preReadItems_) {
            LogInfo(pair.first << " is cancelled for pre-read");
            pair.second->interrupt();
            pair.second->join();
            delete pair.second;
        }
    }


    bool
    ReadManager::NeedRead(const unsigned long long number)
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        ReadMap::iterator i = items_.find(number);
        if ( i != items_.end() ) {
            if ( i->second.preRead ) {
                //  pre-read is ready
                return false;
            } else {
                LogWarn(number << " in read task already");
                return true;
            }
        }
        lock.unlock();

        if ( cache_->IsFullFile(number) ) {
            return false;
        } else {
            return true;
        }
    }


    bool
    ReadManager::ReadBegin(
            const unsigned long long number,
            string & tape,
            int timeout)
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        ReadMap::iterator i = items_.find(number);
        if ( i != items_.end() ) {
            LogWarn(number << " in read task already");
            return true;
        }
        lock.unlock();

        if ( ! ReadPrepare(number) ) {
            LogWarn(number << " fails to prepare cache file");
            return false;
        }

        fs::path path;
        if ( ! database_->GetFileBackupInfo(number,tape,path) ) {
            return false;
        }

        StopPreRead(tape);

        auto_ptr<FileOperationInterface> source;

#ifdef MORE_TEST
        source.reset( new FileOperationDelay(path,O_RDONLY,0,timeout,0));
#else
        try {
            source.reset( new FileOperationPriority(
                    path, O_RDONLY, tape, timeout,
                    ScheduleInterface::PRIORITY_READ ) );
        } catch ( const std::exception & e ) {
            LogInfo(number << " fails to schedule tape " << tape);
            return false;
        }
#endif

        lock.lock();
        auto_ptr<FileOperationBitmap> target;
        target.reset(cache_->GetFileOperationBitmap(number,O_WRONLY));
        if ( NULL == target.get() ) {
            LogWarn(number << " cannot be created in cache");
            return false;
        }
        ReadItem item;
        item.tape = tape;
        item.offset = -1;
        item.task = new ReadTask(
                number, source.release(), target.release(), this);
        item.preRead = false;
        items_.insert( ReadMap::value_type( number, item ) );
        return true;
    }


    bool
    ReadManager::CheckRead(
            const unsigned long long number, off_t offset, size_t size)
    {
        ReadTask * task = NULL;
        {
            boost::lock_guard<boost::mutex> lock(mutex_);

            ReadMap::iterator i = items_.find(number);
            if ( i == items_.end() ) {
                LogWarn(number << " is not in the read task");
                return true;
            }

            i->second.offset = offset;
            task = i->second.task;
        }
        return task->Prepare(offset,size);
    }


    bool
    ReadManager::CheckWrite(
            const unsigned long long number, off_t offset, size_t size)
    {
        return CheckRead(number,offset,size);
    }


    bool
    ReadManager::TruncateRead(const unsigned long long number,off_t length)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        ReadMap::iterator i = items_.find(number);
        if ( i == items_.end() ) {
            LogWarn(number << " is not in the read task");
            return true;
        }

        return i->second.task->Truncate(length);
    }


    bool
    ReadManager::ReadEnd(const unsigned long long number)
    {
        boost::unique_lock<boost::mutex> lock(mutex_);

        ReadMap::iterator i = items_.find(number);
        if ( i == items_.end() ) {
            LogWarn(number << " is not in the read task");
            return true;
        }

        auto_ptr<ReadTask> task(i->second.task);

        string tape = i->second.tape;
        off_t offset = i->second.offset;
        bool preRead = i->second.preRead;

        items_.erase(i);

        lock.unlock();

        task.reset();

        if ( ! preRead ) {
            StartPreRead(tape,number,offset);
        }

        return true;
    }


    bool
    ReadManager::ReadPrepare(const unsigned long long number, off_t length)
    {
        if ( cache_->ExistFile(number) ) {
            return true;
        }

        if ( ! cache_->CreateFile(number) ) {
            return false;
        }
        auto_ptr<FileOperationBitmap> file;
        file.reset(cache_->GetFileOperationBitmap(number,O_RDWR));
        if ( NULL == file.get() ) {
            cache_->DeleteFile(number);
            return false;
        }

        if ( length < 0 ) {
            if ( ! database_->GetFileBackupInfo(number,length) ) {
                return false;
            }
        }

        return file->TruncateBitmap(length);
    }


    void
    ReadManager::StartPreRead(
            const string & tape,
            const unsigned long long number,
            off_t offset)
    {
        if ( offset < 0 ) {
            return;
        }

        static const off_t readSize = Factory::GetConfigure()->GetValueSize(
                Configure::CacheFileSizeRead );
        if ( readSize <= 0 ) {
            return;
        }

        static const off_t freeSize = Factory::GetConfigure()->GetValueSize(
                Configure::ReadCacheFreeSize );
        off_t usedCapacity,freeCapacity;
        if ( ! cache_->GetCapacity(usedCapacity,freeCapacity) ) {
            return;
        }
        if ( freeCapacity <= freeSize + readSize ) {
            return;
        }

        boost::lock_guard<boost::mutex> lock(mutexPreRead_);

        PreReadMap::iterator i = preReadItems_.find(tape);
        if ( i != preReadItems_.end() ) {
            boost::this_thread::disable_interruption disable;
            if ( ! i->second->timed_join(boost::posix_time::seconds(1) ) ) {
                LogWarn(tape << " is in pre-read already");
                return;
            }
            delete i->second;
            preReadItems_.erase(i);
        }

        preReadItems_.insert( PreReadMap::value_type(
                tape,
                new boost::thread(
                    &ReadManager::PreReadTask,this,tape,number,offset) ) );
    }


    void
    ReadManager::StopPreRead(const string & tape)
    {
        boost::unique_lock<boost::mutex> lock(mutexPreRead_);

        PreReadMap::iterator i = preReadItems_.find(tape);
        if ( i == preReadItems_.end() ) {
            return;
        }
        boost::thread * thread = i->second;
        preReadItems_.erase(i);

        lock.unlock();

        thread->interrupt();
        thread->join();
        delete thread;
    }


    void
    ReadManager::PreReadTask(
            const string & tape,
            const unsigned long long number,
            off_t offset)
    {
        static const off_t freeSize = Factory::GetConfigure()->GetValueSize(
                Configure::ReadCacheFreeSize );

        off_t sizeRead = Factory::GetConfigure()->GetValueSize(
                Configure::CacheFileSizeRead );

        unsigned long long current = number;
        off_t length = -1;
        do {
            off_t usedCapacity,freeCapacity;
            if ( ! cache_->GetCapacity(usedCapacity,freeCapacity) ) {
                return;
            }
            if ( freeCapacity <= freeSize + sizeRead ) {
                return;
            }

            if ( boost::this_thread::interruption_requested() ) {
                LogInfo(current << " is interrupted for pre-read");
                return;
            }

            if ( NeedRead(current) ) {
                if ( ! ReadPrepare(current,length) ) {
                    LogWarn(current << " fails to prepare cache file");
                    return;
                }
                auto_ptr<FileOperationBitmap> file;
                file.reset(cache_->GetFileOperationBitmap(current,O_WRONLY));
                if ( NULL == file.get() ) {
                    LogWarn(current << " fails to create cache file");
                    return;
                }
                if ( ! file->GetBitmapLength(length) ) {
                    LogWarn(current << " fails to get bitmap length");
                    return;
                }
                length = min(length,offset+sizeRead);
                if ( length <= offset ) {
                    LogInfo(number << " finishes to pre-read");
                    return;
                }
                if ( ! PreRead(tape,current,file.get(),offset,length) ) {
                    return;
                }
                sizeRead = sizeRead - (length - offset);
            } else {
                struct stat stat;
                if ( ! cache_->GetFileStat(current,stat) ) {
                    LogInfo(current << " fails to stat cache file");
                    return;
                }
                if ( offset > stat.st_size ) {
                    offset = stat.st_size;
                }
                sizeRead = sizeRead - (stat.st_size - offset);
            }

            if ( ! database_->GetNextBackupFile(current,tape,current,length) ) {
                return;
            }
            offset = 0;
        } while ( sizeRead > 0 );
    }


    bool
    ReadManager::PreRead(
            const string & tape,
            const unsigned long long number,
            FileOperationBitmap * file,
            off_t offset,
            off_t offsetEnd)
    {
        off_t size;
        string tapeRead = tape;
        fs::path path;
        if ( ! database_->GetFileBackupInfo(number,size,tapeRead,path) ) {
            LogWarn(number << " fails to get backup info");
            return false;
        }
        if ( tapeRead != tape ) {
            LogWarn(number << " fails to get backup info for tape " << tape);
            return false;
        }

        if ( boost::this_thread::interruption_requested() ) {
            LogInfo(number << " is interrupted for pre-read");
            return false;
        }

        auto_ptr<FileOperationInterface> source;
#ifdef MORE_TEST
        source.reset( new FileOperationDelay(
                path,O_RDONLY,0,PreReadTimeout,0) );
#else
        try {
            source.reset( new FileOperationPriority(
                    path, O_RDONLY, tape, PreReadTimeout,
                    ScheduleInterface::PRIORITY_PREREAD ) );
        } catch ( const std::exception & e ) {
            LogInfo(number << " fails for pre-read " << path << ": " << e.what());
            return false;
        }
#endif

        if ( boost::this_thread::interruption_requested() ) {
            LogInfo(number << " is interrupted for pre-read");
            return false;
        }

        size_t sizeBlock;
        if ( ! file->GetBlockSize(sizeBlock) ) {
            LogWarn(number << " fails to get cache bitmap block size");
            return false;
        }

        const size_t bufsize = 512 * 1024;
        boost::scoped_array<char> buffer(new char[bufsize]());
        for ( off_t o = offset; o < offsetEnd; o += sizeBlock ) {
            size_t size = sizeBlock;
            if ( offsetEnd - o < (off_t)sizeBlock ) {
                size = (size_t)(offsetEnd - o);
            }
            if ( file->CheckBitmap(o, size) ) {
                continue;
            }
            for ( off_t i = o; i < o + (off_t)size; ) {
                if ( boost::this_thread::interruption_requested() ) {
                    LogInfo(number << " is interrupted for pre-read");
                    return false;
                }
                size_t sizeRead = min(size,bufsize);
                if ( ! source->Read(i,buffer.get(),sizeRead,sizeRead) ) {
                    LogWarn(number << " fails to read from tape at " << i);
                    return false;
                }
                if ( sizeRead == 0 ) {
                    LogWarn(number << " read zero data from tape at " << i);
                    return false;
                }
                if ( ! file->Write(i,buffer.get(),sizeRead,sizeRead) ) {
                    LogWarn(number << " fails to write at " << i);
                    return false;
                }
                i += sizeRead;
            }
        }

        LogInfo(number << " finishes to pre-read");
        return true;
    }


    void
    ReadManager::FinishReadTask(const unsigned long long number)
    {
        boost::unique_lock<boost::mutex> lock(mutex_);

        ReadMap::iterator i = items_.find(number);
        if ( i == items_.end() ) {
            return;
        }

        string tape = i->second.tape;
        off_t offset = i->second.offset;
        i->second.preRead = true;

        lock.unlock();

        StopPreRead(tape);
        StartPreRead(tape,number,offset);

        return;
    }

}

