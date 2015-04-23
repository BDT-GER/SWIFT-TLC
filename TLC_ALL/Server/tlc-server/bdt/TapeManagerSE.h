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
 * TapeManagerSE.h
 *
 *  Created on: Jul 24, 2012
 *      Author: More Zeng
 */


#pragma once


#include "TapeManager.h"

#ifdef MORE_TEST
#else
#include "../ltfs_management/TapeLibraryMgr.h"
using namespace ltfs_management;
#endif


namespace bdt
{

    class TapeManagerSE : public TapeManager
    {
    public:
        TapeManagerSE();

        virtual
        ~TapeManagerSE();

        bool
        SetTapeStatus( const string & tape, enum TapeStatus status );

        enum TapeStatus
        GetTapeStatus( const string & tape );

        bool
        GetTapesUse(vector<string> & tapes,const fs::path & path,off_t size);
        bool GetShareAvailableTapes(const string& uuid, vector<map<string, off_t> >& tapesList);

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

        virtual bool
        GetDriveNum(int& driveNum);
        virtual bool GetTapeActivity(const string& barcode, int& act, int& percentage);

    private:
        fs::path folder_;
#ifdef MORE_TEST
        tape::TapeLibraryManager * manager_;
#else
        typedef map<string,int> StateMap;
        StateMap state_;
#endif

        bool
        GetTapeStatus(
                const string & tape,
#ifdef MORE_TEST
                tape::Changer & changer,
#else
                LtfsChangerInfo & changer,
#endif
                enum TapeStatus & status );

    };

}

