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
 * FuseCallback.h
 *
 *  Created on: Feb 22, 2012
 *      Author: More Zeng
 */

#pragma once


#include "FuseHeader.h"


class FuseBase;


class FuseCallback
{
public:
    static FuseCallback * Instance();
    void SetBase(FuseBase *);
    static struct fuse_operations FuseOperations;

private:
    FuseCallback();

    static int getattr(const char *, struct stat *);
    static int readlink(const char *, char *, size_t);
    static int getdir(const char *, fuse_dirh_t, fuse_dirfil_t);
    static int mknod(const char *, mode_t,dev_t);
    static int mkdir(const char *, mode_t);
    static int unlink(const char *);
    static int rmdir(const char *);
    static int symlink(const char *, const char *);
    static int rename(const char *, const char *);
    static int link(const char *, const char *);
    static int chmod(const char *, mode_t);
    static int chown(const char *, uid_t, gid_t);
    static int truncate(const char *, off_t);
    static int utime(const char *, struct utimbuf *);
    static int open(const char *, struct fuse_file_info *);
    static int read(const char *, char *, size_t, off_t,
            struct fuse_file_info *);
    static int write(const char *, const char *, size_t, off_t,
            struct fuse_file_info *);
    static int statfs(const char *, struct statvfs *);
    static int flush(const char *, struct fuse_file_info *);
    static int release(const char *, struct fuse_file_info *);
    static int fsync(const char *, int, struct fuse_file_info *);
    static int setxattr(const char *, const char *, const char *, size_t, int);
    static int getxattr(const char *, const char *, char *, size_t);
    static int listxattr(const char *, char *, size_t);
    static int removexattr(const char *, const char *);

    static int opendir(const char *, struct fuse_file_info *);
    static int readdir(const char *, void *, fuse_fill_dir_t, off_t,
            struct fuse_file_info *);
    static int releasedir(const char *, struct fuse_file_info *);
    static int fsyncdir(const char *, int, struct fuse_file_info *);
    static void * init(struct fuse_conn_info *);
    static void destroy(void *);

    static int access(const char *, int);
    static int create(const char *, mode_t, struct fuse_file_info *);
    static int ftruncate(const char *, off_t, struct fuse_file_info *);
    static int fgetattr(const char *, struct stat *, struct fuse_file_info *);

    static int lock(const char *, struct fuse_file_info *, int,
            struct flock *);
    static int utimens(const char *, const struct timespec [2]);
    static int bmap(const char *, size_t, uint64_t *);

    static int ioctl(const char *, int, void *, struct fuse_file_info *,
            unsigned int, void *);
    static int poll(const char *, struct fuse_file_info *,
            struct fuse_pollhandle *, unsigned *);

    static FuseCallback * Self;
    static FuseBase * Base;
};

