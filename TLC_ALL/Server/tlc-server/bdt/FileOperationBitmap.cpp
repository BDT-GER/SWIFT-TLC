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
 * FileOperationBitmap.cpp
 *
 *  Created on: Mar 12, 2013
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "FileOperationBitmap.h"


namespace bdt
{

    FileOperationBitmap::FileOperationBitmap(
            const fs::path & path,
            mode_t mode,
            int flags,
            unsigned long sizeBlock)
    : FileOperation(path,mode,flags), ea_(path)
    {
        bitmap_.reset(new Bitmap(this,sizeBlock));
    }


    FileOperationBitmap::FileOperationBitmap(
            const fs::path & path, int flags, unsigned long sizeBlock)
    : FileOperation(path,flags), ea_(path)
    {
        bitmap_.reset(new Bitmap(this,sizeBlock));
    }


    FileOperationBitmap::~FileOperationBitmap()
    {
        if ( bitmap_->IsFull() ) {
            bitmap_.reset();
            ea_.DeleteName(Inode::ATTRIBUTE_BITMAP);
            ea_.DeleteName(Inode::ATTRIBUTE_SIZE);
        }
    }


    bool
    FileOperationBitmap::GetBitmapLength(off_t & length)
    {
        int size = sizeof(length);
        return ea_.GetValue(Inode::ATTRIBUTE_SIZE,&length,size,size);
    }


    bool
    FileOperationBitmap::SetBitmapLength(off_t length)
    {
        return bitmap_->TruncateBitmap(length);
    }


    bool
    FileOperationBitmap::GetLength(off_t & length)
    {
        int size = sizeof(length);
        return ea_.GetValue(Inode::ATTRIBUTE_SIZE,&length,size,size);
    }


    bool
    FileOperationBitmap::SetLength(off_t length)
    {
        return ea_.SetValue(Inode::ATTRIBUTE_SIZE,&length,sizeof(length));
    }


    bool
    FileOperationBitmap::GetBitmap(void * buffer,int & size)
    {
        return ea_.GetValue(Inode::ATTRIBUTE_BITMAP,buffer,size,size);
    }


    bool
    FileOperationBitmap::SetBitmap(const void * buffer,int size)
    {
        return ea_.SetValue(Inode::ATTRIBUTE_BITMAP,buffer,size);
    }


    bool
    FileOperationBitmap::Write(
            off_t offset, const void * buffer, size_t bufsize, size_t & size )
    {
        bool ret = FileOperation::Write(offset,buffer,bufsize,size);
        if ( ret ) {
            bitmap_->MarkBitmap(offset,size);
        }
        return ret;
    }


    bool
    FileOperationBitmap::Truncate(off_t length)
    {
        bool ret = FileOperation::Truncate(length);
        if ( ret ) {
            bitmap_->Truncate(length);
        }
        return ret;
    }


    bool
    FileOperationBitmap::GetBlockSize(size_t & size)
    {
        return bitmap_->GetBlockSize(size);
    }


    bool
    FileOperationBitmap::TruncateBitmap(off_t length)
    {
        bool ret = FileOperation::Truncate(length);
        if ( ret ) {
            bitmap_->TruncateBitmap(length);
        }
        return ret;
    }


    bool
    FileOperationBitmap::CheckBitmap(off_t offset,size_t size)
    {
        return bitmap_->CheckBitmap(offset,size);
    }


    bool
    FileOperationBitmap::IsFull()
    {
        return bitmap_->IsFull();
    }
}
