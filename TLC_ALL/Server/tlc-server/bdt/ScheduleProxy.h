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
 * ScheduleProxy.h
 *
 *  Created on: Oct 19, 2012
 *      Author: More Zeng
 */


#pragma once


#include "ScheduleProxyServer.h"


namespace bdt
{

    class ScheduleProxy : public ScheduleInterface
    {
    public:
        ScheduleProxy();

        virtual
        ~ScheduleProxy();

        bool
        RequestTapes( const vector<string> & tapes, bool mount,
                bool share, int timeout, int priority);

        void
        ReleaseTapes(const vector<string> & tapes, bool share);

    private:
        auto_ptr<ScheduleProxyServer> server_;

        void
        RequestTapesThread(
                const string & seed,
                const vector<string> & tapes, bool mount,
                bool share, int timeout, int priority,
                bool * ret );

        void
        RequestTapeThread(
                const string & seed,
                const vector<string> & tapes, bool mount,
                bool share, int timeout, int priority,
                string * ret );

    };

}

