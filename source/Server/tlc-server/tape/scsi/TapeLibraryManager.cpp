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
 * TapeLibraryManager.cpp
 *
 *  Created on: Aug 9, 2012
 *      Author: Sam Chen
 */


#include "cmn.h"
#include "../TapeLibraryManager.h"
#include "../../ltfs_management/TapeLibraryMgr.h"


namespace tape
{

    TapeLibraryManager::TapeLibraryManager()
    {
    }


    TapeLibraryManager::~TapeLibraryManager()
    {
    }


    bool
    TapeLibraryManager::GetChangerList(
            vector<Changer> & changers,
            Error & error) const
    {
        changers.clear();

        vector<LtfsChangerInfo> infos;
        LtfsError err;
        if( ! LtfsLibraries::Instance()->GetChangerList(infos,err) ) {
            LtfsLogError("Failed to get changers from HAL");

            LtfsLibraries::Instance()->Refresh(err);
            if( ! LtfsLibraries::Instance()->GetChangerList(infos,err) ) {
                LtfsLogError("Failed to get changers from HAL after refresh");
                ConvertError(error, err);
                return false;
            }
        }

        for ( vector<LtfsChangerInfo>::iterator i = infos.begin();
                i != infos.end();
                ++ i ) {
            changers.push_back( i->mSerial );
        }

        return true;
    }

}
