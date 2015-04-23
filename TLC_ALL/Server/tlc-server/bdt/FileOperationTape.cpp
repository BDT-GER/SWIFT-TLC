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
 * FileOperationTape.cpp
 *
 *  Created on: Dec 14, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "TapeManagerStop.h"
#include "FileOperationTape.h"


namespace bdt
{

    static const int intervalEvent = 60;


    FileOperationTape::FileOperationTape(
            const fs::path & path,
            mode_t mode,
            int flags,
            const string & tape )
    : tape_(tape), stop_(NULL),
      timeEvent_(boost::posix_time::second_clock::local_time())
    {
        try {
            file_.reset(new FileOperation(path,mode,flags));
        } catch ( const ExceptionFileNoSpace & e ) {
            Factory::GetTapeManager()->SetTapeStatus(
                    tape_, TapeManagerInterface::STATUS_FULL );
            throw;
        } catch ( const ExceptionFileIOError & e ) {
            Factory::GetTapeManager()->SetTapeStatus(
                    tape_, TapeManagerInterface::STATUS_CORRUPT );
            throw;
        }
        stop_ = dynamic_cast<TapeManagerStop *>(Factory::GetTapeManager());
        if ( NULL == stop_ ) {
            LogError(path << " " << tape);
        }
        timeEvent_ -= boost::posix_time::seconds(intervalEvent);
    }


    FileOperationTape::FileOperationTape(
            const fs::path & path,
            int flags,
            const string & tape )
    : tape_(tape), stop_(NULL),
      timeEvent_(boost::posix_time::second_clock::local_time())
    {
        file_.reset(new FileOperation(path,flags));
        stop_ = dynamic_cast<TapeManagerStop *>(Factory::GetTapeManager());
        if ( NULL == stop_ ) {
            LogError(path << " " << tape);
        }
        timeEvent_ -= boost::posix_time::seconds(intervalEvent);
    }


    bool
    FileOperationTape::IsTapeStopped()
    {
        if ( NULL == stop_ ) {
            return false;
        }
        return stop_->IsTapeStopped(tape_);
    }


    void
    FileOperationTape::ReportEvent(
            const string & eventID,const string & message )
    {
        boost::posix_time::ptime current =
                boost::posix_time::second_clock::local_time();
        if ( (current - timeEvent_).total_seconds() >= intervalEvent ) {
            timeEvent_ = current;
            EventError(eventID,message);
        }
    }


    bool
    FileOperationTape::GetStat(struct stat & stat)
    {
        return file_->GetStat(stat);
    }


    bool
    FileOperationTape::Read(
            off_t offset, void * buffer, size_t bufsize, size_t & size )
    {
        if ( IsTapeStopped() ) {
            LogError(tape_);
            return false;
        }

        bool ret = file_->Read(offset,buffer,bufsize,size);
        if ( ! ret ) {
            int errorCode = errno;
            LogError(tape_ << " " << path_ << " : " << errorCode);
            switch ( errorCode ) {
            case EIO:
                Factory::GetTapeManager()->SetTapeStatus(
                        tape_, TapeManagerInterface::STATUS_CORRUPT );
                break;
            case ENOSPC:
                Factory::GetTapeManager()->SetTapeStatus(
                        tape_, TapeManagerInterface::STATUS_FULL );
                break;
            default:
                break;
            }
            ReportEvent("Tape_Read_Failed",
                    "Failed to read file " + path_.string()
                    + " from tape " + tape_ + ".");
        }
        return ret;
    }


    bool
    FileOperationTape::Write(
            off_t offset, const void * buffer, size_t bufsize, size_t & size )
    {
        if ( IsTapeStopped() ) {
            LogError(tape_);
            return false;
        }

        bool ret = file_->Write(offset,buffer,bufsize,size);
        if ( ! ret ) {
            int errorCode = errno;
            LogError(tape_ << " " << path_ << " : " << errorCode);
            switch ( errorCode ) {
            case EIO:
                Factory::GetTapeManager()->SetTapeStatus(
                        tape_, TapeManagerInterface::STATUS_CORRUPT );
                break;
            case ENOSPC:
                Factory::GetTapeManager()->SetTapeStatus(
                        tape_, TapeManagerInterface::STATUS_FULL );
                break;
            default:
                break;
            }
            ReportEvent("Tape_Write_Failed",
                    "Failed to write file " + path_.string()
                    + " to tape " + tape_ + ".");
        }
        return ret;
    }


    bool
    FileOperationTape::Truncate(off_t length)
    {
        return file_->Truncate(length);
    }


    bool
    FileOperationTape::Sync(bool data)
    {
        return file_->Sync(data);
    }

}
