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
 * FileOperationCIFS.cpp
 *
 *  Created on: Apr 24, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "FileOperationCIFS.h"


namespace bdt
{

    off_t FileOperationCIFS::SizeInodeRead = 128 * 1024;
    off_t FileOperationCIFS::SizeInodeBegin = 896 * 1024;
    off_t FileOperationCIFS::SizeInodeEnd = 128 * 1024;
    off_t FileOperationCIFS::SizeInode = 1024 * 1024;


    FileOperationCIFS::FileOperationCIFS(
            Inode * inode,FileOperationInterface * oper,int wait)
    : FileOperationEntity(oper),
      ready_(false), inode_(inode),
      written_(false), wait_(wait), length_(-1)
    {
        assert( NULL != inode );
        assert( NULL != oper );
        if ( ! inode->GetSize(length_) ) {
            length_ = -1;
            LogWarn(inode->Path());
        }
    }


    FileOperationCIFS::~FileOperationCIFS()
    {
        if ( NULL != thread_.get() ) {
            thread_->interrupt();
            thread_->join();
        }

        struct stat stat;
        if ( FileOperationEntity::GetStat(stat) ) {
            if ( written_ ) {
                if ( stat.st_size != length_ ) {
                    LogError("Length " << length_
                            << " does not match file size " << stat.st_size);
                    length_ = stat.st_size;
                }
                inode_->SetSize(length_);
                if ( length_ > SizeInodeBegin ) {
                    off_t offset = max( static_cast<off_t>(SizeInodeBegin),
                            length_ - SizeInodeEnd );
                    size_t size = length_ - offset;
                    boost::scoped_array<char> buffer(new char[size]);
                    if ( FileOperationEntity::Read( offset,
                            buffer.get(), size, size ) ) {
                        if ( ! inode_->Write( SizeInodeBegin,
                                buffer.get(), size, size ) ) {
                            LogError(offset << ":" << size);
                        }
                    } else {
                        LogError(offset << ":" << size);
                    }
                }
            }
        }
    }


    bool
    FileOperationCIFS::Read(
            off_t offset,void * buffer,size_t bufsize,size_t & size)
    {
        if ( (off_t)(offset + bufsize) > SizeInodeRead ) {
            CheckReadReady(offset);
        }

        if ( ! IsInodeReadable() ) {
            CheckReadReady(offset);
            goto EntityRead;
        }

        if ( length_ <= SizeInode ) {
            goto InodeRead;
        }

        if ( ready_ ) {
            goto EntityRead;
        }

        if ( written_ ) {
            goto EntityRead;
        }

        if ( offset < SizeInodeBegin ) {
            if ( (off_t)(offset + bufsize) > SizeInodeBegin ) {
                goto EntityRead;
            } else {
                goto InodeRead;
            }
        } else if ( offset + SizeInodeEnd >= length_ ) {
            offset = SizeInode - (length_ - offset);
            goto InodeRead;
        } else {
            goto EntityRead;
        }


InodeRead:

        if ( ((off_t)(offset + bufsize) > SizeInodeRead) && (! ready_) ) {
            boost::unique_lock<boost::mutex> lock(mutex_);
            int timeout = wait_.GetWait(bufsize);
            for ( int wait = 0; (wait < timeout) && (!ready_); ++ wait ) {
                cond_.timed_wait(lock,boost::posix_time::seconds(1));
            }
            if ( !ready_ ) {
                wait_.SetWait(bufsize,timeout);
            }
        }

        return inode_->Read(offset,buffer,bufsize,size);


EntityRead:

        if ( ! ready_ ) {
            boost::unique_lock<boost::mutex> lock(mutex_);
            while ( !ready_ ) {
                cond_.wait(lock);
            }
        }

        return FileOperationEntity::Read(offset,buffer,bufsize,size);
    }


    bool
    FileOperationCIFS::Write(
            off_t offset,const void * buffer,size_t bufsize,size_t & size)
    {
        bool ret = FileOperationEntity::Write(offset,buffer,bufsize,size);
        if ( ! ret ) {
            return false;
        }

        length_ = max<off_t>(offset+size,length_);

        if ( offset < SizeInodeBegin ) {
            size_t sizeInode = std::min( bufsize,
                    static_cast<size_t>(SizeInodeBegin - offset) );
            if ( ! inode_->Write( offset, buffer, sizeInode, sizeInode ) ) {
                LogError(offset<<":"<<sizeInode)
                return false;
            }
        }

        if ( (off_t)(offset + size) > SizeInodeBegin ) {
            written_ = true;
        }

        return true;
    }


    bool
    FileOperationCIFS::Truncate(off_t length)
    {
        if ( ! FileOperationEntity::Truncate(length) ) {
            return false;
        }

        if ( length > SizeInodeBegin ) {
            if ( ! inode_->Truncate(SizeInodeBegin) ) {
                return false;
            }
        } else {
            if ( ! inode_->Truncate(length) ) {
                return false;
            }
        }

        if ( length > SizeInodeBegin ) {
            written_ = true;
        }

        length_ = length;

        return true;
    }


    bool
    FileOperationCIFS::CheckReadReady(off_t offset)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        if ( ready_ ) {
            if ( NULL != thread_.get() ) {
                thread_->join();
                thread_.reset();
            }
            return true;
        }

        if ( NULL != thread_.get() ) {
            return true;
        }

        thread_.reset( new boost::thread(
                &FileOperationCIFS::ReadReady, this, offset ) );

        return true;
    }


    void
    FileOperationCIFS::ReadReady(off_t offset)
    {
        try {
            char buffer[1024];
            size_t size;
            LogDebug(inode_->Path() << " " << offset);
            if ( FileOperationEntity::Read(
                    offset, buffer, sizeof(buffer), size ) ) {
                LogDebug(inode_->Path());
            } else {
                LogError(inode_->Path() << " " << offset);
            }
        } catch (const std::exception & e) {
            LogError(e.what());
        }

        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            ready_ = true;
        }
        cond_.notify_all();
    }

}
