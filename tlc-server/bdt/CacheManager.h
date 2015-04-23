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
 * CacheManager.h
 *
 *  Created on: Apr 10, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class FileOperationBitmap;

    class CacheManager
    {
    public:
        CacheManager();

        ~CacheManager();

        bool
        GetCapacity(off_t & usedCapacity, off_t & freeCapacity);

        bool
        CreateNewFile(unsigned long long & number);

        bool
        ExistFile(const unsigned long long number);

        bool
        CreateFile(const unsigned long long number);

        bool
        DeleteFile(const unsigned long long number);

        bool
        GetFileStat(const unsigned long long number, struct stat & stat);

        FileOperationInterface *
        GetFileOperation(const unsigned long long number, int flags);

        FileOperationBitmap *
        GetFileOperationBitmap(const unsigned long long number, int flags);

        bool
        IsFullFile(const unsigned long long number);

        enum {
            STATE_NORMAL = 0,
            STATE_WRITE_CACHE_FULL = 1,
        };

        bool
        GetCacheState(int & state)
        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            state = state_;
            return true;
        }

        bool
        SetCacheState(const int state)
        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            state_ = state;
            return true;
        }

    private:
        boost::mutex mutex_;

        fs::path folder_;

        volatile unsigned long long number_;

        volatile int state_;

        unsigned long long
        GetNumber(const fs::path & path, int level);

        bool
        GetIntValue(const string & name, int & value);

        bool
        GetPath(const unsigned long long number, fs::path & path);
    };

}

