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
 * ServiceServer.h
 *
 *  Created on: Nov 15, 2012
 *      Author: More Zeng
 */


#pragma once


#include "SocketServer.h"


namespace bdt
{

    class ServiceServer : public SocketServer
    {
    public:
        ServiceServer();

        virtual
        ~ServiceServer();

        static string const Service;
        static string const ReleaseFile;
        static string const ReleaseInode;
        static string const ReleaseTape;
        static string const GetCacheCapacity;
        static string const SetThrottle;
        static string const Import;
        static string const SetName;
        static string const StopTape;
        static string const SetCacheState;

    private:
        fs::path folderMeta_;
        fs::path folderCache_;
//        auto_ptr<MetaManager> meta_;

        void
        ServiceThread(int handle);

    };

}

