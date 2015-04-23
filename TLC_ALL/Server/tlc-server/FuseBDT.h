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
 * FuseBDT.h
 *
 *  Created on: Mar 6, 2012
 *      Author: More Zeng
 */

#pragma once


#include "FuseHeader.h"
#include "FuseBase.h"


class FuseBDT : public FuseBase
{
public:
    FuseBDT();

    virtual
    ~FuseBDT();

    virtual int getattr (const char *, struct stat *);
    virtual int readlink (const char *, char *, size_t);
//    virtual int getdir (const char *, fuse_dirh_t, fuse_dirfil_t);
    virtual int mknod (const char *, mode_t, dev_t);
    virtual int mkdir (const char *, mode_t);
    virtual int unlink (const char *);
    virtual int rmdir (const char *);
    virtual int symlink (const char *, const char *);
    virtual int rename (const char *, const char *);
    virtual int link (const char *, const char *);
    virtual int chmod (const char *, mode_t);
    virtual int chown (const char *, uid_t, gid_t);
    virtual int truncate (const char *, off_t);
    virtual int utime (const char *, struct utimbuf *);
    virtual int open (const char *, struct fuse_file_info *);
    virtual int read (const char *, char *, size_t, off_t,
            struct fuse_file_info *);
    virtual int write (const char *, const char *, size_t, off_t,
            struct fuse_file_info *);
    virtual int statfs (const char *, struct statvfs *);
    virtual int flush (const char *, struct fuse_file_info *);
    virtual int release (const char *, struct fuse_file_info *);
    virtual int fsync (const char *, int, struct fuse_file_info *);
    virtual int setxattr (const char *, const char *, const char *, size_t,
            int);
    virtual int getxattr (const char *, const char *, char *, size_t);
    virtual int listxattr (const char *, char *, size_t);
    virtual int removexattr (const char *, const char *);

    virtual int opendir(const char *, struct fuse_file_info *);
    virtual int readdir(const char *, void *, fuse_fill_dir_t, off_t,
            struct fuse_file_info *);
    virtual int releasedir(const char *, struct fuse_file_info *);
    virtual int fsyncdir(const char *, int, struct fuse_file_info *);
    virtual void * init(struct fuse_conn_info *);
    virtual void destroy(void *);

    virtual int access(const char *, int);
    virtual int create(const char *, mode_t, struct fuse_file_info *);
    virtual int ftruncate(const char *, off_t, struct fuse_file_info *);
    virtual int fgetattr(const char *, struct stat *, struct fuse_file_info *);

private:
    auto_ptr<ServiceServer> server_;
    MetaManager * meta_;

    typedef map< string, boost::mutex * > MutexMap;
    MutexMap mutex_;

    boost::mutex * GetMutex(const string & tape)
    {
        MutexMap::iterator pos = mutex_.find(tape);
        if ( pos == mutex_.end() ) {
            mutex_.insert(
                    MutexMap::value_type( tape, new boost::mutex() ) );
            pos = mutex_.find(tape);
        }
        return pos->second;
    }

    FileOperationInterface *
    OpenFile(const char *, int);

};

