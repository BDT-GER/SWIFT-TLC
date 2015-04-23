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
 * InodeTapeStatus.h
 *
 *  Created on: Mar 30, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class InodeTapeStatus : public Inode
    {
    public:

        enum {
            FILE_SIZE = 1024,
        };

        InodeTapeStatus(const fs::path & pathname)
        : Inode(pathname)
        {
        }

        virtual
        ~InodeTapeStatus()
        {
        }

        off_t
        GetSize()
        {
            return FILE_SIZE;
        }

        bool
        SetSize(off_t size)
        {
            return false;
        }

        bool
        GetStat(struct stat & stat)
        {
            Inode::GetStat(stat);
            stat.st_mtime = time(NULL);
            stat.st_ino = 0;
            return true;
        }
    };

}

