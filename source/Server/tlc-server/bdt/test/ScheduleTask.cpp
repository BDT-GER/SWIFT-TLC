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
 * ScheduleTask.cpp
 *
 *  Created on: Jul 23, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "ScheduleTask.h"


ScheduleTask::ScheduleTask(
        ScheduleInterface * schedule,
        const string & tape,
        int timeout,
        int priority,
        int duration)
: schedule_(schedule), tape_(tape), timeout_(timeout), priority_(priority),
  duration_(duration), finished_(false),
  begin_(boost::posix_time::microsec_clock::local_time()),
  end_(boost::posix_time::microsec_clock::local_time())
{
}


ScheduleTask::~ScheduleTask()
{
    if ( thread_.get() ) {
        thread_->interrupt();
        thread_->join();
    }
}


void
ScheduleTask::Start()
{
    thread_.reset( new boost::thread(&ScheduleTask::RunTask,this) );
}


void
ScheduleTask::RunTask()
{
    finished_ = false;
    if (false == schedule_->RequestTape(tape_,true,false,timeout_,priority_)) {
        return;
    }
    begin_ = boost::posix_time::microsec_clock::local_time();
    try {
        boost::this_thread::sleep(
                boost::posix_time::milliseconds(duration_) );
    } catch (const boost::thread_interrupted & e) {
    }
    end_ = boost::posix_time::microsec_clock::local_time();
    schedule_->ReleaseTape(tape_,false);
    finished_ = true;
}


bool
ScheduleTask::Finished()
{
    return finished_;
}


boost::posix_time::ptime
ScheduleTask::Begin()
{
    return begin_;
}


boost::posix_time::ptime
ScheduleTask::End()
{
    return end_;
}

