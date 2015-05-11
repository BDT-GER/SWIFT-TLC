/* Copyright (c) 2014 BDT Media Automation GmbH
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
 * Bitmap.h
 *
 *  Created on: Nov 28, 2014
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class BitmapIO
    {
    public:
        virtual bool GetLength(off_t & length) = 0;

        virtual bool SetLength(off_t length) = 0;

        //  buffer == NULL || size == 0: Query the bitmap size
        virtual bool GetBitmap(void * buffer,int & size) = 0;

        virtual bool SetBitmap(const void * buffer,int size) = 0;
    };


    class Bitmap
    {
    public:
        Bitmap(BitmapIO * io, size_t sizeBlock = 0);

        ~Bitmap();

        bool
        GetBlockSize(size_t & size)
        {
            if ( sizeBlock_ <= 0 ) {
                size = 0;
                return false;
            } else {
                size = sizeBlock_;
                return true;
            }
        }

        bool
        GetLength(off_t & length);

        bool
        TruncateBitmap(off_t length);

        bool
        Truncate(off_t length);

        bool
        CheckBitmap(off_t offset,size_t size);

        bool
        IsFull();

        void
        MarkBitmap(off_t offset,size_t size);

        void
        DebugDump();

    private:
        boost::mutex mutex_;

        BitmapIO * io_;
        unsigned long sizeBlock_;
        enum {
            LENGTH_BITMAP = 16 * 1024,
        };
        boost::scoped_array<char> bitmap_;

        off_t begin_;
        off_t end_;
        off_t length_;
        bool dirty_;

        bool
        OpenBitmap();

        void
        MarkBitmapUnlock(off_t offset,size_t size);

        bool
        SaveBitmap(bool force);

        void
        GetBitmap(off_t offset,bool & mark);

        void
        SetBitmap(off_t offset,bool mark);
    };

}

