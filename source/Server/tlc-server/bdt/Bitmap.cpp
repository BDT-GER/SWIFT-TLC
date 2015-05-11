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
 * Bitmap.cpp
 *
 *  Created on: Nov 28, 2014
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "Bitmap.h"


namespace bdt
{

    Bitmap::Bitmap(BitmapIO * io, size_t sizeBlock)
    : io_(io), sizeBlock_(sizeBlock), bitmap_(new char[LENGTH_BITMAP]()),
      begin_(-1), end_(-1), length_(0), dirty_(false)
    {
        OpenBitmap();
        * reinterpret_cast<typeof(sizeBlock_) *>(bitmap_.get()) = sizeBlock_;
    }


    Bitmap::~Bitmap()
    {
        SaveBitmap(true);
    }


    bool
    Bitmap::OpenBitmap()
    {
        if ( io_ == NULL ) {
            return false;
        }

        if ( ! io_->GetLength(length_) ) {
            return false;
        }

        int size = 0;
        if ( ! io_->GetBitmap(NULL,size) ) {
            return false;
        }
        if ( size > LENGTH_BITMAP ) {
            return false;
        }
        if ( ! io_->GetBitmap(bitmap_.get(),size) ) {
            return false;
        }
        sizeBlock_ = * reinterpret_cast<typeof(sizeBlock_) *>(bitmap_.get());
        return true;
    }


    bool
    Bitmap::GetLength(off_t & length)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        length = length_;
        return true;
    }


    bool
    Bitmap::TruncateBitmap(off_t length)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        length_ = length;
        SaveBitmap(true);
        if ( io_ == NULL ) {
            return true;
        }
        return io_->SetLength(length);
    }


    bool
    Bitmap::Truncate(off_t length)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        if ( length > length_ ) {
            MarkBitmapUnlock(length_, length - length_);
        } else {
            length_ = length;
        }
        SaveBitmap(true);
        if ( io_ == NULL ) {
            return true;
        }
        return io_->SetLength(length);
    }


    void
    Bitmap::MarkBitmap(off_t offset,size_t size)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        MarkBitmapUnlock(offset,size);

        SaveBitmap(false);
    }


    void
    Bitmap::DebugDump()
    {
        bool mark0,mark1,mark2,mark3;
        GetBitmap(0 * sizeBlock_,mark0);
        GetBitmap(1 * sizeBlock_,mark1);
        GetBitmap(2 * sizeBlock_,mark2);
        GetBitmap(3 * sizeBlock_,mark3);
        LogDebug( "block: " << sizeBlock_ << " length: " << length_
                << " begin: " << begin_ << " end: " << end_
                << " bitmap: " << mark0 << mark1 << mark2 << mark3 );
    }


    void
    Bitmap::GetBitmap(off_t offset,bool & mark)
    {
        int offsetBlock = offset / sizeBlock_;
        int offsetByte = offsetBlock / 8 + sizeof(sizeBlock_);
        int offsetBit = offsetBlock % 8;

        mark = (bitmap_[offsetByte] & (1 << offsetBit)) ? true : false;
    }


    void
    Bitmap::SetBitmap(off_t offset,bool mark)
    {
        int offsetBlock = offset / sizeBlock_;
        int offsetByte = offsetBlock / 8 + sizeof(sizeBlock_);
        int offsetBit = offsetBlock % 8;

        unsigned char mask = 1 << offsetBit;
        unsigned char old = bitmap_[offsetByte];
        if ( mark ) {
            bitmap_[offsetByte] |= mask;
        } else {
            bitmap_[offsetByte] &= (~mask);
        }
        if ( bitmap_[offsetByte] != old ) {
            dirty_ = true;
        }
    }


    bool
    Bitmap::SaveBitmap(bool force)
    {
        if ( sizeBlock_ <= 0 ) {
            return false;
        }

        if ( (! force) && (! dirty_) ) {
            return true;
        }

        int size = sizeof(sizeBlock_);
        size += (length_ + sizeBlock_ * 8 - 1) / (sizeBlock_ * 8);
        for ( int i = 1; i < 8; ++ i ) {
            SetBitmap(length_ + sizeBlock_ * i - 1, false);
        }
        memset(bitmap_.get() + size, 0, LENGTH_BITMAP - size);

        if ( io_ == NULL ) {
            return true;
        }
        return io_->SetBitmap(bitmap_.get(),size) && io_->SetLength(length_);
    }


    void
    Bitmap::MarkBitmapUnlock(off_t offset,size_t size)
    {
        if ( begin_ < 0 ) {
            begin_ = offset;
            end_ = offset + size;
        } else {
            if ( end_ != offset ) {
                //  not continual write
                begin_ = offset;
            }
            end_ = offset + size;
        }

        //LogDebug(begin_ << " " << end_ << " " << length_);

        if ( begin_ > length_ ) {
            SetBitmap( length_, false );
        }

        if ( begin_ % sizeBlock_ ) {
            begin_ = (begin_ / sizeBlock_ + 1) * sizeBlock_;
            if ( begin_ >= end_ ) {
                begin_ = end_;
                if ( end_ > length_ ) {
                    length_ = end_;
                }
                //LogDebug(begin_ << " " << end_ << " " << length_);
                return;
            }
            //LogDebug(begin_ << " " << end_ << " " << length_);
        }

        for ( ; (off_t)(begin_ + sizeBlock_) <= end_; begin_ += sizeBlock_ ) {
            //LogDebug(begin_ << " " << end_ << " " << length_);
            SetBitmap(begin_,true);
        }

        if ( end_ >= length_ ) {
            length_ = end_;
            //LogDebug(begin_ << " " << end_ << " " << length_);
            SetBitmap( length_ - 1, true );
        }
    }


    bool
    Bitmap::CheckBitmap(off_t offset,size_t size)
    {
        if ( size == 0 ) {
            return true;
        }
        off_t end = offset + size;
        offset = (offset / sizeBlock_) * sizeBlock_;
        for ( ; offset < end; offset  += sizeBlock_ ) {
            bool mark = false;
            GetBitmap(offset,mark);
            if ( ! mark ) {
                return false;
            }
        }
        if ( length_ < end ) {
            return false;
        }
        return true;
    }


    bool
    Bitmap::IsFull()
    {
        return CheckBitmap(0,length_);
    }
}

