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
 * CacheManager.cpp
 *
 *  Created on: Apr 10, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "CacheManager.h"
#include "FileOperation.h"
#include "FileOperationBitmap.h"


namespace bdt
{

    static const string numberFile = "number";

    CacheManager::CacheManager()
    : folder_(Factory::GetCacheFolder() / Factory::GetService()),
      state_(STATE_NORMAL)
    {
        if (fs::exists(folder_)) {
            if (fs::is_directory(folder_)) {
            } else {
                throw PathNotExistException(folder_.string());
            }
        } else {
            if ( ! fs::create_directory(folder_) ) {
                throw PathNotExistException(folder_.string());
            }
        }

        number_ = GetNumber(folder_,0);

        fs::path numberPath = folder_ / numberFile;
        ifstream file(numberPath.string().c_str());
        unsigned long long number = 0;
        file >> number;
        if ( number > number_ ) {
            number_ = number;
        }
    }


    CacheManager::~CacheManager()
    {
        fs::path numberPath = folder_ / numberFile;
        ofstream file(numberPath.string().c_str(),ios::out|ios::trunc);
        file << number_;
    }


    bool
    CacheManager::GetCapacity(off_t & usedCapacity,off_t & freeCapacity)
    {
        struct statfs stat;
        if ( 0 != statfs(folder_.string().c_str(), &stat) ) {
            LogWarn(folder_);
            return false;
        }

        freeCapacity = (off_t)stat.f_bsize * stat.f_bfree;
        usedCapacity = (off_t)stat.f_bsize
                * (stat.f_blocks - stat.f_bfree);
        return true;
    }


    FileOperationInterface *
    CacheManager::GetFileOperation(const unsigned long long number, int flags)
    {
        fs::path path;
        if ( ! GetPath(number,path) ) {
            errno = ENOENT;
            return NULL;
        }
        if ( ! fs::is_regular_file(path) ) {
            errno = ENOENT;
            return NULL;
        }
        try {
            return new FileOperation(path,flags);
        } catch ( const std::exception & e ) {
            LogWarn(path << " " << e.what());
            return NULL;
        }
    }


    FileOperationBitmap *
    CacheManager::GetFileOperationBitmap(
            const unsigned long long number, int flags)
    {
        static const int sizeBlock = Factory::GetConfigure()->GetValueSize(
                Configure::CacheFileSizeBlock );

        fs::path path;
        if ( ! GetPath(number,path) ) {
            errno = ENOENT;
            return NULL;
        }
        if ( ! fs::is_regular_file(path) ) {
            errno = ENOENT;
            return NULL;
        }
        try {
            return new FileOperationBitmap(path,flags,sizeBlock);
        } catch ( const std::exception & e ) {
            LogWarn(path << " " << e.what());
            return NULL;
        }
    }

    bool
    CacheManager::GetFileStat(const unsigned long long number, struct stat & stat)
    {
        fs::path path;
        if ( ! GetPath(number,path) ) {
            errno = ENOENT;
            return false;
        }
        return (0 == ::stat(path.string().c_str(),&stat));
    }


    bool
    CacheManager::CreateNewFile(unsigned long long & number)
    {
        boost::lock_guard<boost::mutex> lock_(mutex_);

        number = number_ + 1;
        fs::path path;
        if ( ! GetPath(number,path) ) {
            return false;
        }

        if ( ! fs::exists(path.parent_path()) ) {
            fs::create_directories(path.parent_path());
        }
        ofstream(path.string().c_str());
        ++ number_;
        return true;
    }


    bool
    CacheManager::CreateFile(const unsigned long long number)
    {
        fs::path path;
        if ( ! GetPath(number,path) ) {
            errno = ENOENT;
            return false;
        }

        if ( fs::exists(path) ) {
            errno = EEXIST;
            return false;
        }

        if ( ! fs::exists(path.parent_path()) ) {
            fs::create_directories(path.parent_path());
        }
        ofstream(path.string().c_str());

        if ( number > number_ ) {
            number_ = number;
        }
        return true;
    }


    bool
    CacheManager::DeleteFile(const unsigned long long number)
    {
        fs::path path;
        if ( ! GetPath(number,path) ) {
            return true;
        }
        if ( ! fs::exists(path) ) {
            return true;
        }
        try {
            fs::remove(path);
            return true;
        } catch ( const std::exception & e ) {
            LogWarn("Fail to remove " << path << ": " << e.what());
            return false;
        }
    }


    bool
    CacheManager::ExistFile(const unsigned long long number)
    {
        fs::path path;
        if ( ! GetPath(number,path) ) {
            return false;
        }
        return fs::exists(path);
    }


    bool
    CacheManager::IsFullFile(const unsigned long long number)
    {
        fs::path path;
        if ( ! GetPath(number,path) ) {
            return false;
        }

        if ( ! fs::is_regular_file(path) ) {
            return false;
        }

        ExtendedAttribute ea(path);
        int size;
        if ( ea.GetValue(Inode::ATTRIBUTE_BITMAP,NULL,0,size) ) {
            return false;
        } else {
            return true;
        }
    }


    bool
    CacheManager::GetIntValue(const string & name, int & value)
    {
        if (name.length() != 2) {
            return false;
        }
        value = 0;
        for ( int i = 0; i < 2; ++ i ) {
            value = value * 16;
            if ( name[i] >= '0' && name[i] <= '9' ) {
                value += name[i] - '0';
                continue;
            }
            if ( name[i] >= 'A' && name[i] <= 'F' ) {
                value += name[i] - 'A' + 10;
                continue;
            }
            if ( name[i] >= 'a' && name[i] <= 'f' ) {
                value += name[i] - 'a' + 10;
                continue;
            }
            return false;
        }
        return true;
    }

    unsigned long long
    CacheManager::GetNumber(const fs::path & folder, int level)
    {
        int maxValue = -1;
        string maxName;

        fs::directory_iterator end;
        for ( fs::directory_iterator i(folder); i != end; ++ i ) {
            if ( level < 7 ) {
                if ( fs::is_regular_file(i->status()) ) {
                    if ( i->path() == folder_ / numberFile ) {
                        continue;
                    }
                    LogWarn( "File " << i->path()
                            << " should not exist in cache");
                    continue;
                }
            } else {
                if ( fs::is_directory(i->status()) ) {
                    LogWarn( "Folder " << i->path()
                            << " should not exist in cache");
                    continue;
                }
            }
            string name = i->path().filename().string();
            int value = 0;
            if ( ! GetIntValue(name,value) ) {
                LogWarn( "Invalid cache item " << i->path() );
            }
            if ( value > maxValue ) {
                maxValue = value;
                maxName = name;
            }
        }

        if ( maxValue < 0 ) {
            return 0;
        }
        if ( level == 7 ) {
            return maxValue;
        } else {
            return maxValue * (1LL << (8 * (7 - level)))
                    + GetNumber(folder / maxName, level + 1);
        }
    }


    bool
    CacheManager::GetPath(const unsigned long long number, fs::path & path)
    {
        unsigned long long current = number;
        string result;
        for ( int i=0; i<8; ++i ) {
            int leaf = current & 0xff;
            current = current >> 8;
            ostringstream os;
            os << setw(2) << setfill('0') << hex << leaf;
            if ( result.empty() ) {
                result = os.str();
            } else {
                result = os.str() + "/" + result;
            }
        }
        path = folder_ / result;
        return true;
    }

}

