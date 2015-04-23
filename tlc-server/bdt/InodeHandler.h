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
 * InodeHandler.h
 *
 *  Created on: Nov 5, 2014
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class MetaDatabase;


    class InodeHandler
    {
    public:

        InodeHandler(const fs::path & path);

        ~InodeHandler();

        enum Action {
            ActionOpen,
            ActionWriteBegin,
            ActionWriteEnd,
            ActionClose,
            ActionDelete,
            ActionRename,
        };

        FileOperationInterface *
        GetFileOperation(int flags);

        bool InvokeAction(Action action, const string & parameter = "");

        enum IOAction {
            IOActionRead,
            IOActionWrite,
            IOActionTruncate,
            IOActionFlush,
            IOActionClose
        };

        bool InvokeIOAction(IOAction action, off_t offset, size_t size);

        bool NeedBackup(
                unsigned long long & number,
                off_t & size,
                boost::posix_time::ptime & time);

        bool InvokeBackup(
                FileOperationInterface * file,
                const string & tape,
                fs::path & path,
                bool & writeTape);

        bool GetStat(struct stat & stat);

        bool IsInUse();

        bool Handle();

        bool Persist();

    private:
        boost::mutex mutex_;
        boost::mutex mutexIO_;
        boost::mutex mutexBackup_;

        CacheManager * cache_;
        auto_ptr<MetaDatabase> database_;
        MetaManager * meta_;

        ReadManager * read_;
        bool checkRead_;
        bool beginRead_;

        /// the original path before rename
        fs::path pathOld_;
        /// the current path
        fs::path path_;

        /// cache file number
        unsigned long long number_;

        Inode::State state_;
        void SetState(Inode::State state);

        off_t size_;

        bool NeedBackup();

        bool RequireBackup();

        int refer_;
        int writting_;
        bool access_;
        boost::posix_time::ptime written_;
        bool needBackup_;

        auto_ptr<Inode> inode_;
    };
}
 
