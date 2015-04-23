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
 * FileOperationUnchange.h
 *
 *  Created on: Nov 27, 2012
 *      Author: More Zeng
 */


#pragma once


#include "FileOperationEntity.h"


namespace bdt
{

    class FileOperationUnchange : public FileOperationEntity
    {
    public:
        FileOperationUnchange(FileOperationInterface * oper)
        : FileOperationEntity(oper)
        {
        }

        virtual
        ~FileOperationUnchange()
        {
        }

        virtual bool
        CreateFile(int flag,mode_t mode,bool recur)
        {
            return false;
        }

        virtual bool
        OpenFile(int flag)
        {
            flag &= ~ O_ACCMODE;
            flag |= O_RDONLY;
            return FileOperationEntity::OpenFile(flag);
        }

        virtual bool
        Write(off_t offset,const void * buffer,size_t bufsize,size_t & size)
        {
            size = bufsize;
            return true;
        }

        virtual bool
        Truncate(off_t length)
        {
            return true;
        }

        virtual bool
        Delete()
        {
            return false;
        }

    };

}

