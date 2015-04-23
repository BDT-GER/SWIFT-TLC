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
 * FuseRedirect.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: More Zeng
 */


#include <sys/types.h>
#include <dirent.h>
#include <sys/xattr.h>
#include <errno.h>

#include <fstream>
#include <iostream>

using namespace std;

#include "FuseRedirect.h"


#ifdef DEBUG
ofstream debug("/tmp/debug.log",ios::app);
#define Logger(msg) debug << __PRETTY_FUNCTION__ << "\t" << msg << endl
#else
#define Logger(msg)
#endif


FuseRedirect::FuseRedirect(const std::string & redirect)
{
    state_.RedirectPath = redirect;
}


FuseRedirect::~FuseRedirect()
{
}


void *
FuseRedirect::GetState()
{
    return & state_;
}


static int FuseRet(int ret)
{
    if ( ret < 0 ) {
        ret = - errno;
        if ( ret == 0 ) {
            ret = - ENOENT;
        }
    }

    Logger ( ret );

    return ret;
}


int FuseRedirect::getattr (const char *path, struct stat *stbuf)
{
    Logger ( path );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::lstat(pathname.c_str(), stbuf) );
}


int FuseRedirect::readlink (const char *path, char *buf, size_t size)
{
    Logger ( path );

    std::string pathname = state_.RedirectPath + path;

    int ret = ::readlink(pathname.c_str(), buf, size - 1);
    if ( ret >= 0 ) {
        buf[ret] = '\0';
        ret = 0;
    }

    return FuseRet( ret );
}


int FuseRedirect::mknod (const char *path, mode_t mode, dev_t dev)
{
    Logger ( path << "\tmode:0" << oct << mode << dec );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::mknod(pathname.c_str(), mode, dev) );
}


int FuseRedirect::mkdir (const char *path, mode_t mode)
{
    Logger ( path << "\tmode:0" << oct << mode << dec );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::mkdir(pathname.c_str(), mode) );
}


int FuseRedirect::unlink (const char *path)
{
    Logger ( path );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::unlink(pathname.c_str()) );
}


int FuseRedirect::rmdir (const char *path)
{
    Logger ( path );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::rmdir(pathname.c_str()) );
}


int FuseRedirect::symlink (const char *path, const char *link)
{
    Logger ( path << " " << link );

    std::string linkname = state_.RedirectPath + link;

    return FuseRet( ::symlink(path,linkname.c_str()) );
}


int FuseRedirect::rename (const char *path, const char *to)
{
    Logger ( path << " " << to );

    std::string pathname = state_.RedirectPath + path;
    std::string newpathname = state_.RedirectPath + to;

    return FuseRet( ::rename(pathname.c_str(), newpathname.c_str()) );
}


int FuseRedirect::link (const char *path, const char *to)
{
    Logger ( path << " " << to );

    std::string pathname = state_.RedirectPath + path;
    std::string newpathname = state_.RedirectPath + to;

    return FuseRet( ::link(pathname.c_str(), newpathname.c_str()) );
}


int FuseRedirect::chmod (const char *path, mode_t mode)
{
    Logger ( path << "\tmode:0" << oct << mode << dec );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::chmod(pathname.c_str(), mode) );
}


int FuseRedirect::chown (const char *path, uid_t uid, gid_t gid)
{
    Logger ( path << " " << uid << " " << gid );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::chown(pathname.c_str(), uid, gid) );
}


int FuseRedirect::truncate (const char *path, off_t size)
{
    Logger ( path << " " << size );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::truncate(pathname.c_str(), size) );
}


int FuseRedirect::utime (const char *path, struct utimbuf *buf)
{
    Logger ( path );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::utime(pathname.c_str(), buf) );
}


int FuseRedirect::open (const char *path, struct fuse_file_info *info)
{
    Logger ( path );

    std::string pathname = state_.RedirectPath + path;

    int fd = ::open(pathname.c_str(), info->flags);
    if ( fd < 0 ) {
    	Logger ( path << ":" << errno );
        return FuseRet( -1 );
    }

    info->fh = fd;

    return FuseRet(0);
}


int FuseRedirect::read (const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *info)
{
    Logger ( path << "\toffset:" << offset << "\tsize:" << size );

    return FuseRet( ::pread(info->fh, buf, size, offset) );
}


int FuseRedirect::write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *info)
{
    Logger ( path << "\toffset:" << offset << "\tsize:" << size );

    return FuseRet( ::pwrite(info->fh, buf, size, offset) );
}


int FuseRedirect::statfs (const char *path, struct statvfs *stbuf)
{
    Logger ( path );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::statvfs(pathname.c_str(), stbuf) );
}


int FuseRedirect::flush (const char *path, struct fuse_file_info *info)
{
    Logger ( path );

    return FuseRet(0);
}


int FuseRedirect::release (const char *path, struct fuse_file_info *info)
{
    Logger ( path );

    return FuseRet( ::close(info->fh) );
}


int FuseRedirect::fsync (const char *path, int mode, struct fuse_file_info *info)
{
    Logger ( path << " " << mode );

    if ( mode ) {
        return FuseRet( ::fdatasync( info->fh ) );
    } else {
        return FuseRet( ::fsync( info->fh ) );
    }
}


int FuseRedirect::setxattr (const char *path, const char *name, const char *value, size_t size, int flags)
{
    Logger ( path << " " << name );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::lsetxattr(pathname.c_str(), name, value, size, flags) );
}


int FuseRedirect::getxattr (const char *path, const char *name, char *value, size_t size)
{
    Logger ( path << " " << name );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::lgetxattr(pathname.c_str(), name, value, size) );
}


int FuseRedirect::listxattr (const char *path, char *list, size_t size)
{
    Logger ( path );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::llistxattr(pathname.c_str(), list, size) );
}


int FuseRedirect::removexattr (const char *path, const char *name)
{
    Logger ( path );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::lremovexattr(pathname.c_str(), name) );
}

int FuseRedirect::opendir(const char *path, struct fuse_file_info *info)
{
    Logger ( path );

    std::string pathname = state_.RedirectPath + path;

    DIR * dir = ::opendir(pathname.c_str());
    if ( dir == NULL ) {
        return FuseRet( -1 );
    }

    info->fh = (uint64_t)dir;

    return FuseRet(0);
}


int FuseRedirect::readdir(const char *path, void *buf, fuse_fill_dir_t filler,
        off_t offset, struct fuse_file_info *info)
{
    Logger ( path );

    DIR * dir = (DIR *) info->fh;

    struct dirent * de = ::readdir(dir);
    if ( de == NULL ) {
        return FuseRet(-1);
    }

    do {
        if (filler(buf, de->d_name, NULL, 0) != 0) {
            return -ENOMEM;
        }
    } while ( (de = ::readdir(dir)) != NULL );

    return FuseRet(0);
}


int FuseRedirect::releasedir(const char *path, struct fuse_file_info *info)
{
    Logger ( path );
    ::closedir( (DIR *) info->fh );
    return FuseRet(0);
}


int FuseRedirect::fsyncdir(const char *path, int, struct fuse_file_info *)
{
    Logger ( path );
    return FuseRet(0);
}


void FuseRedirect::destroy(void *)
{
}


int FuseRedirect::access(const char *path, int mask)
{
    Logger ( path << "\tmode:0" << oct << mask << dec );

    std::string pathname = state_.RedirectPath + path;

    return FuseRet( ::access(pathname.c_str(), mask) );
}


int FuseRedirect::create(const char *path, mode_t mode,
        struct fuse_file_info *info)
{
    Logger ( path << "\tmode:0" << oct << mode << dec );
    mode = 0777;
    Logger ( path << "\tmode:0" << oct << mode << dec );

    std::string pathname = state_.RedirectPath + path;

    int fd = ::creat(pathname.c_str(), mode);
    if ( fd < 0 ) {
        return FuseRet( -1 );
    }

    info->fh = fd;

    return FuseRet(0);
}


int FuseRedirect::ftruncate(const char *path, off_t offset,
        struct fuse_file_info *info)
{
    Logger ( path << " " << offset );

    return FuseRet( ::ftruncate(info->fh, offset) );
}


int FuseRedirect::fgetattr(const char *path, struct stat *stbuf,
        struct fuse_file_info *info)
{
    Logger ( path );

    return FuseRet( ::fstat(info->fh, stbuf) );
}

