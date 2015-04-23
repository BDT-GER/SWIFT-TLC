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
 * FileOperationTape.h
 *
 *  Created on: Dec 14, 2012
 *      Author: More Zeng
 */


#pragma once


#include "FileOperation.h"


namespace bdt
{

    class TapeManagerStop;

    class FileOperationTape : public FileOperationInterface
    {
    public:
        FileOperationTape(
                const fs::path & path,
                mode_t mode,
                int flags,
                const string & tape);

        FileOperationTape(
                const fs::path & path,
                int flags,
                const string & tape);

        virtual
        ~FileOperationTape()
        {
        }

        bool
        GetStat(struct stat & stat);

        bool
        Read(off_t offset,void * buffer,size_t bufsize,size_t & size);

        bool
        Write(off_t offset,const void * buffer,size_t bufsize,size_t & size);

        bool
        Truncate(off_t length);

        bool
        Sync(bool data);

    private:
        auto_ptr<FileOperation> file_;
        fs::path path_;
        string tape_;
        TapeManagerStop * stop_;
        boost::posix_time::ptime timeEvent_;

        bool
        IsTapeStopped();

        void
        ReportEvent(const string & eventID,const string & message);

    };

}

