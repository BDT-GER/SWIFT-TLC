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
 * ScheduleInterface.h
 *
 *  Created on: May 7, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class ScheduleInterface
    {
    public:
        ScheduleInterface()
        {
        }

        virtual
        ~ScheduleInterface()
        {
        }

        virtual bool
        RequestTapes( const vector<string> & tapes, bool mount,
                bool share, int timeout, int priority ) = 0;

        virtual void
        ReleaseTapes(const vector<string> & tapes, bool share) = 0;

        bool
        RequestTape( const string & tape, bool mount,
                bool share, int timeout, int priority )
        {
            vector<string> tapes;
            tapes.push_back(tape);
            return RequestTapes(tapes,mount,share,timeout,priority);
        }

        void
        ReleaseTape(const string & tape,bool share)
        {
            vector<string> tapes;
            tapes.push_back(tape);
            return ReleaseTapes(tapes,share);
        }

        enum
        {
            PRIORITY_CARTRIDGE_DELETE_FILE = 0,
            PRIORITY_AUDIT_TAPE = 1,
            PRIORITY_DIAGNOSE_CARTRIDGE = 2,
            PRIORITY_PREREAD = 3,
            PRIORITY_WRITE = 4,
            PRIORITY_ASSIGN_TAPES= 5,
            PRIORITY_MANAGE_CARTRIDGE = 5,
            PRIORITY_READ = 6,
            PRIORITY_DRIVE_CLEAN = 7
            //PRIORITY_VERIFY_CARTRIDGE = x,
        };

    };


    class ScheduleRelease
    {
    public:
        ScheduleRelease( ScheduleInterface * schedule, bool share )
        : schedule_(schedule), share_(share)
        {
        }

        void
        AddTape(const string & tape)
        {
            tapes_.insert(tape);
        }

        void
        AddTapes(const vector<string> & tapes)
        {
            BOOST_FOREACH( const string & tape, tapes ) {
                tapes_.insert(tape);
            }
        }

        ~ScheduleRelease()
        {
            BOOST_FOREACH( const string & tape, tapes_ ) {
                schedule_->ReleaseTape(tape,share_);
            }
        }

    private:
        ScheduleInterface * schedule_;
        bool share_;
        set<string> tapes_;
    };

}

