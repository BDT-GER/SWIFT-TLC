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
 * TapeManagerLE.h
 *
 *  Created on: Mar 6, 2012
 *      Author: More Zeng
 */


#pragma once


#include "TapeManager.h"


namespace bdt
{

    class TapeManagerLE : public TapeManager
    {
    public:
        TapeManagerLE();

        virtual
        ~TapeManagerLE();

        enum TapeStatus
        GetTapeStatus( const string & tape );

        fs::path
        GetPath( const string & tape, const fs::path & path );

    private:
        fs::path folder_;

        time_t time_;
        typedef map<string,enum TapeStatus> TapeStatusMap;
        TapeStatusMap tape_;
        boost::mutex mutex_;

        bool
        RefreshTapeStatus();
    };

}

