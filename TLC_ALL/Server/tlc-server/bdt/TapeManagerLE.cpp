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
 * TapeManagerLE.cpp
 *
 *  Created on: Mar 6, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "TapeManagerLE.h"


namespace bdt
{

    TapeManagerLE::TapeManagerLE()
    : folder_(Factory::GetTapeFolder()), time_(0)
    {
    }


    TapeManagerLE::~TapeManagerLE()
    {
    }


    //  not thread safety
    bool
    TapeManagerLE::RefreshTapeStatus()
    {
        time_t current = time(NULL);
        if ( current <= time_ + 1 ) {
            return true;
        }

        tape_.clear();

        ifstream fs("/tmp/ltfs_tape_info.txt");
        char buffer[1024];
        while ( memset(buffer,0,sizeof(buffer))
                && fs.getline(buffer,sizeof(buffer)) ) {
            string tape = buffer;
            string status = buffer + 9;
            if ( status.npos == status.find("Valid LTFS") ) {
                tape_[tape] = STATUS_CORRUPT;
            }
            else if ( status.npos == status.find("Data transfer element") ) {
                tape_[tape] = STATUS_OFFLINE;
            } else {
                tape_[tape] = STATUS_ONLINE;
            }
        }

        time_ = current;

        return true;
    }


    enum TapeManagerInterface::TapeStatus
    TapeManagerLE::GetTapeStatus( const string & tape )
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        fs::path folderTape = folder_ / tape;

        RefreshTapeStatus();

        TapeStatusMap::iterator i = tape_.find(tape);
        if ( tape_.end() == i ) {
            return STATUS_EXPORT;
        }
        return i->second;
    }


    fs::path
    TapeManagerLE::GetPath( const string & tape, const fs::path & path )
    {
        return folder_ / tape / path;
    }

}
