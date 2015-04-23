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
 * TapeManager.h
 *
 *  Created on: Mar 2, 2012
 *      Author: More Zeng
 */


#pragma once


#include "TapeRedirect.h"


namespace bdt
{

    class TapeManager : public TapeManagerInterface
    {
    public:
        TapeManager();

        virtual
        ~TapeManager();

        bool
        Refresh(const fs::path & config);

        virtual enum TapeStatus
        GetTapeStatus( const string & tape );

        virtual bool
        SetTapeStatus( const string & tape, enum TapeStatus status );

        virtual bool
        GetTapesUse(vector<string> & tapes,const fs::path & path,off_t size);

        virtual bool
        GetShareAvailableTapes(const string& uuid, vector<map<string, off_t> >& tapesList);

        virtual bool GetDriveNum(int& driveNum);
        virtual bool GetTapeActivity(const string& barcode, int& act, int& percentage);

        virtual bool
        SetTapesUse(
                const vector<string> & tapes,
                int fileNumber,
                off_t fileSize,
                off_t tapeSize );

        /*virtual bool
        GetCapacity(
                const string & service,
                size_t & fileNumber,
                off_t & usedSize,
                off_t & freeSize );*/

        virtual fs::path
        GetPath( const string & tape, const fs::path & path );

    private:
        TapeRedirect redirect_;

        struct Capacity
        {
            Capacity()
            : fileNumber(0), usedSize(0), freeSize(0)
            {
                freeSize = 1024LL * 1024 * 1024 * 1024;
            }
            size_t fileNumber;
            off_t usedSize;
            off_t freeSize;
        };
        typedef map<string,Capacity> TapeCapacity;
        TapeCapacity capacity_;

        bool
        SetDefaultTape( const string & tape );

    };

}

