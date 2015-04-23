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
 * TapeManager.cpp
 *
 *  Created on: Mar 2, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "TapeManager.h"


namespace bdt
{

    TapeManager::TapeManager()
    {
#ifdef MORE_TEST
        Refresh("/tmp/bdt.config");
#endif
    }


    TapeManager::~TapeManager()
    {
    }


    bool
    TapeManager::Refresh(const fs::path & pathConfig)
    {
        redirect_.Clear();

        ifstream config(pathConfig.string().c_str());
        char buffer[1024];
        while ( config.getline(buffer,sizeof(buffer)) ) {

            string input(buffer);
            boost::trim(input);
            vector<string> items;
            boost::split(items,input,boost::is_any_of(" \t"));
            if ( items.size() < 2 ) {
                continue;
            }

            string tape = items[0];
            if ( tape.empty() || tape[0] == '#' ) {
                continue;
            }

            int mode = -1;
            try {
                mode = boost::lexical_cast<int>(items[1]);
            } catch(...) {
            }
            if ( mode < 0 ) {
                LogWarn( "Invalid input:" << input );
                continue;
            }

            if ( ! redirect_.AppendRedirect(
                    tape,
                    mode,
                    items.size() > 2 ? items[2] : "") ) {
                LogWarn( "Invalid input:" << input );
            }
        }

        return true;
    }


    bool
    TapeManager::SetTapeStatus( const string & tape, enum TapeStatus status )
    {
        return false;
    }


    enum TapeManagerInterface::TapeStatus
    TapeManager::GetTapeStatus( const string & tape )
    {
        return STATUS_UNKNOWN;
    }

    bool
    TapeManager::GetShareAvailableTapes(const string& uuid, vector<map<string, off_t> >& tapesList)
    {
    	return false;
    }


    bool
    TapeManager::GetDriveNum(int& driveNum)
    {
        return false;
    }
    bool TapeManager::GetTapeActivity(const string& barcode, int& act, int& percentage)
    {
    	return false;
    }


    bool
    TapeManager::GetTapesUse(
            vector<string> & tapes, const fs::path & path, off_t size )
    {
        tapes.clear();
        string content = redirect_.GetTape(path);
        if ( content.empty() ) {
            return false;
        }
        boost::split(tapes,content,boost::is_any_of(","));

        TapeCapacity::iterator i = capacity_.insert(
                TapeCapacity::value_type( tapes[0],Capacity() ) ).first;
        return i->second.freeSize >= size;
    }


    bool
    TapeManager::SetTapesUse(
            const vector<string> & tapes,
            int fileNumber,
            off_t fileSize,
            off_t tapeSize )
    {
        TapeCapacity::iterator i = capacity_.insert(
                TapeCapacity::value_type( tapes[0],Capacity() ) ).first;
        if ( (fileNumber < 0)
                && ((int)i->second.fileNumber <= abs(fileNumber)) ) {
            i->second.fileNumber = 0;
        } else {
            i->second.fileNumber += fileNumber;
        }
        if ( (fileSize < 0) && (i->second.usedSize <= abs(fileSize)) ) {
            fileSize = - i->second.usedSize;
        }
        i->second.usedSize += fileSize;
        i->second.freeSize -= fileSize;
        return true;
    }


    bool
    TapeManager::SetDefaultTape( const string & tape )
    {
        return redirect_.AppendRedirect(tape,TapeRedirect::MODE_DEFAULT);
    }


    /*bool
    TapeManager::GetCapacity(
            const string & service,
            size_t & fileNumber,
            off_t & usedSize,
            off_t & freeSize )
    {
        string tape;
        fs::path servicePath = fs::path("/") / service / "/";
        if ( GetTapeUse(tape,servicePath,0) ) {
            LogDebug(service << " : " << tape);
        } else {
            tape = service;
            LogDebug(tape);
        }

        TapeCapacity::iterator i = capacity_.insert(
                TapeCapacity::value_type( tape,Capacity() ) ).first;
        fileNumber = i->second.fileNumber;
        usedSize = i->second.usedSize;
        freeSize = i->second.freeSize;

        return true;
    }*/


    fs::path
    TapeManager::GetPath( const string & tape, const fs::path & path )
    {
        return fs::path(tape) / path;
    }

}
