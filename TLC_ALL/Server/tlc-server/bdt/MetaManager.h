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
 * MetaManager.h
 *
 *  Created on: Feb 29, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class InodeHandler;
    class MetaDatabase;


    struct BackupItem
    {
        fs::path path;
        unsigned long long number;
        off_t size;
        boost::posix_time::ptime time;
    };


    class MetaManager
    {
    public:
        MetaManager();

        ~MetaManager();

        Inode *
        GetInode(const fs::path & path);

        bool
        CreateFile(const fs::path & path,mode_t mode);

        bool
        CreateFolder(const fs::path & path,mode_t mode);

        bool
        DeleteInode(const fs::path & path);

        bool
        RenameInode(const fs::path & from, const fs::path & to);

        bool
        GetActiveStat(const fs::path & path,struct stat & stat);

        FileOperationInterface *
        GetFileOperation(const fs::path & path, int flags);

        bool
        CheckFreeCapacity();

        void
        GetBackupList(vector<BackupItem> & list);

        bool
        Backup(
                const fs::path & path,
                FileOperationInterface * file,
                const string & tape,
                fs::path & pathNew,
                bool & writeTape );

        bool
        IsFileInUse(const fs::path & path);

        bool
        ReleaseFile(const fs::path & path);

        bool
        LoadHandlers();

    private:
        boost::mutex mutex_;

        fs::path folder_;
        auto_ptr<MetaDatabase> database_;
        CacheManager * cache_;

        typedef map<fs::path, InodeHandler *> MapHandlerType;
        typedef vector<InodeHandler *> ListHandlerType;

        MapHandlerType handlersOpen_;
        ListHandlerType handlersDelete_;

        boost::posix_time::ptime check_;

        auto_ptr<boost::thread> checkHandlersThread_;

        void
        CheckHandlersTask();

        void
        CheckHandlers(bool checkOpenHandlers);

        bool
        GetCapacity(off_t & usedCapacity, off_t & freeCapacity);

        fs::path
        SaveHandlersPathname();

        bool
        SaveHandlers();

        void
        ScanHandlers(const fs::path & folder);
    };

}

