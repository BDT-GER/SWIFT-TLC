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
 * FileOperation.h
 *
 *  Created on: Mar 2, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class FileOperation : public FileOperationInterface
    {
    public:
        //  create
        FileOperation(const fs::path & path,mode_t mode,int flags);

        //  open
        FileOperation(const fs::path & path,int flags);

        virtual
        ~FileOperation();

        virtual bool
        GetStat(struct stat & stat);

        virtual bool
        Read(off_t offset,void * buffer,size_t bufsize,size_t & size);

        virtual bool
        Write(off_t offset,const void * buffer,size_t bufsize,size_t & size);

        virtual bool
        Truncate(off_t length);

        virtual bool
        Sync(bool data);

    private:
        int handle_;
    };

}

