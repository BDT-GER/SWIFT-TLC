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
 * MetaDatabase.cpp
 *
 *  Created on: Dec 4, 2014
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "MetaDatabase.h"
#ifdef MORE_TEST
#else
#include "../ltfs_management/CatalogDbManager.h"
#include "../lib/common/Common.h"
#endif

#include <boost/regex.hpp>


namespace bdt
{

    MetaDatabase::MetaDatabase()
    {
#ifdef MORE_TEST
#else
        catalog_ = ltfs_management::CatalogDbManager::Instance();
#endif
    }


    MetaDatabase::~MetaDatabase()
    {
    }


    bool
    MetaDatabase::DeleteFile(const unsigned long long number)
    {
#ifdef MORE_TEST
        return true;
#else
        return catalog_->DeleteUuid(
                Factory::GetService(), boost::lexical_cast<string>(number) );

#endif
    }


    bool
    MetaDatabase::DeleteFolder(const fs::path & path)
    {
#ifdef MORE_TEST
        return true;
#else
        return catalog_->DeleteMetaFolder(
                Factory::GetService(), path.string() );
#endif
    }


    bool
    MetaDatabase::RenameFile(
            const unsigned long long number, const fs::path & path)
    {
#ifdef MORE_TEST
        return true;
#else
        return catalog_->RenameMetaFile(
                Factory::GetService(),
                boost::lexical_cast<string>(number),
                path.string() );
#endif
    }


    bool
    MetaDatabase::RenameFolder(const fs::path & from, const fs::path & to)
    {
#ifdef MORE_TEST
        return true;
#else
        return catalog_->RenameMetaFolder(
                Factory::GetService(),from.string(),to.string());
#endif
    }


    bool
    MetaDatabase::GetFileBackupInfo(
            const unsigned long long number,
            off_t & size)
    {
        string tape;
        fs::path path;
        return GetFileBackupInfo(number,size,tape,path);
    }


    bool
    MetaDatabase::GetFileBackupInfo(
            const unsigned long long number,
            const string & tape,
            fs::path & path)
    {
#ifdef MORE_TEST
        off_t size;
        string tapeBackup = tape;
        return GetFileBackupInfo(number,size,tapeBackup,path);
#else
        path = GetTapeFilePath(number,tape);
        return true;
#endif
    }


    bool
    MetaDatabase::GetFileBackupInfo(
            const unsigned long long number,
            off_t & size,
            string & tape,
            fs::path & path)
    {
#ifdef MORE_TEST
        size = 5 * 1024 * 1024;
        if ( tape.empty() ) {
            tape = "MoreTest";
        }
        path = "/dev/zero";
        return true;
#else
        map<string,ltfs_management::BackupInfo> infos;
        if ( ! catalog_->GetBackupInfo(
                Factory::GetService(),
                boost::lexical_cast<string>(number),
                infos ) ) {
            return false;
        }
        if ( infos.size() == 0 ) {
            return false;
        }
        map<string,ltfs_management::BackupInfo>::iterator i = infos.begin();
        if ( ! tape.empty() ) {
            i = infos.find(tape);
        }
        if ( i == infos.end() ) {
            i = infos.begin();
        }
        tape = i->first;
        path = i->second.mTapeFilePath;
        size = i->second.mSize;
        return true;
#endif
    }


    bool
    MetaDatabase::GetNextBackupFile(
            const unsigned long long number,
            const string & tape,
            unsigned long long & next,
            off_t & size)
    {
#ifdef MORE_TEST
        next = number + 1;
        size = 5 * 1024 * 1024;
        return true;
#else
        string path;
        string nextName;
        if ( ! catalog_->GetNextTapeFile(
                Factory::GetService(),
                tape,
                boost::lexical_cast<string>(number),
                size,
                nextName) ) {
            return false;
        }
        try {
            next = boost::lexical_cast<unsigned long long>(nextName);
        } catch ( const std::exception & e ) {
            return false;
        }
        return true;
#endif
    }


    bool
    MetaDatabase::IsFilterFile(const fs::path & path)
    {
        if(IsFilterFolder(path.parent_path())){
            return true;
        }

        static string filterFile = Factory::GetConfigure()->GetValue(
                Configure::FilterFilePattern );
        static boost::regex regFile(filterFile);

        if ( filterFile.empty() ) {
            return false;
        }

        return boost::regex_match(path.string(),regFile);
    }


    bool
    MetaDatabase::IsFilterFolder(const fs::path & path)
    {
        static string filterFolder = Factory::GetConfigure()->GetValue(
                Configure::FilterFolderPattern );
        static boost::regex regFolder(filterFolder);

        if ( filterFolder.empty() ) {
            return false;
        }

        return boost::regex_match(path.string(),regFolder);
    }


    bool
    MetaDatabase::IsBackupFile(const fs::path & path)
    {
        if(IsBackupFolder(path.parent_path())){
            return true;
        }

        static string backupFile = Factory::GetConfigure()->GetValue(
                Configure::BackupFilePattern );
        static boost::regex regFile(backupFile);

        if ( backupFile.empty() ) {
            return false;
        }

        return boost::regex_match(path.string(),regFile);
    }


    bool
    MetaDatabase::IsBackupFolder(const fs::path & path)
    {
        static string backupFolder = Factory::GetConfigure()->GetValue(
                Configure::BackupFolderPattern );
        static boost::regex regFolder(backupFolder);

        if ( backupFolder.empty() ) {
            return false;
        }

        return boost::regex_match(path.string(),regFolder);
    }

}

