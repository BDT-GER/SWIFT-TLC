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
 * FileOperationCIFS.h
 *
 *  Created on: Apr 24, 2012
 *      Author: More Zeng
 */


#pragma once


#include "FileOperationEntity.h"
#include "CIFSWait.h"


namespace bdt
{

    class FileOperationCIFS : public FileOperationEntity
    {
    public:
        FileOperationCIFS(
                Inode * inode, FileOperationInterface * oper, int wait);

        virtual
        ~FileOperationCIFS();

        virtual bool
        Read(off_t offset, void * buffer, size_t bufsize, size_t & size);

        virtual bool
        Write(
                off_t offset,
                const void * buffer,
                size_t bufsize,
                size_t & size);

        virtual bool
        Truncate(off_t length);

        static off_t SizeInodeRead;
        static off_t SizeInodeBegin;
        static off_t SizeInodeEnd;
        static off_t SizeInode;

        static bool CheckInode(Inode * inode)
        {
            off_t size;
            if ( ! inode->GetSize(size) ) {
                return false;
            }
            if ( size <= SizeInode ) {
                return size == (off_t)fs::file_size(inode->Path());
            } else {
                return SizeInode == (off_t)fs::file_size(inode->Path());
            }
            return false;
        }

    protected:
        virtual bool
        CheckReadReady(off_t offset);

        bool ready_;

        auto_ptr<Inode> inode_;

        bool
        IsInodeReadable()
        {
            return CheckInode(inode_.get());
        }

    private:
        auto_ptr<boost::thread> thread_;
        boost::mutex mutex_;
        boost::condition_variable cond_;

        void
        ReadReady(off_t offset);

        bool written_;

        CIFSWait wait_;

        off_t length_;
    };

}

