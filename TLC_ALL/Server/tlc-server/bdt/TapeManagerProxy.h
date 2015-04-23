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
 * TapeManagerProxy.h
 *
 *  Created on: Oct 22, 2012
 *      Author: More Zeng
 */


#pragma once


#include "TapeManagerProxyServer.h"


namespace bdt
{

    class TapeManagerProxy : public TapeManagerInterface
    {
    public:
        TapeManagerProxy();

        virtual
        ~TapeManagerProxy();

        bool
        SetTapeStatus( const string & tape, enum TapeStatus status );

        enum TapeStatus
        GetTapeStatus( const string & tape );

        bool GetShareAvailableTapes(const string& uuid,
        		vector<map<string, off_t> >& tapesList);

        bool
        GetTapesUse(vector<string> & tapes,const fs::path & path,off_t size);

        bool
        SetTapesUse(
                const vector<string> & tapes,
                int fileNumber,
                off_t fileSize,
                off_t tapeSize );

        /*bool
        GetCapacity(
                const string & service,
                size_t & fileNumber,
                off_t & usedSize,
                off_t & freeSize );*/

        fs::path
        GetPath( const string & tape, const fs::path & path );

        bool
        CheckTapeState( const string & tape, enum TapeState state );

        bool
        SetTapeStateNoLock( const string & tape, enum TapeState state );

        bool
        LockTapeState( const string & tape );

        bool
        SetTapeState( const string & tape, enum TapeState state );

        bool
        SetTapeAction( const string & tape, enum TapeAction action );

        bool
        GetDriveNum(int& driveNum);
        bool GetTapeActivity(const string& barcode, int& act, int& percentage);

    private:
        fs::path folder_;

        auto_ptr<TapeManagerProxyServer> server_;

        string service_;

    };

}

