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
 * PriorityTape.cpp
 *
 *  Created on: Jul 17, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "PriorityTape.h"

const int GEN_TRICKLE_CACHE_SUSPEND_TIME = 60*15; // 15 minutes

namespace bdt
{

    boost::mutex PriorityTape::mutexDump_;


    PriorityTape::PriorityTape()
    : enable_(false), busy_(false), reference_(0),
      time_(boost::posix_time::microsec_clock::local_time()),
      priorityFile_(-1),
      idleFile_( (int)Factory::GetConfigure()->GetValueSize(
              Configure::FileIdleTime ) ),
      timeFile_(boost::posix_time::microsec_clock::local_time()),
      enableDump_(false)
    {
        int idle = (int)Factory::GetConfigure()->GetValueSize(
                Configure::TapeIdleTime );
        time_ = time_ - boost::posix_time::seconds(idle);
    }


    PriorityTape::~PriorityTape()
    {
    }


    bool
    PriorityTape::CheckTape(RequestData & data,int & wait)
    {
        if ( ! enable_ ) {
            return false;
        }
        if ( busy_ ) {
            return false;
        }
        if ( ! RequestDataCompare::Equal(*data_.begin(),data) ) {
            return false;
        }

        if ( data.priority >= priorityFile_ ) {
            return true;
        }

        LogDebug("switch priority: "
                << boost::join(data.tapes,",") << " " << data.priority);
        boost::posix_time::ptime current =
                boost::posix_time::microsec_clock::local_time();
        int elapse = (current - timeFile_).total_milliseconds();
        int needWait = idleFile_;
        /*if(data.priority == bdt::ScheduleInterface::PRIORITY_VERIFY_CARTRIDGE && priorityFile_ == bdt::ScheduleInterface::PRIORITY_READ){
        	needWait = GEN_TRICKLE_CACHE_SUSPEND_TIME;
        }*/
        if ( elapse >= needWait * 1000 ) {
            return true;
        } else {
            wait = min( needWait * 1000 - elapse, wait );
            return false;
        }
    }


    bool
    PriorityTape::RequestNotShare(
            const vector<string> & tapes,
            int timeout, int priority )
    {
        LogDebug(boost::join(tapes,",") << " " << timeout << " " << priority);

        boost::unique_lock<boost::mutex> lock(mutex_);

        boost::posix_time::ptime begin =
                boost::posix_time::microsec_clock::local_time();

        RequestData data(priority,begin,tapes);
        DumpData("before insert");
        data_.insert(data);
        DumpData("after insert");

        boost::posix_time::ptime current =
                boost::posix_time::microsec_clock::local_time();
        int wait = timeout - (current - begin).total_milliseconds();
        while ( ! CheckTape(data,wait) ) {
            int elapse = (current - begin).total_milliseconds();
            if ( elapse >= timeout ) {
                goto Failed;
            }
            try {
                condition_.timed_wait(
                        lock, boost::posix_time::milliseconds(wait) );
            } catch (const boost::thread_interrupted & e) {
                goto Failed;
            }
            current = boost::posix_time::microsec_clock::local_time();
            wait = timeout - (current - begin).total_milliseconds();
        }

        DumpData("before run");
        busy_ = true;
        ++ reference_;
        priorityFile_ = data.priority;
        data_.erase(data);
        DumpData("after run");

        return true;

Failed:

        DumpData("before fail");
        data_.erase(data);
        DumpData("after fail");

        return false;
    }


    bool
    PriorityTape::RequestForShare(
            const vector<string> & tapes,
            int timeout, int priority )
    {
        LogDebug(boost::join(tapes,",") << " " << timeout << " " << priority);

        boost::unique_lock<boost::mutex> lock(mutex_);

        boost::posix_time::ptime begin =
                boost::posix_time::microsec_clock::local_time();

        RequestData data(priority,begin,tapes);
        DumpData("before insert");
        data_.insert(data);
        DumpData("after insert");

        while ( ! enable_ ) {
            boost::posix_time::ptime current =
                    boost::posix_time::microsec_clock::local_time();
            int elapse = (current - begin).total_milliseconds();
            if ( elapse >= timeout ) {
                goto Failed;
            }
            try {
                condition_.timed_wait(
                        lock,
                        boost::posix_time::milliseconds(timeout - elapse) );
            } catch (const boost::thread_interrupted & e) {
                goto Failed;
            }
        }

        DumpData("before run");
        ++ reference_;
        data_.erase(data);
        DumpData("after run");

        lock.unlock();
        condition_.notify_all();

        return true;

Failed:

        DumpData("before fail");
        data_.erase(data);
        DumpData("after fail");

        return false;
    }


    bool
    PriorityTape::Request(
            const vector<string> & tapes,
            bool share, int timeout, int priority )
    {
        if ( share ) {
            return RequestForShare(tapes,timeout,priority);
        } else {
            return RequestNotShare(tapes,timeout,priority);
        }
    }


    void
    PriorityTape::Release(bool share)
    {
        boost::unique_lock<boost::mutex> lock(mutex_);

        DumpData("release");

        -- reference_;
        if ( reference_ < 0 ) {
            LogError(share << " " << reference_);
            reference_ = 0;
        }

        time_ = boost::posix_time::microsec_clock::local_time();

        if ( ! share ) {
            assert( true == busy_ );
            busy_ = false;
            timeFile_ = boost::posix_time::microsec_clock::local_time();
            if ( enable_ ) {
                lock.unlock();
                condition_.notify_all();
            }
        }
    }


    bool
    PriorityTape::Enable(bool enable)
    {
        boost::unique_lock<boost::mutex> lock(mutex_);

        if ( enable_ == enable ) {
            return true;
        }

        enable_ = enable;

        if ( enable ) {
            if ( ! data_.empty() ) {
                lock.unlock();
                condition_.notify_all();
            }
        }

        return true;
    }

}
