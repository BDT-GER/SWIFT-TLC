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
 * ScheduleProxyServer.h
 *
 *  Created on: Oct 19, 2012
 *      Author: More Zeng
 */


#pragma once


#include "SocketServer.h"
#include "ScheduleAccount.h"


namespace bdt
{

    class ScheduleProxyServer : public SocketServer
    {
    public:
        ScheduleProxyServer();

        virtual
        ~ScheduleProxyServer();

        static string const Service;
        static string const Request;
        static string const Release;
        static string const Interrupt;

        bool
        InterruptThread(const string & seed);

        bool
        InsertSeed(const string & seed,const boost::thread::id & id);

        bool
        RemoveSeed(const string & seed);
        static unsigned int RecentReadActPercentage(const string& barcode);

    private:
        ScheduleInterface * schedule_;

        void
        ServiceThread(int handle);

        ScheduleAccount account_;

        typedef map<string,boost::thread::id> SeedList;
        SeedList seeds_;
        boost::mutex mutex_;

    };

}

