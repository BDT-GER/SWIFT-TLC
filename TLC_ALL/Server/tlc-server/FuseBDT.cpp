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
 * FuseBDT.cpp
 *
 *  Created on: Mar 6, 2012
 *      Author: More Zeng
 */


#include <dirent.h>
#include "bdt/stdafx.h"
#include <attr/xattr.h>
#include "bdt/CacheManager.h"
#include "bdt/ReadManager.h"
#include "bdt/MetaManager.h"
#include "bdt/ServiceServer.h"
#include "bdt/FileOperationCIFS.h"
#include "ltfs_management/TapeLibraryMgr.h"

using namespace bdt;
using namespace ltfs_management;

#include "FuseBDT.h"

const time_t CREATE_CHECK_CAPACITY_INTERVAL = 1 * 60; // 1 minutes
const off_t MIN_CHECK_LAST_FREE_SIZE = 10LL * 1024 * 1024 * 1024; // 10G

FuseBDT::FuseBDT()
: meta_(NULL)
{
}


FuseBDT::~FuseBDT()
{
}


#define FuseReturn(ret,msg)             \
    LogDebug(ret << ":\t" << msg);      \
    return ret;
#define FuseReturnError(msg)            \
    do {                                \
        int ret = -errno;               \
        if ( ret == 0 ) {               \
            ret = - ENOENT;             \
        }                               \
        LogDebug(ret << ":\t" << msg);  \
        return ret;                     \
    } while (false);


int FuseBDT::getattr (const char * pathname, struct stat *stbuf)
{
    LogDebug ( pathname );

    try {
        if ( meta_->GetActiveStat(pathname, *stbuf) ) {
            FuseReturn(0,pathname);
        }
        auto_ptr<Inode> inode(meta_->GetInode(pathname));
        if ( inode.get() ) {
            if ( inode->GetStat(*stbuf) ) {
                //LogInfo(boost::lexical_cast<string>(stbuf->st_size));
                FuseReturn(0,pathname);
            }
        }
    } catch (const std::exception & e) {
        LogError(e.what());
    }

    FuseReturnError(pathname);
}


int FuseBDT::readlink (const char * pathname, char *buf, size_t size)
{
    errno = ENOTSUP;
    FuseReturnError(pathname);
}


int FuseBDT::mknod (const char * pathname, mode_t mode, dev_t rdev)
{
    errno = ENOTSUP;
    FuseReturnError(pathname);
}


int FuseBDT::mkdir (const char * pathname, mode_t mode)
{
    LogDebug ( pathname << "\tmode:0" << oct << mode << dec );
    mode = mode | 0777;

    try {
        fs::path path(pathname);
        auto_ptr<Inode> inode( meta_->GetInode(path) );
        if ( inode.get() ) {
            errno = EEXIST;
            FuseReturnError(pathname);
        }

        if ( meta_->CreateFolder(path,mode) ) {
            FuseReturn(0,pathname);
        } else {
            FuseReturnError(pathname);
        }
    } catch (const std::exception & e) {
        LogError(e.what());
    }

    FuseReturnError(pathname);
}


int FuseBDT::unlink (const char * pathname)
{
    LogDebug ( pathname );

    fs::path path(pathname);

    if ( meta_->DeleteInode(path) ) {
        FuseReturn(0,pathname);
    } else {
        FuseReturnError(pathname);
    }
}


int FuseBDT::rmdir (const char * pathname)
{
    LogDebug ( pathname );

    fs::path path(pathname);
    auto_ptr<Inode> inode( meta_->GetInode(path) );
    if ( NULL == inode.get() ) {
        FuseReturn(0,pathname);
    }

    if ( inode->IsFile() ) {
        errno = ENOTDIR;
        FuseReturnError(pathname);
    }

    fs::path pathInode = inode->Path();
    if ( ! fs::is_empty(pathInode) ) {
        errno = ENOTEMPTY;
        FuseReturnError(pathname);
    }

    if ( meta_->DeleteInode(path) ) {
        FuseReturn(0,pathname);
    } else {
        FuseReturnError(pathname);
    }
}


int FuseBDT::symlink (const char * pathname, const char *to)
{
    errno = ENOTSUP;
    FuseReturnError(pathname);
}


int FuseBDT::rename (const char * pathname, const char *to)
{
    LogDebug(pathname << " " << to);

    try {
        fs::path path(pathname);
        auto_ptr<Inode> inode( meta_->GetInode(path) );
        if ( NULL == inode.get() ) {
            errno = ENOENT;
            FuseReturnError(pathname);
        }

        if ( meta_->RenameInode(pathname,to) ) {
            FuseReturn(0,pathname);
        } else {
            FuseReturnError(pathname);
        }
    } catch (const std::exception & e) {
        LogError(e.what());
    }

    FuseReturnError(pathname);
}


int FuseBDT::link (const char * pathname, const char *to)
{
    errno = ENOTSUP;
    FuseReturnError(pathname);
}


int FuseBDT::chmod (const char * pathname, mode_t mode)
{
    LogError ( pathname << oct << " 0" << mode << dec );
    errno = EROFS;
    FuseReturnError(pathname);
}


int FuseBDT::chown (const char * pathname, uid_t uid, gid_t gid)
{
    LogError ( pathname << " " << uid << " " << gid );
    errno = EROFS;
    FuseReturnError(pathname);
}


int FuseBDT::truncate (const char * pathname, off_t size)
{
    LogDebug ( pathname << " " << size );

    static off_t maxSize = Factory::GetConfigure()->GetValueSize(
            Configure::FileMaxSize );
    if ( (maxSize > 0) && (size > maxSize) ) {
        errno = EFBIG;
        FuseReturnError(pathname);
    }

    auto_ptr<FileOperationInterface> file( OpenFile(pathname,O_WRONLY) );
    if ( NULL == file.get() ) {
        FuseReturnError(pathname);
    }

    if ( file->Truncate(size) ) {
        FuseReturn(0,pathname);
    } else {
        FuseReturnError(pathname);
    }
}


int FuseBDT::utime (const char * pathname, struct utimbuf *buf)
{
    LogDebug ( pathname );

    fs::path path(pathname);
    auto_ptr<Inode> inode( meta_->GetInode(path) );
    if ( NULL == inode.get() ) {
        errno = ENOENT;
        FuseReturnError(pathname);
    }

    if ( 0 == ::utime( inode->Path().string().c_str(), buf ) ) {
        FuseReturn(0,pathname);
    } else {
        FuseReturnError(pathname);
    }
}


FileOperationInterface *
FuseBDT::OpenFile(const char * pathname, int flags)
{
    LogDebug ( pathname << oct << " 0" << flags << dec );

    try {
        fs::path path(pathname);
        auto_ptr<Inode> inode( meta_->GetInode(path) );
        if ( NULL == inode.get() ) {
            errno = ENOENT;
            return NULL;
        }
        if ( ! inode->IsFile() ) {
            errno = EISDIR;
            return NULL;
        }

        return meta_->GetFileOperation(path,flags);
    } catch (const std::exception & e) {
        LogError(e.what());
    }

    return NULL;
}

int FuseBDT::open (const char * pathname, struct fuse_file_info * info)
{
    LogDebug ( pathname );

    FileOperationInterface * interface = OpenFile(pathname,info->flags);
    if ( NULL == interface ) {
        FuseReturnError(pathname);
    }

    info->fh = reinterpret_cast<uint64_t>(interface);

    FuseReturn(0,pathname);
}


int FuseBDT::read (
        const char * pathname,
        char *buf,
        size_t size,
        off_t offset,
        struct fuse_file_info *info )
{
#ifdef DEBUG
    LogDebug ( pathname << " offset: " << offset << " size: " << size );
#endif

    FileOperationInterface * file
            = reinterpret_cast<FileOperationInterface *>(info->fh);
    size_t sizeRead;
    if ( file->Read(offset,buf,size,sizeRead) ) {
#ifdef DEBUG
        FuseReturn(sizeRead,pathname);
#else
        return sizeRead;
#endif
    } else {
        FuseReturnError(pathname);
    }
}


int FuseBDT::write (
        const char * pathname,
        const char * buf,
        size_t size,
        off_t offset,
        struct fuse_file_info * info )
{
#ifdef DEBUG
    LogDebug ( pathname << " offset: " << offset << " size: " << size );
#endif

    static off_t maxSize = Factory::GetConfigure()->GetValueSize(
            Configure::FileMaxSize );
    if ( (maxSize > 0) && ((off_t)(offset + size) > maxSize) ) {
        errno = EFBIG;
        FuseReturnError(pathname);
    }

    FileOperationInterface * file
            = reinterpret_cast<FileOperationInterface *>(info->fh);

//    string tape;
//    file->GetTape(tape);
//    boost::lock_guard<boost::mutex> lock(*GetMutex(tape));

    Factory::GetThrottle()->Request(size);

    size_t sizeWrite;
    if ( file->Write(offset,buf,size,sizeWrite) ) {
#ifdef DEBUG
        FuseReturn(sizeWrite,pathname);
#else
        return sizeWrite;
#endif
    } else {
        FuseReturnError(pathname);
    }
}


int FuseBDT::statfs (const char * pathname, struct statvfs *stbuf)
{
    stbuf->f_bsize = 512 * 1024;
    stbuf->f_frsize = stbuf->f_bsize;
    stbuf->f_flag = 0;
    stbuf->f_fsid = 0;
    stbuf->f_namemax = PATH_MAX;
    stbuf->f_blocks = 1024 * 1024;
    stbuf->f_bavail = 512 * 1024;
    stbuf->f_bfree = 512 * 1024;
    stbuf->f_files = 1024 * 1024;
    stbuf->f_favail = 512 * 1024;
    stbuf->f_ffree = 512 * 1024;

    off_t usedSize,freeSize;
    size_t fileNumber;
    if ( TapeLibraryMgr::GetTapeGroupCapacity(
            Factory::GetService(), fileNumber, usedSize, freeSize ) ) {
        off_t backupSize = 0;
        vector<BackupItem> items;
        meta_->GetBackupList(items);
        BOOST_FOREACH(const BackupItem & item, items) {
            backupSize += item.size;
        }
        usedSize += backupSize;
        freeSize -= backupSize;
        if ( freeSize < 0 ) {
            freeSize = 0;
        }
        stbuf->f_blocks = (usedSize + freeSize) / stbuf->f_bsize;
        stbuf->f_bavail = freeSize / stbuf->f_bsize;
        stbuf->f_bfree = stbuf->f_bavail;
        if ( stbuf->f_files <= fileNumber ) {
            stbuf->f_favail = 0;
            stbuf->f_bfree = 0;
        } else {
            stbuf->f_favail = stbuf->f_files - fileNumber;
            stbuf->f_ffree = stbuf->f_favail;
        }
    }

    FuseReturn(0,pathname);
}


int FuseBDT::flush (const char * pathname, struct fuse_file_info *info)
{
    FileOperationInterface * file
        = reinterpret_cast<FileOperationInterface *>(info->fh);
    if ( file->Flush() ) {
        FuseReturn(0,pathname);
    } else {
        FuseReturnError(pathname);
    }
}


int FuseBDT::release (const char * pathname, struct fuse_file_info * info)
{
    LogDebug ( pathname );

    FileOperationInterface * file
            = reinterpret_cast<FileOperationInterface *>(info->fh);

    delete file;

    FuseReturn(0,pathname);
}


int
FuseBDT::fsync (const char * pathname, int mode, struct fuse_file_info *info)
{
    LogDebug ( pathname << "\tmode:" << mode );

    FileOperationInterface * file
            = reinterpret_cast<FileOperationInterface *>(info->fh);

    if ( !file->Sync(mode?true:false) ) {
        FuseReturnError(pathname);
    } else {
        FuseReturn(0,pathname);
    }
}


int FuseBDT::setxattr (
        const char * pathname,
        const char *name,
        const char *value,
        size_t size,
        int flags )
{
    LogDebug ( pathname << " " << name );

    try {
        auto_ptr<Inode> inode(meta_->GetInode(pathname));
        if ( inode.get() ) {
            if ( flags != 0 ) {
                int ret;
                if ( inode->GetExtendedAttribute(name,NULL,0,ret) ) {
                    if ( flags == XATTR_CREATE ) {
                        errno = EEXIST;
                        FuseReturnError(pathname);
                    }
                } else {
                    if ( flags == XATTR_REPLACE ) {
                        errno = ENOATTR;
                        FuseReturnError(pathname);
                    }
                }
            }
            if ( inode->SetExtendedAttribute(name,value,size) ) {
                FuseReturn(0,pathname);
            } else {
                FuseReturnError(pathname);
            }
        }
    } catch (const std::exception & e) {
        LogError(e.what());
    }

    FuseReturnError(pathname);
}


int FuseBDT::getxattr (
        const char * pathname,
        const char *name,
        char *value,
        size_t size )
{
    LogDebug ( pathname << " " << name );

    try {
        auto_ptr<Inode> inode(meta_->GetInode(pathname));
        if ( inode.get() ) {
            int ret;
            if ( inode->GetExtendedAttribute(name,value,size,ret) ) {
                FuseReturn(ret,pathname);
            } else {
                FuseReturnError(pathname);
            }
        }
    } catch (const std::exception & e) {
        LogError(e.what());
    }

    FuseReturnError(pathname);
}


int FuseBDT::listxattr (const char * pathname, char *list, size_t size)
{
    LogDebug ( pathname << " " << size);

    try {
        auto_ptr<Inode> inode(meta_->GetInode(pathname));
        if ( inode.get() ) {
            vector<string> names;
            if ( ! inode->ListExtendedAttribute(names) ) {
                FuseReturnError(pathname);
            }
            size_t ret = 0;
            for ( vector<string>::iterator i = names.begin();
                    i != names.end();
                    ++ i ) {
                ret = ret + i->size() + 1;
            }
            if ( size == 0 ) {
                FuseReturn(ret,pathname);
            }
            if ( size < ret ) {
                errno = ERANGE;
                FuseReturnError(pathname);
            }
            for ( vector<string>::iterator i = names.begin();
                    i != names.end();
                    ++ i ) {
                strcpy(list,i->c_str());
                list = list + i->size() + 1;
            }
            FuseReturn(ret,pathname);
        }
    } catch (const std::exception & e) {
        LogError(e.what());
    }

    FuseReturnError(pathname);
}


int FuseBDT::removexattr (const char * pathname, const char *name)
{
    LogDebug ( pathname << " " << name );

    try {
        auto_ptr<Inode> inode(meta_->GetInode(pathname));
        if ( inode.get() ) {
            if ( inode->DeleteExtendedAttribute(name) ) {
                FuseReturn(0,pathname);
            } else {
                FuseReturnError(pathname);
            }
        }
    } catch (const std::exception & e) {
        LogError(e.what());
    }

    FuseReturnError(pathname);
}

int FuseBDT::opendir(const char * pathname, struct fuse_file_info * info)
{
    LogDebug ( pathname );

    try {
        fs::path path(pathname);
        auto_ptr<Inode> inode( meta_->GetInode(path) );
        if ( NULL == inode.get() ) {
            errno = ENOENT;
            FuseReturnError(pathname);
        }
        if ( inode->IsFile() ) {
            errno = ENOTDIR;
            FuseReturnError(pathname);
        }

        info->fh = reinterpret_cast<uint64_t>( ::opendir(
                inode->Path().string().c_str() ) );
        if ( info->fh == 0 ) {
            FuseReturnError(pathname);
        } else {
            FuseReturn(0,pathname);
        }
    } catch (const std::exception & e) {
        LogError(e.what());
    }

    FuseReturnError(pathname);
}


int FuseBDT::readdir (
        const char * pathname,
        void * buf,
        fuse_fill_dir_t filler,
        off_t offset,
        struct fuse_file_info * info )
{
    LogDebug ( pathname );

    DIR * dir = reinterpret_cast<DIR *>(info->fh);

    struct dirent * ent = NULL;
    do {
        ent = ::readdir(dir);
        if ( ent != NULL ) {
            if ( filler(buf,ent->d_name,NULL,0) != 0 ) {
                errno = ENOMEM;
                FuseReturnError(pathname);
            }
        }
    } while ( ent != NULL );

    FuseReturn(0,pathname);
}


int FuseBDT::releasedir(const char * pathname, struct fuse_file_info * info)
{
    LogDebug ( pathname );

    DIR * dir = reinterpret_cast<DIR *>(info->fh);

    ::closedir( dir );

    FuseReturn(0,pathname);
}


int FuseBDT::fsyncdir(const char * pathname, int, struct fuse_file_info *)
{
    FuseReturn(0,pathname);
}


void * FuseBDT::init(struct fuse_conn_info *)
{
#ifdef MORE_TEST
    string rootLogger = "/tmp/bdt-ltfs/";
    if ( ! fs::exists(rootLogger) ) {
        fs::create_directory(rootLogger);
    }
    CreateLogger(rootLogger + Factory::GetService());
#endif

    Factory::CreateConfigure();
#ifdef MORE_TEST
    Factory::GetConfigure()->Refresh("/tmp/bdt.config");
#endif
    Factory::CreateThrottle(
            Factory::GetConfigure()->GetValueSize(Configure::ThrottleInterval),
            Factory::GetConfigure()->GetValueSize(Configure::ThrottleValve) );
    Factory::CreateTapeLibraryManager();
    Factory::CreateTapeManager();
    Factory::CreateCacheManager();
    Factory::CreateReadManager();
    Factory::CreateSchedule();
    Factory::CreateMetaManager();
    meta_ = bdt::Factory::GetMetaManager();

    FileOperationCIFS::SizeInode =
            Factory::GetConfigure()->GetValueSize(Configure::MetaFileSize);
    FileOperationCIFS::SizeInodeBegin =
            FileOperationCIFS::SizeInode - FileOperationCIFS::SizeInodeEnd;

    bdt::Factory::StartBackendTasks();

    server_.reset( new ServiceServer() );

    return GetState();
}


namespace bdt
{
    void
    Factory::ReleaseConfigure()
    {
        configure_.reset();
    }

    void
    Factory::ReleaseThrottle()
    {
        throttle_.reset();
    }

    void
    Factory::ReleaseTapeManager()
    {
        tape_.reset();
    }

    void
    Factory::ReleaseCacheManager()
    {
        cache_.reset();
    }

    void
    Factory::ReleaseReadManager()
    {
        read_.reset();
    }

    void
    Factory::ReleaseMetaManager()
    {
        meta_.reset();
    }

    void
    Factory::ReleaseSchedule()
    {
        schedule_.reset();
    }

    void
    Factory::ReleaseTapeLibraryManager()
    {
        changer_.reset();
    }
}


void FuseBDT::destroy(void *)
{
    server_.reset();

    bdt::Factory::StopBackendTasks();

    meta_ = NULL;
    bdt::Factory::ReleaseMetaManager();
    bdt::Factory::ReleaseSchedule();
    bdt::Factory::ReleaseReadManager();
    bdt::Factory::ReleaseCacheManager();
    bdt::Factory::ReleaseTapeManager();
    bdt::Factory::ReleaseTapeLibraryManager();
    bdt::Factory::ReleaseThrottle();
    bdt::Factory::ReleaseConfigure();
}


int FuseBDT::access(const char * pathname, int mask)
{
    errno = ENOTSUP;
    FuseReturnError(pathname);
}


int FuseBDT::create(const char * pathname, mode_t mode,
        struct fuse_file_info * info)
{
    LogDebug ( pathname
            << "\tmode:0" << oct << mode
            << " flags:0" << info->flags
            << " flags:0x" << hex << info->flags
            << dec );
    mode = mode | 0777;
    info->flags = info->flags & (~O_EXCL);

    static time_t lastCheckTime = 0;
    static off_t lastFreeSize = 0;
    static bool lastEnoughFree = false;

    try {
        fs::path path(pathname);
        auto_ptr<Inode> inode( meta_->GetInode(path) );
        if ( inode.get() ) {
            errno = EEXIST;
            FuseReturnError(pathname);
        }

        time_t now = time(NULL);
        if(now - lastCheckTime > CREATE_CHECK_CAPACITY_INTERVAL || lastFreeSize < MIN_CHECK_LAST_FREE_SIZE || false == lastEnoughFree){
            //TODO
            //assert(false);
            size_t fileNumber;
            off_t usedSize, freeSize;
            lastFreeSize = MIN_CHECK_LAST_FREE_SIZE;
            lastCheckTime = time(NULL);
            if ( TapeLibraryMgr::GetTapeGroupCapacity(
                    Factory::GetService(),fileNumber,usedSize,freeSize) ) {
                lastFreeSize = freeSize;
                off_t backupSize = 0;
                vector<BackupItem> items;
                meta_->GetBackupList(items);
                BOOST_FOREACH(const BackupItem & item, items) {
                    backupSize += item.size;
                }
                if ( backupSize >= freeSize ) {
                    LogWarn(pathname);
                    errno = ENOSPC;
                    lastEnoughFree = false;
                    FuseReturnError(pathname);
                }
                lastEnoughFree = true;
            }
        }

        if ( ! meta_->CreateFile(path,mode) ) {
            FuseReturnError(pathname);
        }

        FileOperationInterface * file = meta_->GetFileOperation(
                path, info->flags );
        if ( file == NULL ) {
            FuseReturnError(pathname);
        }

        info->fh = reinterpret_cast<uint64_t>(file);
    #if FUSE_VERSION >= 28
        info->keep_cache = 1;
    #else
        info->direct_io = 1;
    #endif

        FuseReturn(0,pathname);
    } catch (const std::exception & e) {
        LogError(e.what());
    }

    FuseReturnError(pathname);
}


int FuseBDT::ftruncate(const char * pathname, off_t length,
        struct fuse_file_info * info)
{
    LogDebug ( pathname << "\tlength:" << length );

    static off_t maxSize = Factory::GetConfigure()->GetValueSize(
            Configure::FileMaxSize );
    if ( (maxSize > 0) && (length > maxSize) ) {
        errno = EFBIG;
        FuseReturnError(pathname);
    }

    FileOperationInterface * file
            = reinterpret_cast<FileOperationInterface *>(info->fh);
    if ( file->Truncate(length) ) {
        FuseReturn(0,pathname);
    } else {
        FuseReturnError(pathname);
    }
}


int
FuseBDT::fgetattr(const char * pathname, struct stat * stbuf,
        struct fuse_file_info * info)
{
    LogDebug ( pathname );

    FileOperationInterface * file
            = reinterpret_cast<FileOperationInterface *>(info->fh);
    if ( file->GetStat(*stbuf) ) {
        FuseReturn(0,pathname);
    } else {
        errno = ENOENT;
        FuseReturnError(pathname);
    }
}

