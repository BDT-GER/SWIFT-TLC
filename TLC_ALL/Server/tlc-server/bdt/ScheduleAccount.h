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
 * ScheduleAccount.h
 *
 *  Created on: Dec 25, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class ScheduleAccount
    {
    public:
        ScheduleAccount(ScheduleInterface * schedule);

        ~ScheduleAccount();

        void
        InsertSchedule(pid_t pid,const string & tape,bool share);

        void
        DeleteSchedule(pid_t pid,const string & tape,bool share);

    private:
        ScheduleInterface * schedule_;

        typedef map<pid_t,vector<string> > ScheduleMap;
        ScheduleMap scheduleMap_;
        boost::mutex mutex_;

        void
        SetSchedule(string & schedule,const string & tape,bool share);

        void
        GetSchedule(const string & schedule,string & tape,bool & share);

        boost::thread thread_;

        void
        ServerThread();

        bool
        CleanUp();
    };

}

