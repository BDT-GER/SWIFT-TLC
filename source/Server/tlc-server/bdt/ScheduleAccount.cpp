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
 * ScheduleAccount.cpp
 *
 *  Created on: Dec 25, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "ScheduleAccount.h"


namespace bdt
{

    ScheduleAccount::ScheduleAccount(ScheduleInterface * schedule)
    : schedule_(schedule),
      thread_(boost::thread(&ScheduleAccount::ServerThread,this))
    {
    }


    ScheduleAccount::~ScheduleAccount()
    {
        thread_.interrupt();
        thread_.join();
    }


    void
    ScheduleAccount::InsertSchedule(pid_t pid,const string & tape,bool share)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        string schedule;
        SetSchedule(schedule,tape,share);

        ScheduleMap::iterator i = scheduleMap_.insert(
                ScheduleMap::value_type(pid,vector<string>()) ).first;
        i->second.push_back(schedule);
    }


    void
    ScheduleAccount::DeleteSchedule(pid_t pid,const string & tape,bool share)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        string schedule;
        SetSchedule(schedule,tape,share);

        ScheduleMap::iterator i = scheduleMap_.find(pid);
        if ( scheduleMap_.end() == i ) {
            LogError(pid << " : " << schedule);
            return;
        }

        vector<string>::iterator iter = find(
                i->second.begin(),
                i->second.end(),
                schedule );
        if ( i->second.end() == iter ) {
            LogError(pid << " : " << schedule);
            return;
        } else {
            i->second.erase(iter);
            return;
        }
    }


    void
    ScheduleAccount::SetSchedule(
            string & schedule, const string & tape, bool share )
    {
        schedule = tape + "_" + boost::lexical_cast<string>(share);
    }

    void
    ScheduleAccount::GetSchedule(
            const string & schedule, string & tape, bool & share )
    {
        string::size_type size = schedule.size();
        tape = schedule.substr( 0, size - 2 );
        share = boost::lexical_cast<bool>( schedule.substr(size-1,1) );
    }


    void
    ScheduleAccount::ServerThread()
    {
        try {
            while ( true ) {
                boost::this_thread::sleep(boost::posix_time::seconds(5));
                while ( CleanUp() ) {
                }
            }
        } catch (const boost::thread_interrupted & e) {
        }
    }


    bool
    ScheduleAccount::CleanUp()
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        for ( ScheduleMap::iterator i = scheduleMap_.begin();
                i != scheduleMap_.end();
                ++ i ) {
            string proc = "/proc/";
            proc += boost::lexical_cast<string>(i->first);
            struct stat stat;
            if ( 0 == ::stat(proc.c_str(),&stat) ) {
                continue;
            }

            LogError("Cleanup the schedule resources for process "
                    << i->first);
            for ( vector<string>::iterator iter = i->second.begin();
                    iter != i->second.end();
                    ++ iter ) {
                LogError("Cleanup the schedule resource " << *iter);
                string tape;
                bool share;
                GetSchedule(*iter,tape,share);
                schedule_->ReleaseTape(tape,share);
            }
            scheduleMap_.erase(i);
            return true;
        }

        return false;
    }

}
