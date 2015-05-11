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
 * Inode.h
 *
 *  Created on: Feb 29, 2012
 *      Author: More Zeng
 */


#pragma once


#include "ExtendedAttribute.h"
#include "FileDigest.h"


namespace bdt
{

    class Inode
    {
    public:
        Inode(const fs::path & path);

        Inode(int handle);

        ~Inode();

        fs::path
        Path()
        {
            return xattr_.GetPath();
        }


        static const string ATTRIBUTE_NUMBER;
        static const string ATTRIBUTE_TAPE;
        static const string ATTRIBUTE_TAPESIZE;
        static const string ATTRIBUTE_STATE;
        static const string ATTRIBUTE_BITMAP;
        static const string ATTRIBUTE_SIZE;
        static const string ATTRIBUTE_MD5;
        static const string ATTRIBUTE_SHA1;
        static const string ATTRIBUTE_BACKUP;
        static const string ATTRIBUTE_ONLINE;
        static const string ATTRIBUTE_CORRUPTED;


        bool
        GetSize(off_t & size);

        bool
        SetSize(off_t size);


        bool
        GetStat(struct stat & stat);

        bool
        IsFile()
        {
            return S_ISDIR(stat_.st_mode) ? false : true;
        }


        bool
        SetNumber(const unsigned long long number);

        bool
        GetNumber(unsigned long long & number);


        bool
        SetTape(const string & tape);

        bool
        GetTape(string & tape);

        bool
        SetTapeSize(const unsigned long long size);

        bool
        GetTapeSize(unsigned long long & size);


        enum State {
            StateBegin,
            StateWrite,
            StateDelete,
        };

        bool
        SetState(const long state);

        bool
        GetState(long & state);

        bool
        SetBackup(const long state);

        bool
        GetBackup(long & state);


        enum OnlineState {
            OnlineStateUnknown = 0,
            OnlineStateOnline = 1,
            OnlineStateOffline = 2,
        };

        bool
        GetOnline(OnlineState & state);


        bool
        SetDigest(FileDigest * digest);

        bool
        CheckDigest(FileDigest * digest);


        bool
        Read(off_t offset,void * buffer,size_t bufsize,size_t & size);

        bool
        Write(off_t offset,const void * buffer,size_t bufsize,size_t & size);

        bool
        Truncate(off_t length);


        bool
        GetExtendedAttribute(const string & name,
                void * buf,int bufsize,int & size);

        bool
        SetExtendedAttribute(const string & name,const void * buf,int size);

        bool
        ListExtendedAttribute(vector<string> & names);

        bool
        DeleteExtendedAttribute(const string & name);

    private:
        struct stat stat_;
        ExtendedAttribute xattr_;

        bool
        CheckExtendedAttribute(const string & name);
    };


    class FileTimeSwitch
    {
    public:
        FileTimeSwitch( const fs::path & path, struct ::utimbuf times )
        : path_(path), times_(times)
        {
        }

        ~FileTimeSwitch()
        {
            if ( 0 != ::utime( path_.string().c_str(), &times_ ) ) {
                LogError(path_);
            }
        }

    private:
        fs::path path_;
        struct ::utimbuf times_;
    };

}

