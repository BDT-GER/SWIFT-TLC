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
 * FileOperationDelay.cpp
 *
 *  Created on: Apr 24, 2012
 *      Author: More Zeng
 */

#include "stdafx.h"
#include "FileOperationDelay.h"


namespace bdt
{

    FileOperationDelay::FileOperationDelay(
            const fs::path & path,
            mode_t mode,
            int flags,
            int delayFirst,
            int delayRead,
            int delayWrite)
    : FileOperation(path,mode,flags),
      delayFirst_(delayFirst), delayRead_(delayRead), delayWrite_(delayWrite)
    {
    }


    FileOperationDelay::FileOperationDelay(
            const fs::path & path,
            int flags,
            int delayFirst,
            int delayRead,
            int delayWrite)
    : FileOperation(path,flags),
      delayFirst_(delayFirst), delayRead_(delayRead), delayWrite_(delayWrite)
    {
    }


    FileOperationDelay::~FileOperationDelay()
    {
    }


    bool
    FileOperationDelay::Read(
            off_t offset,void * buffer,size_t bufsize,size_t & size)
    {
        boost::this_thread::disable_interruption disable;

        if ( delayFirst_ > 0 ) {
            boost::this_thread::sleep(
                    boost::posix_time::milliseconds(delayFirst_) );
            delayFirst_ = 0;
        }

        if ( delayRead_ > 0 ) {
            boost::this_thread::sleep( boost::posix_time::milliseconds(
                    delayRead_ ) );
        }

        return FileOperation::Read(offset,buffer,bufsize,size);
    }


    bool
    FileOperationDelay::Write(
            off_t offset,const void * buffer,size_t bufsize,size_t & size)
    {
        boost::this_thread::disable_interruption disable;

        if ( delayFirst_ > 0 ) {
            boost::this_thread::sleep(
                    boost::posix_time::milliseconds(delayFirst_) );
            delayFirst_ = 0;
        }

        if ( delayWrite_ > 0 ) {
            boost::this_thread::sleep( boost::posix_time::milliseconds(
                    delayWrite_ ) );
        }

        return FileOperation::Write(offset,buffer,bufsize,size);
    }


}
