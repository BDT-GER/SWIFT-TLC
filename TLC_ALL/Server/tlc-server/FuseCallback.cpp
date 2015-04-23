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
 * FuseCallback.cpp
 *
 *  Created on: Feb 22, 2012
 *      Author: More Zeng
 */


#include "bdt/stdafx.h"

#include "FuseCallback.h"
#include "FuseBase.h"


FuseCallback * FuseCallback::Self = NULL;
FuseBase * FuseCallback::Base = NULL;

struct fuse_operations FuseCallback::FuseOperations =
{
    getattr     : FuseCallback::getattr,
    readlink  	: FuseCallback::readlink,
    getdir    	: NULL,
    mknod     	: FuseCallback::mknod,
    mkdir     	: FuseCallback::mkdir,
    unlink    	: FuseCallback::unlink,
    rmdir     	: FuseCallback::rmdir,
    symlink   	: FuseCallback::symlink,
    rename    	: FuseCallback::rename,
    link      	: FuseCallback::link,
    chmod     	: FuseCallback::chmod,
    chown     	: FuseCallback::chown,
    truncate  	: FuseCallback::truncate,
    utime     	: FuseCallback::utime,
    open      	: FuseCallback::open,
    read      	: FuseCallback::read,
    write     	: FuseCallback::write,
    statfs    	: FuseCallback::statfs,
    flush       : FuseCallback::flush,
    release     : FuseCallback::release,
    fsync       : FuseCallback::fsync,
    setxattr    : FuseCallback::setxattr,
    getxattr    : FuseCallback::getxattr,
    listxattr   : FuseCallback::listxattr,
    removexattr : FuseCallback::removexattr,

    opendir     : FuseCallback::opendir,
    readdir     : FuseCallback::readdir,
    releasedir  : FuseCallback::releasedir,
    fsyncdir    : FuseCallback::fsyncdir,
    init        : FuseCallback::init,
    destroy     : FuseCallback::destroy,
    access      : FuseCallback::access,
    create      : FuseCallback::create,
    ftruncate   : FuseCallback::ftruncate,
    fgetattr    : FuseCallback::fgetattr,
};


FuseCallback::FuseCallback()
{
}

void
FuseCallback::SetBase(FuseBase *base)
{
    FuseCallback::Base = base;
}


FuseCallback *
FuseCallback::Instance()
{
    if ( FuseCallback::Self == NULL ) {
        FuseCallback::Self = new FuseCallback();
    }

    return FuseCallback::Self;
}


int FuseCallback::getattr (const char *path, struct stat *stbuf)
{
    if (Base)
        return Base->getattr (path, stbuf);

    return -ENOENT;
}


int FuseCallback::readlink (const char *path, char *buf, size_t size)
{
    if (Base)
        return Base->readlink (path, buf, size);

    return -ENOENT;
}


int FuseCallback::mknod (const char *path, mode_t mode, dev_t rdev)
{
    if (Base)
        return Base->mknod (path, mode, rdev);

    return -ENOENT;
}


int FuseCallback::mkdir (const char *path, mode_t mode)
{
    if (Base)
        return Base->mkdir (path, mode);

    return -ENOENT;
}


int FuseCallback::unlink (const char *path)
{
    if (Base)
        return Base->unlink (path);

    return -ENOENT;
}


int FuseCallback::rmdir (const char *path)
{
    if (Base)
        return Base->rmdir (path);

    return -ENOENT;
}


int FuseCallback::symlink (const char *path, const char *to)
{
    if (Base)
        return Base->symlink (path, to);

    return -ENOENT;
}


int FuseCallback::rename (const char *path, const char *to)
{
    if (Base)
        return Base->rename (path, to);

    return -ENOENT;
}


int FuseCallback::link (const char *path, const char *to)
{
    if (Base)
        return Base->link (path, to);

    return -ENOENT;
}


int FuseCallback::chmod (const char *path, mode_t mode)
{
    if (Base)
        return Base->chmod (path, mode);

    return -ENOENT;
}


int FuseCallback::chown (const char *path, uid_t uid, gid_t gid)
{
    if (Base)
        return Base->chown (path, uid, gid);

    return -ENOENT;
}


int FuseCallback::truncate (const char *path, off_t size)
{
    if (Base)
        return Base->truncate (path, size);

    return -ENOENT;
}


int FuseCallback::utime (const char *path, struct utimbuf *buf)
{
    if (Base)
        return Base->utime (path, buf);

    return -ENOENT;
}


int FuseCallback::open (const char *path, struct fuse_file_info *info)
{
    if (Base)
        return Base->open (path, info);

    return -ENOENT;
}


int FuseCallback::read (const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *info)
{
    if (Base)
        return Base->read (path, buf, size, offset, info);

    return -ENOENT;
}


int FuseCallback::write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *info)
{
    if (Base)
        return Base->write (path, buf, size, offset, info);

    return -ENOENT;
}


int FuseCallback::statfs (const char *path, struct statvfs *stbuf)
{
    if (Base)
        return Base->statfs (path, stbuf);

    return -ENOENT;
}


int FuseCallback::flush (const char *path, struct fuse_file_info * info)
{
    if (Base)
        return Base->flush (path, info);

    return -ENOENT;
}


int FuseCallback::release (const char *path, struct fuse_file_info *info)
{
    if (Base)
        return Base->release (path, info);

    return -ENOENT;
}


int FuseCallback::fsync (const char *path, int mode, struct fuse_file_info *info)
{
    if (Base)
        return Base->fsync (path, mode, info);

    return -ENOENT;
}


int FuseCallback::setxattr (const char *path, const char *name, const char *value, size_t size, int flags)
{
    if (Base)
        return Base->setxattr (path, name, value, size, flags);

    return -ENOENT;
}


int FuseCallback::getxattr (const char *path, const char *name, char *value, size_t size)
{
    if (Base)
        return Base->getxattr (path, name, value, size);

    return -ENOENT;
}


int FuseCallback::listxattr (const char *path, char *list, size_t size)
{
    if (Base)
        return Base->listxattr (path, list, size);

    return -ENOENT;
}


int FuseCallback::removexattr (const char *path, const char *name)
{
    if (Base)
        return Base->removexattr (path, name);

    return -ENOENT;
}


int FuseCallback::opendir(const char *path, struct fuse_file_info *info)
{
    if (Base)
        return Base->opendir (path, info);

    return -ENOENT;
}


int FuseCallback::readdir(const char *path, void *buf, fuse_fill_dir_t filler,
        off_t offset, struct fuse_file_info *info)
{
    if (Base)
        return Base->readdir (path, buf, filler, offset, info);

    return -ENOENT;
}


int FuseCallback::releasedir(const char *path, struct fuse_file_info *info)
{
    if (Base)
        return Base->releasedir (path, info);

    return -ENOENT;
}


int FuseCallback::fsyncdir(const char *path, int datasync,
         struct fuse_file_info *info)
{
    if (Base)
        return Base->fsyncdir (path, datasync, info);

    return -ENOENT;
}


void * FuseCallback::init(struct fuse_conn_info *conn)
{
    if (Base)
        return Base->init (conn);

    return NULL;
}


void FuseCallback::destroy(void *userdata)
{
    if (Base)
        return Base->destroy (userdata);
}


int FuseCallback::access(const char *path, int mask)
{
    if (Base)
        return Base->access (path, mask);

    return -ENOENT;
}


int FuseCallback::create(const char *path, mode_t mode,
        struct fuse_file_info *info)
{
    if (Base)
        return Base->create (path, mode, info);

    return -ENOENT;
}


int FuseCallback::ftruncate(const char *path, off_t offset,
        struct fuse_file_info *info)
{
    if (Base)
        return Base->ftruncate (path, offset, info);

    return -ENOENT;
}


int FuseCallback::fgetattr(const char * path, struct stat *statbuf,
        struct fuse_file_info *info)
{
    if (Base)
        return Base->fgetattr (path, statbuf, info);

    return -ENOENT;
}

