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
 * Inode.cpp
 *
 *  Created on: Feb 29, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "CacheManager.h"

#include <attr/xattr.h>


namespace bdt
{

    const string Inode::ATTRIBUTE_NUMBER("user.vsvfs.number");
    const string Inode::ATTRIBUTE_TAPE("user.vsvfs.tape");
    const string Inode::ATTRIBUTE_TAPESIZE("user.vsvfs.tapesize");
    const string Inode::ATTRIBUTE_STATE("user.vsvfs.state");
    const string Inode::ATTRIBUTE_BITMAP("user.vsvfs.bitmap");
    const string Inode::ATTRIBUTE_SIZE("user.vsvfs.size");
    const string Inode::ATTRIBUTE_MD5("user.vsvfs.md5");
    const string Inode::ATTRIBUTE_SHA1("user.vsvfs.sha1");
    const string Inode::ATTRIBUTE_BACKUP("user.vsvfs.backup");
    const string Inode::ATTRIBUTE_CORRUPTED("user.vs.corrupted");
    const string Inode::ATTRIBUTE_ONLINE("user.vs.online");


    Inode::Inode(const fs::path & path)
    : xattr_(path)
    {
        memset( &stat_, 0, sizeof(stat_) );
        int ret = ::stat( path.string().c_str(), &stat_ );
        if ( 0 != ret ) {
            LogError(path);
            ::stat( "/dev/zero", &stat_ );
        }
    }


    Inode::Inode(int handle)
    : xattr_(handle)
    {
        memset( &stat_, 0, sizeof(stat_) );
        int ret = ::fstat( handle, &stat_ );
        if ( 0 != ret ) {
            LogError(Path());
            ::stat( "/dev/zero", &stat_ );
        }
    }


    Inode::~Inode()
    {
    }


    bool
    Inode::SetSize(off_t size)
    {
        if ( ! IsFile() ) {
            return false;
        }

        return xattr_.SetValue(ATTRIBUTE_SIZE,&size,sizeof(size));
    }


    bool
    Inode::GetStat(struct stat & stat)
    {
        stat = stat_;
        GetSize(stat.st_size);
        stat.st_blocks = (stat.st_size + 512 - 1) / 512;
        return true;
    }


    bool
    Inode::GetSize(off_t & size)
    {
        size = 0;

        if ( ! IsFile() ) {
            if ( 0 == ::stat( Path().string().c_str(), &stat_ ) ) {
                size = stat_.st_size;
                return true;
            } else {
                size = 0;
                return false;
            }
        }

        int valuesize;
        bool ret = xattr_.GetValue(
                ATTRIBUTE_SIZE, &size, sizeof(size), valuesize );
        if ( ret ) {
            return true;
        }

        if ( 0 == ::stat( Path().string().c_str(), &stat_ ) ) {
            size = stat_.st_size;
        } else {
            size = 0;
        }
        return false;
    }


    bool
    Inode::SetNumber(const unsigned long long number)
    {
        if ( ! IsFile() ) {
            return false;
        }

        return xattr_.SetValue(
                ATTRIBUTE_NUMBER, &number, sizeof(number) );
    }


    bool
    Inode::GetNumber(unsigned long long & number)
    {
        number = 0;

        if ( ! IsFile() ) {
            return false;
        }

        int valuesize;
        return xattr_.GetValue(
                ATTRIBUTE_NUMBER, &number, sizeof(number), valuesize );
    }


    bool
    Inode::SetTape(const string & tape)
    {
        if ( ! IsFile() ) {
            return false;
        }

        return xattr_.SetStringValue(ATTRIBUTE_TAPE, tape);
    }


    bool
    Inode::GetTape(string & tape)
    {
        if ( ! IsFile() ) {
            return false;
        }

        return xattr_.GetStringValue(ATTRIBUTE_TAPE, tape);
    }


    bool
    Inode::SetTapeSize(const unsigned long long size)
    {
        if ( ! IsFile() ) {
            return false;
        }

        return xattr_.SetValue(
                ATTRIBUTE_TAPESIZE, &size, sizeof(size) );
    }


    bool
    Inode::GetTapeSize(unsigned long long & size)
    {
        size = 0;

        if ( ! IsFile() ) {
            return false;
        }

        int valuesize;
        return xattr_.GetValue(
                ATTRIBUTE_TAPESIZE, &size, sizeof(size), valuesize );
    }


    bool
    Inode::SetState(const long state)
    {
        if ( ! IsFile() ) {
            return false;
        }

        return xattr_.SetValue(
                ATTRIBUTE_STATE, &state, sizeof(state) );
    }


    bool
    Inode::GetState(long & state)
    {
        state = 0;

        if ( ! IsFile() ) {
            return false;
        }

        int valuesize;
        return xattr_.GetValue(
                ATTRIBUTE_STATE, &state, sizeof(state), valuesize );
    }


    bool
    Inode::SetBackup(const long backup)
    {
        if ( ! IsFile() ) {
            return false;
        }

        return xattr_.SetValue(
                ATTRIBUTE_BACKUP, &backup, sizeof(backup) );
    }


    bool
    Inode::GetBackup(long & backup)
    {
        backup = 0;

        if ( ! IsFile() ) {
            return false;
        }

        int valuesize;
        return xattr_.GetValue(
                ATTRIBUTE_BACKUP, &backup, sizeof(backup), valuesize );
    }


    bool
    Inode::SetDigest(FileDigest * digest)
    {
        bool ret = true;
        string checksum;
        if ( digest->GetDigest(FileDigest::DIGEST_MD5,checksum) ) {
            ret = ret && xattr_.SetValue(
                    ATTRIBUTE_MD5, checksum.c_str(), checksum.size() );
        }
        if ( digest->GetDigest(FileDigest::DIGEST_SHA1,checksum) ) {
            ret = ret && xattr_.SetValue(
                    ATTRIBUTE_SHA1, checksum.c_str(), checksum.size() );
        }
        return ret;
    }


    bool
    Inode::CheckDigest(FileDigest * digest)
    {
        string checksum;
        char buffer[1024];
        int buflen;
        if ( digest->GetDigest(FileDigest::DIGEST_MD5,checksum) ) {
            if ( xattr_.GetValue(
                    ATTRIBUTE_MD5, buffer, sizeof(buffer), buflen ) ) {
                assert( buflen < static_cast<int>(sizeof(buffer)) );
                buffer[buflen] = '\0';
                if ( checksum != buffer ) {
                    return false;
                }
            }
        }
        if ( digest->GetDigest(FileDigest::DIGEST_SHA1,checksum) ) {
            if ( xattr_.GetValue(
                    ATTRIBUTE_SHA1, buffer, sizeof(buffer), buflen ) ) {
                assert( buflen < static_cast<int>(sizeof(buffer)) );
                buffer[buflen] = '\0';
                if ( checksum != buffer ) {
                    return false;
                }
            }
        }
        return true;
    }


    bool
    Inode::Read(off_t offset,void * buffer,size_t bufsize,size_t & size)
    {
        int handle = open( Path().string().c_str(), O_RDONLY );
        if ( handle < 0 ) {
            return false;
        }

        size = pread(handle,buffer,bufsize,offset);
        bool ret = (size >= 0) ? true : false;
        close(handle);
        return ret;
    }

    bool
    Inode::Write(off_t offset,const void * buffer,size_t bufsize,size_t & size)
    {
        int handle = open( Path().string().c_str(), O_WRONLY );
        if ( handle < 0 ) {
            return false;
        }

        size = pwrite(handle,buffer,bufsize,offset);
        bool ret = (size >= 0) ? true : false;
        close(handle);
        return ret;
    }


    bool
    Inode::Truncate(off_t length)
    {
        if ( 0 != truncate( Path().string().c_str(), length ) ) {
            return false;
        }

        return SetSize(length);
    }


    bool
    Inode::CheckExtendedAttribute(const string & name)
    {
        if ( name.substr(0,5) != "user." ) {
            return false;
        }
        if ( name.substr(0,11) == "user.vsvfs." ) {
            return false;
        }
        return true;
    }


    bool
    Inode::GetOnline(OnlineState & state)
    {
        unsigned long long number;
        if ( ! GetNumber(number) ) {
            state = OnlineStateOnline;
            return true;
        }

//        MetaManager * meta = Factory::GetMetaManager();
//        if ( meta == NULL ) {
//            state = OnlineStateUnknown;
//            return true;
//        }
//        string fullname = Path().string();
//        fs::path path = Factory::GetMetaFolder() / Factory::GetService();
//        string parent = path.string();
//        if ( fullname.find(parent) != 0 ) {
//            LogError(fullname << " is not a meta file pathname");
//            state = OnlineStateUnknown;
//            return true;
//        }
//        string name = fullname.substr(parent.size());
//        if ( meta->IsFileInUse(name) ) {
//            state = OnlineStateUnknown;
//            return true;
//        }

        CacheManager * cache = Factory::GetCacheManager();
        if ( cache == NULL ) {
            state = OnlineStateOffline;
            return true;
        }

        if ( cache->IsFullFile(number) ) {
            state = OnlineStateOnline;
        } else {
            state = OnlineStateOffline;
        }
        return true;
    }


    bool
    Inode::GetExtendedAttribute(
            const string & name, void * buf, int bufsize, int & size)
    {
        if ( name == ATTRIBUTE_ONLINE ) {
            OnlineState state = OnlineStateUnknown;
            if ( ! GetOnline(state) ) {
                errno = ENOATTR;
                return false;
            }
            if ( buf == NULL || bufsize == 0 ) {
                size = 8;
                return true;
            }
            if ( bufsize < 8 ) {
                errno = ERANGE;
                return false;
            }
            memset(buf,0,bufsize);
            * (int *)buf = state;
            size = 8;
            return true;
        }

        if ( ! CheckExtendedAttribute(name) ) {
            errno = ENOATTR;
            return false;
        }

        return xattr_.GetValue(name,buf,bufsize,size);
    }


    bool
    Inode::SetExtendedAttribute(const string & name,const void * buf,int size)
    {
        if ( ! CheckExtendedAttribute(name) ) {
            errno = ENOATTR;
            return false;
        }

        return xattr_.SetValue(name,buf,size);
    }


    bool
    Inode::ListExtendedAttribute(vector<string> & names)
    {
        names.clear();

        vector<string> attrs;
        bool ret = xattr_.GetNameList(attrs);
        for ( vector<string>::iterator i = attrs.begin();
                i != attrs.end();
                ++ i ) {
            if ( CheckExtendedAttribute(*i) ) {
                names.push_back(*i);
            }
        }

        return ret;
    }


    bool
    Inode::DeleteExtendedAttribute(const string & name)
    {
        if ( ! CheckExtendedAttribute(name) ) {
            errno = ENOATTR;
            return false;
        }

        return xattr_.DeleteName(name);
    }

}
