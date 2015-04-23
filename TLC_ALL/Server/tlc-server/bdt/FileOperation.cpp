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
 * FileOperation.cpp
 *
 *  Created on: Mar 2, 2012
 *      Author: More Zeng
 */

#include "stdafx.h"
#include "FileOperation.h"

#ifdef MORE_TEST
#else
#include "../lib/ltfs_library/HandleManager.h"
#endif


namespace bdt
{

    FileOperation::~FileOperation()
    {
        if ( handle_ >= 0 ) {
            ::close(handle_);
        }
        handle_ = -1;
    }


    FileOperation::FileOperation(const fs::path & path, mode_t mode, int flags)
    {
        handle_ = ::open(path.string().c_str(), O_CREAT|O_EXCL|flags, mode);
        if ( handle_ < 0 ) {
            switch (errno) {
            case EEXIST:
                throw ExceptionFileExistedAlready(path.string());
                break;
            case ENOSPC:
                throw ExceptionFileNoSpace(path.string());
            default:
                throw ExceptionFileIOError(path.string());
                break;
            }
        }

#ifdef MORE_TEST
#else
        ltfs_management::HandleManager::Bind(path.string(),handle_);
#endif
    }


    FileOperation::FileOperation(const fs::path & path, int flags)
    {
        handle_ = ::open(path.string().c_str(), flags);
        if ( handle_ < 0 ) {
            switch (errno) {
            case ENOENT:
                throw ExceptionFileNonExist(path.string());
                break;
            default:
                throw ExceptionFileIOError(path.string());
                break;
            }
        }

#ifdef MORE_TEST
#else
        ltfs_management::HandleManager::Bind(path.string(),handle_);
#endif
    }


    bool
    FileOperation::Read(
            off_t offset,void * buffer,size_t bufsize,size_t & size)
    {
        ssize_t ret = ::pread(handle_,buffer,bufsize,offset);
        if ( ret >= 0 ) {
            size = ret;
#ifdef MORE_TEST
            return true;
#else
            return ltfs_management::HandleManager::Read(
                    handle_, offset, buffer, size );
#endif
        } else {
            size = 0;
            return false;
        }
    }


    bool
    FileOperation::Write(
            off_t offset,const void * buffer,size_t bufsize,size_t & size)
    {
        ssize_t ret = ::pwrite(handle_,buffer,bufsize,offset);
        if ( ret >= 0 ) {
            size = ret;
#ifdef MORE_TEST
            return true;
#else
            return ltfs_management::HandleManager::Write(
                    handle_, offset, (void *)buffer, size );
#endif
        } else {
            size = 0;
            return false;
        }
    }


    bool
    FileOperation::GetStat(struct stat & stat)
    {
        int ret = ::fstat(handle_,&stat);
        return 0 == ret ? true : false;
    }


    bool
    FileOperation::Sync(bool data)
    {
        int ret = data ? ::fdatasync(handle_) : ::fsync(handle_);
        return ret == 0 ? true : false;
    }


    bool
    FileOperation::Truncate(off_t length)
    {
        int ret = ftruncate(handle_,length);
        return ret == 0 ? true : false;
    }

}

