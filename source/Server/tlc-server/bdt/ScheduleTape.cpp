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
 * ScheduleTape.cpp
 *
 *  Created on: May 18, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "ScheduleTape.h"


namespace bdt
{

    ScheduleTape::ScheduleTape(int delayElevator,int delayPath,int delayTape)
    : folder_(Factory::GetTapeFolder().string()),
      delayElevator_(delayElevator), delayPath_(delayPath),
      delayTape_(delayTape),
      referenceTape_(0),
      offsetPath_(0),
      busyPath_(false)
    {
    }


    ScheduleTape::~ScheduleTape()
    {
    }


    bool
    ScheduleTape::Request(
            const fs::path & path, off_t offset, size_t size,
            int timeout, int priority )
    {
        boost::posix_time::ptime begin =
                boost::posix_time::microsec_clock::local_time();

        if ( ! Request(path,true,timeout,priority) ) {
            return false;
        }

        boost::unique_lock<boost::mutex> lock(mutex_);

        boost::posix_time::ptime current =
                boost::posix_time::microsec_clock::local_time();

BusyWait:

        if ( true == busyPath_ ) {
            do {
                try {
                    condition_.timed_wait(
                            lock,
                            boost::posix_time::milliseconds(
                                    delayElevator_ / 2 ) );
                } catch (const boost::thread_interrupted & e) {
                    goto Failed;
                }
                current = boost::posix_time::microsec_clock::local_time();
                if ( false == busyPath_ ) {
                    break;
                }
            } while ( (current - begin).total_milliseconds() < timeout );
        }

        if ( true == busyPath_ ) {
            goto Failed;
        }

        if ( currentPath_.empty() ) {
            currentPath_ = path;
            timePath_ = current;
            offsetPath_ = offset;
        }

PathWait:

        if ( path != currentPath_ ) {
            do {
                if ((current - timePath_).total_milliseconds() >= delayPath_) {
                    currentPath_ = path;
                    timePath_ = current;
                    offsetPath_ = offset;
                }
                if ( path == currentPath_ ) {
                    break;
                }
                try {
                    condition_.timed_wait(
                            lock,
                            boost::posix_time::milliseconds(delayPath_ / 2) );
                } catch (const boost::thread_interrupted & e) {
                    goto Failed;
                }
                current = boost::posix_time::microsec_clock::local_time();
                if ( true == busyPath_ ) {
                    goto BusyWait;
                }
            } while ( (current - begin).total_milliseconds() < timeout );
        }

        if ( path != currentPath_ ) {
            goto Failed;
        }

        if ( offset < offsetPath_ ) {
            do {
                if ( (current - timePath_).total_milliseconds()
                        >= delayElevator_ ) {
                    currentPath_ = path;
                    timePath_ = current;
                    offsetPath_ = offset;
                }
                if ( offset >= offsetPath_ ) {
                    break;
                }
                try {
                    condition_.timed_wait(
                            lock,
                            boost::posix_time::milliseconds(
                                    delayElevator_ / 2 ) );
                } catch (const boost::thread_interrupted & e) {
                    goto Failed;
                }
                current = boost::posix_time::microsec_clock::local_time();
                if ( true == busyPath_ ) {
                    goto BusyWait;
                }
                if ( path != currentPath_ ) {
                    goto PathWait;
                }
            } while ( (current - begin).total_milliseconds() < timeout );
        }

        if ( offset >= offsetPath_ ) {
            offsetPath_ = offset + size;
            busyPath_ = true;
            return true;
        } else {
            goto Failed;
        }

Failed:
        lock.unlock();
        Release(path,true);
        lock.lock();
        return false;
    }


    bool
    ScheduleTape::Request(
            const fs::path & path, bool share, int timeout, int priority )
    {
        if ( ! share ) {
            return Request(path,0,0,timeout,priority);
        }

        boost::unique_lock<boost::mutex> lock(mutex_);

        string tape;
        if ( ! GetTape(path,tape) ) {
            return false;
        }

        if ( currentTape_.empty() ) {
            currentTape_ = tape;
        }

        if ( tape == currentTape_ ) {
            ++ referenceTape_;
            return true;
        }

        boost::posix_time::ptime begin =
                boost::posix_time::microsec_clock::local_time();
        boost::posix_time::ptime current = begin;
        do {
            if ( tape != currentTape_ ) {
                if ( 0 == referenceTape_ ) {
                    if ( (current - timeTape_).total_milliseconds()
                            >= delayTape_ ) {
                        currentTape_ = tape;
                        assert( false == busyPath_ );
                        currentPath_.clear();
                        offsetPath_ = 0;
                    }
                }
            }
            if ( tape == currentTape_ ) {
                break;
            }
            condition_.timed_wait(
                    lock,
                    boost::posix_time::milliseconds(delayTape_) );
            current = boost::posix_time::microsec_clock::local_time();
        } while ( (current - begin).total_milliseconds() < timeout );

        if ( tape == currentTape_ ) {
            ++ referenceTape_;
            return true;
        } else {
            return false;
        }
    }


    void
    ScheduleTape::Release(const fs::path & path,bool share)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        string tape;
        if ( GetTape(path,tape) ) {
            assert( tape == currentTape_ );
        } else {
            assert( false );
        }

        if ( referenceTape_ <= 0 ) {
            assert(false);
        } else {
            -- referenceTape_;
        }

        timeTape_ = boost::posix_time::microsec_clock::local_time();

        if ( ! share ) {
            assert( currentPath_ == path );
            assert( true == busyPath_ );
            busyPath_ = false;
            timePath_ = timeTape_;
        }

        return;
    }

}
