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
 * FileOperationZero.h
 *
 *  Created on: Nov 26, 2012
 *      Author: More Zeng
 */


#pragma once


#include "FileOperationInterface.h"


namespace bdt
{

    class FileOperationZero : public FileOperationInterface
    {
    public:
        FileOperationZero()
        : length_(0)
        {
        }

        virtual
        ~FileOperationZero()
        {
        }

        bool
        CreateFile(int flag,mode_t mode,bool recur)
        {
            return true;
        }

        bool
        OpenFile(int flag)
        {
            return true;
        }

        bool
        Read(off_t offset,void * buffer,size_t bufsize,size_t & size)
        {
            if ( length_ <= offset ) {
                size = 0;
            } else if ( length_ <= (off_t)(offset + bufsize) ) {
                size = length_ - offset;
            } else {
                size = bufsize;
            }
            return true;
        }

        bool
        Write(off_t offset,const void * buffer,size_t bufsize,size_t & size)
        {
            length_ = max<off_t>( offset + size, length_ );
            size = bufsize;
            return true;
        }

        bool
        GetStat(struct stat & stat)
        {
            int ret = ::stat("/dev/zero",&stat);
            assert(0 == ret);
            stat.st_size = length_;
            return true;
        }

        bool
        Sync(bool data)
        {
            return true;
        }

        bool
        Truncate(off_t length)
        {
            length_ = length;
            return true;
        }

    private:
        off_t length_;

    };

}

