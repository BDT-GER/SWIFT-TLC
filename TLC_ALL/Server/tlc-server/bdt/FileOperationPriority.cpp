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
 * FileOperationPriority.cpp
 *
 *  Created on: Jul 25, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "FileOperationPriority.h"
#include "FileOperation.h"


namespace bdt
{

    FileOperationPriority::FileOperationPriority(
            const fs::path & path,
            int flags,
            const string & tape,
            int timeout,
            int priority)
    : schedule_(Factory::GetSchedule()), tape_(tape),
      tapeManager_(Factory::GetTapeManager()),
      action_(TapeManagerInterface::ACTION_IDLE)
    {
        TapeManagerInterface::TapeState state;
        if ( (flags & O_ACCMODE) == O_RDONLY ) {
            state = TapeManagerInterface::STATE_READ;
        } else {
            state = TapeManagerInterface::STATE_WRITE;
        }
        if ( ! CheckRequest(state,timeout,priority) ) {
            throw ScheduleFailureException(tape_);
        }

        file_.reset(new FileOperation(path,flags));
    }


    FileOperationPriority::~FileOperationPriority()
    {
        Close();
    }


    void
    FileOperationPriority::Close()
    {
        if ( tapeManager_->SetTapeAction(
                tape_, TapeManagerInterface::ACTION_IDLE ) ) {
        } else {
            LogError(tape_);
        }

        schedule_->ReleaseTape(tape_,false);

        if ( ! tapeManager_->SetTapeState(
                tape_, TapeManagerInterface::STATE_IDLE ) ) {
            LogError(tape_);
        }
    }


    bool
    FileOperationPriority::CheckRequest(
            TapeManagerInterface::TapeState state, int timeout, int priority)
    {
        if ( ! tapeManager_->SetTapeState(tape_,state) ) {
            errno = ENOMEDIUM;
            return false;
        }

        if ( schedule_->RequestTape(tape_,true,false,timeout,priority) ) {
            return true;
        } else {
            if ( ! tapeManager_->SetTapeState(
                    tape_, TapeManagerInterface::STATE_IDLE ) ) {
                LogError(tape_);
            }
            errno = ENOMEDIUM;
            return false;
        }
    }


    bool
    FileOperationPriority::CheckAction(TapeManagerInterface::TapeAction action)
    {
        if ( action_ == action ) {
            return true;
        }

        bool ret = tapeManager_->SetTapeAction(tape_,action);
        if ( ret ) {
            action_ = action;
        }
        return ret;
    }


    bool
    FileOperationPriority::GetStat(struct stat & stat)
    {
        return file_->GetStat(stat);
    }


    bool
    FileOperationPriority::Read(
            off_t offset, void * buffer, size_t bufsize, size_t & size )
    {
        if ( ! CheckAction(TapeManagerInterface::ACTION_READ) ) {
            errno = EBUSY;
            return false;
        }

        return file_->Read(offset,buffer,bufsize,size);
    }


    bool
    FileOperationPriority::Write(
            off_t offset, const void * buffer, size_t bufsize, size_t & size)
    {
        if ( ! CheckAction(TapeManagerInterface::ACTION_WRITE) ) {
            errno = EBUSY;
            return false;
        }

        return file_->Write(offset,buffer,bufsize,size);
    }


    bool
    FileOperationPriority::Truncate(off_t length)
    {
        if ( ! CheckAction(TapeManagerInterface::ACTION_WRITE) ) {
            errno = EBUSY;
            return false;
        }

        return file_->Truncate(length);
    }


    bool
    FileOperationPriority::Sync(bool data)
    {
        return file_->Sync(data);
    }

}

