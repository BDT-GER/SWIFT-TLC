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
 * MetaDatabase.h
 *
 *  Created on: Dec 4, 2014
 *      Author: More Zeng
 */


#pragma once


namespace ltfs_management
{
    class CatalogDbManager;
};


namespace bdt
{

    class MetaDatabase
    {
    public:
        MetaDatabase();

        ~MetaDatabase();

        bool
        DeleteFile(const unsigned long long number);

        bool
        DeleteFolder(const fs::path & path);

        bool
        RenameFile(const unsigned long long number, const fs::path & path);

        bool
        RenameFolder(const fs::path & from, const fs::path & to);

        bool
        GetFileBackupInfo(const unsigned long long number, off_t & size);

        bool
        GetFileBackupInfo(
                const unsigned long long number,
                const string & tape,
                fs::path & path);

        bool
        GetFileBackupInfo(
                const unsigned long long number,
                off_t & size,
                string & tape,
                fs::path & path);

        bool
        GetNextBackupFile(
                const unsigned long long number,
                const string & tape,
                unsigned long long & next,
                off_t & size);

        bool
        IsFilterFile(const fs::path & path);

        bool
        IsFilterFolder(const fs::path & path);

        bool
        IsBackupFile(const fs::path & path);

        bool
        IsBackupFolder(const fs::path & path);

    private:
#ifdef MORE_TEST
#else
        ltfs_management::CatalogDbManager * catalog_;
#endif
    };

}

