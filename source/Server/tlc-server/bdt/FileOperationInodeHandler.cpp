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
 * FileOperationInodeHandler.cpp
 *
 *  Created on: Nov 25, 2014
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "FileOperationInodeHandler.h"
#include "InodeHandler.h"
#include "CacheManager.h"


namespace bdt
{

    FileOperationInodeHandler::FileOperationInodeHandler(
            InodeHandler * handler,
            FileOperationInterface * entity)
    : FileOperationEntity(entity),
      handler_(handler), write_(false), cache_(Factory::GetCacheManager())
    {
        handler_->InvokeAction(InodeHandler::ActionOpen);
    }


    FileOperationInodeHandler::~FileOperationInodeHandler()
    {
        if ( write_ ) {
            handler_->InvokeAction(InodeHandler::ActionWriteEnd);
        }

        struct stat stat;
        if ( GetStat(stat) ) {
            handler_->InvokeIOAction(
                    InodeHandler::IOActionClose, stat.st_size, 0);
        }

        handler_->InvokeAction(InodeHandler::ActionClose);
    }


    bool
    FileOperationInodeHandler::GetStat(struct stat & stat)
    {
        if ( ! handler_->GetStat(stat) ) {
            return FileOperationEntity::GetStat(stat);
        }
        struct stat statEntity;
        if ( ! FileOperationEntity::GetStat(statEntity) ) {
            return false;
        }
        stat.st_size = statEntity.st_size;
        return true;
    }


    bool
    FileOperationInodeHandler::Write(
            off_t offset, const void * buffer, size_t bufsize, size_t & size)
    {
        static off_t leastSize = Factory::GetConfigure()->GetValueSize(
                Configure::CacheFreeLeastSize );
        static off_t freeSize = Factory::GetConfigure()->GetValueSize(
                Configure::WriteCacheFreeSize );
    
        off_t usedCapacity,freeCapacity;
        if ( cache_->GetCapacity(usedCapacity,freeCapacity) ) {
            if ( freeCapacity < leastSize ) {
                errno = ENOSPC;
                return false;
            }
            if ( freeCapacity < freeSize ) {
                LogWarn("No enough free space to write: " << freeCapacity);
                boost::this_thread::sleep(boost::posix_time::seconds(3));
            } else {
                int state;
                if ( cache_->GetCacheState(state) ) {
                    if ( state == CacheManager::STATE_WRITE_CACHE_FULL ) {
                        LogWarn("Cache state is write cache full");
                        boost::this_thread::sleep(
                                boost::posix_time::seconds(3) );
                    }
                }
            }
        } else {
            errno = ENOSPC;
            return false;
        }

        if ( ! handler_->InvokeIOAction(
                InodeHandler::IOActionWrite, offset, bufsize) ) {
            return false;
        }

        if ( FileOperationEntity::Write(offset,buffer,bufsize,size) ) {
            if ( ! write_ ) {
                write_ = true;
                handler_->InvokeAction(InodeHandler::ActionWriteBegin);
            }
            return true;
        } else {
            return false;
        }
    }


    bool
    FileOperationInodeHandler::Read(
            off_t offset, void * buffer, size_t bufsize, size_t & size)
    {
        if ( ! handler_->InvokeIOAction(
                InodeHandler::IOActionRead, offset, bufsize) ) {
            return false;
        }

        return FileOperationEntity::Read(offset,buffer,bufsize,size);
    }


    bool
    FileOperationInodeHandler::Truncate(off_t length)
    {
        if ( ! handler_->InvokeIOAction(
                InodeHandler::IOActionTruncate, length, 0) ) {
            return false;
        }

        if ( FileOperationEntity::Truncate(length) ) {
            if ( ! write_ ) {
                write_ = true;
                handler_->InvokeAction(InodeHandler::ActionWriteBegin);
            }
            return true;
        } else {
            return false;
        }
    }


    bool
    FileOperationInodeHandler::Flush()
    {
        struct stat stat;
        if ( ! GetStat(stat) ) {
            return false;
        }
        return handler_->InvokeIOAction(
                InodeHandler::IOActionFlush, stat.st_size, 0 );
    }

}

