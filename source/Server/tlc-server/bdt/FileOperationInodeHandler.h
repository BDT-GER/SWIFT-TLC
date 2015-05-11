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
 * FileOperationInodeHandler.h
 *
 *  Created on: Nov 25, 2014
 *      Author: More Zeng
 */


#pragma once


#include "FileOperationEntity.h"


namespace bdt
{

    class InodeHandler;
    class CacheManager;


    class FileOperationInodeHandler : public FileOperationEntity
    {
    public:
        FileOperationInodeHandler(
                InodeHandler * handler,
                FileOperationInterface * entity);

        virtual
        ~FileOperationInodeHandler();

        bool
        GetStat(struct stat & stat);

        bool
        Read(off_t offset,void * buffer,size_t bufsize,size_t & size);

        bool
        Write(off_t offset,const void * buffer,size_t bufsize,size_t & size);

        bool
        Truncate(off_t length);

        bool
        Flush();

    private:
        InodeHandler * handler_;
        bool write_;
        CacheManager * cache_;
    };

}

