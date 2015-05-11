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
 * FileOperationBitmap.h
 *
 *  Created on: Mar 12, 2013
 *      Author: More Zeng
 */


#pragma once


#include "FileOperation.h"
#include "Bitmap.h"


namespace bdt
{

    class FileOperationBitmap : public FileOperation, public BitmapIO
    {
    public:
        FileOperationBitmap(
                const fs::path & path,
                mode_t mode,
                int flags,
                unsigned long sizeBlock);

        FileOperationBitmap(
                const fs::path & path, int flags, unsigned long sizeBlock);

        virtual
        ~FileOperationBitmap();

        bool
        Write(off_t offset,const void * buffer,size_t bufsize,size_t & size);

        bool
        Truncate(off_t length);

        bool
        GetBlockSize(size_t & size);

        bool
        TruncateBitmap(off_t length);

        bool
        CheckBitmap(off_t offset,size_t size);

        bool
        GetBitmapLength(off_t & length);

        bool
        SetBitmapLength(off_t length);

        bool
        IsFull();

        void
        DebugDump();

    private:
        ExtendedAttribute ea_;

        friend class Bitmap;
        auto_ptr<Bitmap> bitmap_;

        bool GetLength(off_t & length);

        bool SetLength(off_t length);

        bool GetBitmap(void * buffer,int & size);

        bool SetBitmap(const void * buffer,int size);
    };

}

