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
 * TapeManagerProxyServer.h
 *
 *  Created on: Oct 22, 2012
 *      Author: More Zeng
 */


#pragma once


#include "SocketServer.h"


namespace bdt
{

    class TapeManagerProxyServer : public SocketServer
    {
    public:
        TapeManagerProxyServer();

        virtual
        ~TapeManagerProxyServer();

        static string const Service;
        static string const GetTapesUse;
        static string const SetTapesUse;
        static string const GetShareAvailableTapes;
        //static string const GetCapacity;
        static string const GetTapeStatus;
        static string const SetTapeStatus;
        static string const CheckTapeState;
        static string const SetTapeStateNoLock;
        static string const LockTapeState;
        static string const SetTapeState;
        static string const SetTapeAction;
        static string const OpenMailSlot;
        static string const InventoryLibrary;
        static string const GetDriveNum;
        static string const GetTapeActivity;

    private:
        TapeManagerInterface * tape_;

        void
        ServiceThread(int handle);

    };

}

