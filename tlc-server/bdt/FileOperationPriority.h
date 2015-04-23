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
 * FileOperationPriority.h
 *
 *  Created on: Jul 25, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class FileOperationPriority : public FileOperationInterface
    {
    public:
        FileOperationPriority(
                const fs::path & path,
                int flags,
                const string & tape,
                int timeout,
                int priority);

        virtual
        ~FileOperationPriority();

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
        boost::mutex mutex_;

        auto_ptr<FileOperationInterface> file_;

        ScheduleInterface * schedule_;
        string tape_;

        TapeManagerInterface * tapeManager_;
        TapeManagerInterface::TapeAction action_;

        bool CheckRequest(
                TapeManagerInterface::TapeState state,
                int timeout,
                int priority);
        bool CheckAction(TapeManagerInterface::TapeAction action);

        void
        Close();
    };

}

