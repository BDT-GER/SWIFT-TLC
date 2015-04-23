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
 * FuseBase.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: More Zeng
 */


#include "bdt/stdafx.h"
#include "FuseBase.h"


FuseBase::FuseBase()
{
}


FuseBase::~FuseBase()
{
}

void *
FuseBase::GetState()
{
    return NULL;
}


static const char * hello_path = "/hello";


int FuseBase::getattr (const char *path, struct stat *stbuf)
{
    memset (stbuf, 0, sizeof (struct stat));

    if (strcmp (path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    } else if (strcmp (path,hello_path) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        return 0;
    } else {
        return -ENOENT;
    }
}


int FuseBase::readlink (const char *path, char *buf, size_t size)
{
    return -ENOENT;
}


int FuseBase::mknod (const char *path, mode_t mode, dev_t rdev)
{
    return -ENOENT;
}


int FuseBase::mkdir (const char *path, mode_t mode)
{
    return -ENOENT;
}


int FuseBase::unlink (const char *path)
{
    return -ENOENT;
}


int FuseBase::rmdir (const char *path)
{
    return -ENOENT;
}


int FuseBase::symlink (const char *path, const char *to)
{
    return -ENOENT;
}


int FuseBase::rename (const char *path, const char *to)
{
    return -ENOENT;
}


int FuseBase::link (const char *path, const char *to)
{
    return -ENOENT;
}


int FuseBase::chmod (const char *path, mode_t mode)
{
    return -ENOENT;
}


int FuseBase::chown (const char *path, uid_t uid, gid_t gid)
{
    return -ENOENT;
}


int FuseBase::truncate (const char *path, off_t size)
{
    return -ENOENT;
}


int FuseBase::utime (const char *path, struct utimbuf *buf)
{
    return -ENOENT;
}


int FuseBase::open (const char *path, struct fuse_file_info *info)
{
    return -ENOENT;
}


int FuseBase::read (const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *info)
{
    return -ENOENT;
}


int FuseBase::write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *info)
{
    return -ENOENT;
}


int FuseBase::statfs (const char *path, struct statvfs *stbuf)
{
    return -ENOENT;
}


int FuseBase::flush (const char *path, struct fuse_file_info *info)
{
    return -ENOENT;
}


int FuseBase::release (const char *path, struct fuse_file_info *info)
{
    return -ENOENT;
}


int FuseBase::fsync (const char *path, int mode, struct fuse_file_info *info)
{
    return -ENOENT;
}


int FuseBase::setxattr (const char *path, const char *name, const char *value, size_t size, int flags)
{
    return -ENOENT;
}


int FuseBase::getxattr (const char *path, const char *name, char *value, size_t size)
{
    return -ENOENT;
}


int FuseBase::listxattr (const char *path, char *list, size_t size)
{
    return -ENOENT;
}


int FuseBase::removexattr (const char *path, const char *name)
{
    return -ENOENT;
}

int FuseBase::opendir(const char *path, struct fuse_file_info *)
{
    if (strcmp(path,"/")!=0) {
        return -ENOENT;
    }

    return 0;
}


int FuseBase::readdir(const char *path, void *buf, fuse_fill_dir_t filler,
        off_t offset, struct fuse_file_info *info)
{
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, hello_path + 1, NULL, 0);
    return 0;
}


int FuseBase::releasedir(const char *, struct fuse_file_info *)
{
    return -ENOENT;
}


int FuseBase::fsyncdir(const char *, int, struct fuse_file_info *)
{
    return -ENOENT;
}


void * FuseBase::init(struct fuse_conn_info *)
{
    return GetState();
}


void FuseBase::destroy(void *)
{
}


int FuseBase::access(const char *, int)
{
    return -ENOENT;
}


int FuseBase::create(const char *, mode_t, struct fuse_file_info *)
{
    return -ENOENT;
}


int FuseBase::ftruncate(const char *, off_t, struct fuse_file_info *)
{
    return -ENOENT;
}


int FuseBase::fgetattr(const char *, struct stat *, struct fuse_file_info *)
{
    return -ENOENT;
}

