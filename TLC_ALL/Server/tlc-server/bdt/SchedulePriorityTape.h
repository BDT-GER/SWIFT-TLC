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
 * SchedulePriorityTape.h
 *
 *  Created on: Jul 17, 2012
 *      Author: More Zeng
 */


#pragma once


#include "PriorityTape.h"
#include "ResourceTape.h"


namespace bdt
{

    class SchedulePriorityTape : public ScheduleInterface
    {
    public:
        SchedulePriorityTape(ResourceTape * resource);

        virtual
        ~SchedulePriorityTape();

        bool
        RequestTapes( const vector<string> & tapes, bool mount,
                bool share, int timeout, int priority );

        void
        ReleaseTapes(const vector<string> & tapes, bool share);

        bool
        IsTapeBusy(const string & tape)
        {
            PriorityTapeMap::iterator i = tapes_.find(tape);
            if ( tapes_.end() == i ) {
                return false;
            }
            return i->second->Busy(false);
        }

    private:
        fs::path path_;

        typedef map<string,PriorityTape *> PriorityTapeMap;
        PriorityTapeMap tapes_;

        auto_ptr<ResourceTape> resource_;

        boost::mutex mutex_;

        bool
        RequestResource(
                const vector<string> & tapes,
                vector<PriorityTape *> & priorities);

        auto_ptr<boost::thread> thread_;

        void
        ScheduleTask();

        boost::posix_time::ptime scheduleTime_;

        void
        Schedule(
                const vector<string> & tapes = vector<string>(),
                int priority = -1 );
    };
}
