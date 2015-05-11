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
 * ReadTask.cpp
 *
 *  Created on: Dec 01, 2014
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "ReadTask.h"
#include "FileOperationBitmap.h"
#include "CacheManager.h"


namespace bdt
{

    static const int BUFFER_SIZE = 512 * 1024;


    ReadTask::ReadTask(
            const unsigned long long number,
            FileOperationInterface * source,
            FileOperationBitmap * file,
            ReadTaskCallback * callback)
    : number_(number), cache_(Factory::GetCacheManager()),
      source_(source), file_(file),
      running_(false), success_(false), current_(-1), errno_(0),
      callback_(callback)
    {
        thread_.reset( new boost::thread( &ReadTask::BackendTask, this ) );
        running_ = true;
    }

    ReadTask::~ReadTask()
    {
        thread_->interrupt();
        thread_->join();
    }


    bool
    ReadTask::Prepare(off_t offset, size_t size)
    {
        boost::unique_lock<boost::mutex> lock(mutex_);

        while ( running_ || success_ ) {
            if ( success_ ) {
                return true;
            }
            if ( file_->IsFull() ) {
                return true;
            }

            size_t sizeBlock;
            if ( ! file_->GetBlockSize(sizeBlock) ) {
                LogError(number_ << " cannot get block size");
                return false;
            }

            off_t length;
            if ( ! file_->GetBitmapLength(length) ) {
                LogError(number_ << " cannot get length");
                return false;
            }

            bool wait = false;
            off_t current = 0;
            for ( current = offset / sizeBlock * sizeBlock;
                     current < (off_t)(offset + size);
                    current += sizeBlock ) {
                if ( current >= length ) {
                    break;
                }
                int sizeCheck = min<int>(sizeBlock,offset+size-current);
                if ( (length - current) < sizeCheck ) {
                    sizeCheck = length - current;
                }
                if ( ! file_->CheckBitmap(current,sizeCheck) ) {
                    LogDebug(number_ << " check bitmap:" << current
                            << " size:" << sizeCheck);
                    queueWait_.push_back(current);
                    wait = true;
                }
            }
            if ( wait ) {
                condition_.wait(lock);
                continue;
            } else {
                if ( (current_ < 0) && queueWait_.empty() ) {
                    queueWait_.push_back(offset / sizeBlock * sizeBlock);
                }
                lock.unlock();
                return true;
            }
        }

        errno = errno_;

        return false;
    }


    bool
    ReadTask::Truncate(off_t length)
    {
        //lock is required, in case the file is writting during truncate
        boost::lock_guard<boost::mutex> lock(mutex_);

        off_t lengthOld;
        if ( ! file_->GetBitmapLength(lengthOld) ) {
            LogError(number_ << " cannot get bitmap length");
            return false;
        }

        if ( length < lengthOld ) {
            if ( file_->SetBitmapLength(length) ) {
                return true;
            } else {
                LogError(number_ << " cannot set bitmap length " << length);
                return false;
            }
        }
        return true;
    }


    bool
    ReadTask::IsValid()
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        if ( running_ ) {
            return true;
        }
        return success_;
    }


    bool
    ReadTask::IsRunning()
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        return running_;
    }


    bool
    ReadTask::CanWriteCache()
    {
        static off_t freeSize = Factory::GetConfigure()->GetValueSize(
                Configure::ReadCacheFreeSize );

        off_t usedCapacity,freeCapacity;
        if ( ! cache_->GetCapacity(usedCapacity,freeCapacity) ) {
            errno = EIO;
            return false;
        }

        if ( freeCapacity < freeSize ) {
            LogWarn("No enough free space: " << freeCapacity);
            errno = ENOSPC;
            return false;
        }

        return true;
    }


    void
    ReadTask::BackendTask()
    {
        success_ = false;
        errno_ = 0;

        off_t length;
        boost::scoped_array<char> buf;
        buf.reset(new char[BUFFER_SIZE]);

        if ( file_->IsFull() ) {
            goto BackendTaskFinish;
        }

        size_t sizeBlock;
        if ( ! file_->GetBlockSize(sizeBlock) ) {
            LogError(number_ << " has a corrupt bitmap");
            errno = EIO;
            goto BackendTaskError;
        }

        do {
            if ( boost::this_thread::interruption_requested() ) {
                LogWarn("Interrupt: " << number_);
                goto BackendTaskStop;
            }
            if ( ! CanWriteCache() ) {
                LogWarn("Cache is not writable: " << number_);
                goto BackendTaskError;
            }

            vector<off_t>::iterator i;
            {
                boost::lock_guard<boost::mutex> lock(mutex_);
                i = queueWait_.begin();
                if ( i != queueWait_.end() ) {
                    current_ = *i;
                    LogDebug(number_ << " " << current_);
                }
            }
            if ( current_ < 0 ) {
                boost::this_thread::yield();
                continue;
            }

            LogDebug(number_ << " " << current_);

            if ( boost::this_thread::interruption_requested() ) {
                LogWarn("Interrupt: " << number_);
                goto BackendTaskStop;
            }
            if ( ! CanWriteCache() ) {
                LogWarn("Cache is not writable: " << number_);
                goto BackendTaskError;
            }

            int sizeRead = sizeBlock;
            {
                boost::lock_guard<boost::mutex> lock(mutex_);
                file_->GetBitmapLength(length);
                if ( length <= current_ ) {
                    LogWarn(number_ << " " << length << " " << current_);
                    i = queueWait_.begin();
                    while ( i != queueWait_.end() ) {
                        if ( length <= *i ) {
                            LogWarn(number_ << " " << *i);
                            i = queueWait_.erase(i);
                        } else {
                            ++ i;
                        }
                    }
                    current_ = 0;
                    if ( file_->IsFull() ) {
                        goto BackendTaskFinish;
                    }
                    continue;
                }
                if ( (length - current_) < sizeRead ) {
                    sizeRead = length - current_;
                }
            }
            condition_.notify_all();

            if ( ! file_->CheckBitmap(current_,sizeRead) ) {
                LogDebug(number_ << " " << current_ << " " << sizeRead);
                size_t size;
                off_t begin = current_;
                while ( source_->Read(
                        begin, buf.get(), BUFFER_SIZE, size ) ) {
                    if ( size == 0 ) {
                        file_->GetBitmapLength(length);
                        if ( length > begin ) {
                            LogError(number_ << " " << length << " " << begin);
                            errno = EIO;
                            goto BackendTaskError;
                        } else {
                            LogWarn(number_ << " " << length << " " << begin);
                            break;
                        }
                    }
                    if ( boost::this_thread::interruption_requested() ) {
                        LogWarn("Interrupt: " << number_);
                        goto BackendTaskStop;
                    }
                    if ( ! CanWriteCache() ) {
                        LogWarn("Cache is not writable: " << number_);
                        goto BackendTaskError;
                    }
                    {
                        //lock is required,
                        //in case the file is truncated during writting
                        boost::lock_guard<boost::mutex> lock(mutex_);
                        file_->GetBitmapLength(length);
                        if ( length <= begin ) {
                            break;
                        }
                        if ( (length - begin) < (int)size ) {
                            size = length - begin;
                        }
                        if ( ! file_->Write(begin,buf.get(),size,size) ) {
                            LogError(number_ << ":" << begin << ":" << size);
                            goto BackendTaskError;
                        }
                    }
                    condition_.notify_all();
                    begin += size;
                    if ( begin >= current_ + sizeRead ) {
                        break;
                    }
                }
            }

            bool notify = false;
            {
                boost::unique_lock<boost::mutex> lock(mutex_);
                i = queueWait_.begin();
                while ( i != queueWait_.end() ) {
                    if ( current_ == *i ) {
                        i = queueWait_.erase(i);
                        notify = true;
                    } else {
                        ++ i;
                    }
                }
                if ( notify ) {
                    lock.unlock();
                    condition_.notify_all();
                }
            }

            current_ += sizeRead;
            file_->GetBitmapLength(length);
            if ( current_ >= length ) {
                if ( file_->IsFull() ) {
                    goto BackendTaskFinish;
                } else {
                    current_ = 0;
                }
            }
        } while (true);

        goto BackendTaskError;


BackendTaskFinish:

        LogDebug(number_ << " read success");

        if ( ! file_->IsFull() ) {
            LogWarn(number_);
            goto BackendTaskError;
        }

        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            running_ = false;
            success_ = file_->IsFull();
            source_.reset();
        }

        condition_.notify_all();

        if ( callback_ != NULL ) {
            callback_->FinishReadTask(number_);
        }

        return;


BackendTaskStop:

        LogDebug(number_ << " read stopped");

        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            running_ = false;
            success_ = false;
            errno_ = ECANCELED;
            source_.reset();
            file_.reset();
        }

        condition_.notify_all();

        return;


BackendTaskError:

        LogDebug(number_ << " read error");

        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            running_ = false;
            success_ = false;
            errno_ = errno;
            source_.reset();
            file_.reset();
        }

        condition_.notify_all();

        return;
    }


}


