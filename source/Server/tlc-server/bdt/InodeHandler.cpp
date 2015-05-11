/* Copyright (c) 2014 BDT Media Automation GmbH
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
 * InodeHandler.cpp
 *
 *  Created on: Nov 5, 2014
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "InodeHandler.h"
#include "CacheManager.h"
#include "MetaDatabase.h"
#include "MetaManager.h"
#include "ReadManager.h"
#include "FileOperationInodeHandler.h"


namespace bdt
{

    const int readTimeout = 60 * 60 * 1000;

 
    InodeHandler::InodeHandler(const fs::path & path)
    : cache_(Factory::GetCacheManager()),
      database_(new MetaDatabase()), meta_(Factory::GetMetaManager()),
      read_(Factory::GetReadManager()), checkRead_(false), beginRead_(false),
      pathOld_(path), path_(path), number_(0), state_(Inode::StateBegin),
      size_(0),
      refer_(0), writting_(0), access_(false),
      written_(boost::posix_time::microsec_clock::local_time()),
      needBackup_(database_->IsBackupFile(path_))
    {
        inode_.reset(meta_->GetInode(path_));
        if (inode_.get() == NULL) {
            LogError(path_ << " does not exist");
            throw PathNotExistException(path_.string());
        }

        if ( inode_->GetNumber(number_) ) {
            if ( read_->NeedRead(number_) ) {
                checkRead_ = true;
                beginRead_ = false;
            } else {
                checkRead_ = false;
            }
        } else {
            //  a new file
            if ( ! cache_->CreateNewFile(number_) ) {
                LogError(path_ << " fails to create the cache file");
                throw ExceptionFileNoSpace(path_.string());
            }
            inode_->SetNumber(number_);
            inode_->SetState(Inode::StateWrite);
        }

        long state;
        if ( ! inode_->GetState(state) ) {
            LogError(path_ << " does not have state");
            throw ExceptionFileIOError(path_.string());
        }
        state_ = (Inode::State)state;

        inode_->GetSize(size_);
    }


    InodeHandler::~InodeHandler()
    {
        if ( beginRead_ ) {
            read_->ReadEnd(number_);
            beginRead_ = false;
        }

        inode_->SetState(state_);

        struct stat statCache;
        if ( cache_->GetFileStat(number_,statCache) ) {
            inode_->SetSize(statCache.st_size);
        }

        struct stat stat;
        if ( inode_->GetStat(stat) ) {
            struct ::utimbuf times;
            times.actime = stat.st_atime;
            times.modtime = stat.st_mtime;
            if ( access_ ) {
                times.actime = max(::time(NULL),stat.st_atime);
                times.modtime = max(statCache.st_mtime,stat.st_mtime);
                ::utime( path_.string().c_str(), &times );
            }
            
        }
    }


    void
    InodeHandler::SetState(Inode::State state)
    {
        state_ = state;
        inode_->SetState(state);
    }


    FileOperationInterface *
    InodeHandler::GetFileOperation(int flags)
    {
        if ( checkRead_ ) {
            read_->ReadPrepare(number_,size_);
        }

        auto_ptr<FileOperationInterface> file;
        file.reset(cache_->GetFileOperation(number_,flags));
        if ( NULL == file.get() ) {
            return NULL;
        }
        return new FileOperationInodeHandler( this, file.release() );
    }


    bool
    InodeHandler::GetStat(struct stat & stat)
    {
        return inode_->GetStat(stat);
    }


    bool
    InodeHandler::NeedBackup(
            unsigned long long & number,
            off_t & size,
            boost::posix_time::ptime & time)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        if ( NeedBackup() ) {
            number = number_;
            size = size_;
            time = written_;
            return true;
        } else {
            return false;
        }
    }


    bool
    InodeHandler::InvokeBackup(
            FileOperationInterface * file,
            const string & tape,
            fs::path & path,
            bool & writeTape)
    {
        boost::lock_guard<boost::mutex> lock(mutexBackup_);

        writeTape = false;

        auto_ptr<FileOperationInterface> source;
        source.reset(cache_->GetFileOperation(number_,O_RDONLY));
        if ( NULL == source.get() ) {
            LogWarn(number_ << " does not exist in cache");
            return false;
        }

        const int bufsize = 512 * 1024;
        boost::scoped_array<char> buffer(new char[bufsize]);
        off_t offset = 0;
        do {
            {
                boost::lock_guard<boost::mutex> lock(mutex_);
                if ( ! NeedBackup() ) {
                    return false;
                }
            }
            size_t size;
            if ( ! source->Read(offset,buffer.get(),bufsize,size) ) {
                break;
            }
            if ( size == 0 ) {
                break;
            }
            writeTape = true;
            if ( ! file->Write(offset,buffer.get(),size,size) ) {
                return false;
            }
            offset += size;
        } while ( true );

        {
            boost::lock_guard<boost::mutex> lock(mutex_);

            if ( ! NeedBackup() ) {
                return false;
            }

            SetState(Inode::StateBegin);
            inode_->SetTape(tape);
            inode_->SetTapeSize(offset);
            path = path_;
            pathOld_ = path_;
            return true;
        }
    }


    bool
    InodeHandler::InvokeAction(Action action, const string & parameter)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        if ( action == ActionOpen ) {
            if ( state_ == Inode::StateDelete ) {
                LogWarn("Cannot open a deleted file " << path_);
                return false;
            }
            ++ refer_;
            return true;
        }

        if ( action == ActionClose ) {
            if ( refer_ <= 0 ) {
                LogWarn("Cannot close a closed file " << path_);
                return false;
            }
            -- refer_;
            return true;
        }

        if ( action == ActionDelete ) {
            if ( state_ == Inode::StateDelete ) {
                LogWarn("Cannot delete a deleted file " << path_);
                return false;
            }
            if ( number_ > 0 ) {
                if ( ! cache_->DeleteFile(number_) ) {
                    LogError(path_ << " fails to be deleted");
                }
            }
            SetState(Inode::StateDelete);
            return true;
        }

        if ( action == ActionWriteBegin ) {
            if ( state_ == Inode::StateDelete ) {
            } else {
                if ( state_ == Inode::StateBegin ) {
                    SetState(Inode::StateWrite);
                    if ( writting_ != 0 ) {
                        LogWarn(path_ << " write refer error: " << writting_);
                    }
                }
                if ( state_ != Inode::StateWrite ) {
                    LogWarn(path_ << " should be in write state: " << state_);
                    SetState(Inode::StateWrite);
                }
            }
            ++ writting_;
            return true;
        }

        if ( action == ActionWriteEnd ) {
            if ( writting_ <= 0 ) {
                LogWarn("Cannot stop writting on a no writting file " << path_);
                return false;
            }
            -- writting_;
            if ( writting_ == 0 ) {
                written_ = boost::posix_time::microsec_clock::local_time();
            }
            return true;
        }

        if ( action == ActionRename ) {
            path_  = parameter;
            inode_.reset(meta_->GetInode(path_));
            inode_->SetState(state_);
            inode_->SetSize(size_);
            needBackup_ = database_->IsBackupFile(path_);
            return true;
        }

        LogWarn("Action " << action << " for state " << state_
                << " on file " << path_);
        return false;
    }


    bool
    InodeHandler::InvokeIOAction(IOAction action, off_t offset, size_t size)
    {
        {
            boost::lock_guard<boost::mutex> lock(mutex_);

            if ( action == IOActionClose || action == IOActionFlush ) {
                size_ = offset + size;
                return inode_->SetSize(size_);
            }
        }

        boost::lock_guard<boost::mutex> lock(mutexIO_);

        if ( action == IOActionRead || action == IOActionWrite
                || action == IOActionTruncate ) {
            access_ = true;
        }

        if ( checkRead_ ) {
            if ( ! beginRead_ ) {
                string tape;
                if ( ! inode_->GetTape(tape) ) {
                    LogError(path_ << " fails to get tape");
                    return false;
                }
                beginRead_ = read_->ReadBegin(number_,tape,readTimeout);
            }
            if ( ! beginRead_ ) {
                return false;
            }
        }

        bool ret = true;

        if ( checkRead_ ) {
            switch (action) {
            case IOActionRead:
                ret = read_->CheckRead(number_,offset,size);
                break;
            case IOActionWrite:
                ret = read_->CheckWrite(number_,offset,size);
                break;
            case IOActionTruncate:
                ret = read_->TruncateRead(number_,offset+size);
                break;
            default:
                break;
            }
        }

        if ( ret ) {
            if ( action == IOActionTruncate ) {
                size_ = offset + size;
                inode_->SetSize(size_);
            }
    
//            const int increase = 1024;
//            if ( action == IOActionWrite ) {
//                if ( offset + (off_t)size >= size_ + increase ) {
//                    size_ = offset + size;
//                    inode_->SetSize(size_);
//                }
//            }
        }

        return ret;
    }


    bool
    InodeHandler::IsInUse()
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        return ( refer_ != 0 );
    }


    bool
    InodeHandler::Handle()
    {
        boost::unique_lock<boost::mutex> lockBackup(
                mutexBackup_, boost::try_to_lock );
        if ( ! lockBackup.owns_lock() ) {
            return false;
        }

        boost::lock_guard<boost::mutex> lock(mutex_);

        if ( refer_ != 0 ) {
            return false;
        }

        if ( writting_ != 0 ) {
            LogWarn(number_ << " writting count " << writting_);
            return false;
        }

        if ( state_ == Inode::StateBegin ) {
            return true;
        }

        if ( state_ == Inode::StateDelete ) {
            return true;
        }

        return ( ! RequireBackup() );
    }


    bool
    InodeHandler::NeedBackup()
    {
        static int waitFile = Factory::GetConfigure()->GetValueSize(
                Configure::BackupWaitFile );

        if ( writting_ != 0 ) {
            return false;
        }

        boost::posix_time::ptime now;
        now = boost::posix_time::second_clock::local_time();
        if ( (now - written_).total_seconds() < waitFile ) {
            return false;
        }

        return RequireBackup();
    }


    bool
    InodeHandler::RequireBackup()
    {
        if ( state_ != Inode::StateWrite ) {
            return false;
        }

        return needBackup_;
    }


    bool
    InodeHandler::Persist()
    {
        return RequireBackup();
    }

}

