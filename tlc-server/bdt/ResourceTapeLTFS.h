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
 * ResourceTapeLTFS.h
 *
 *  Created on: Mar 21, 2013
 *      Author: More Zeng
 */


#pragma once


#include "ResourceTape.h"
#include "../ltfs_management/TapeLibraryMgr.h"


namespace bdt
{

    class ResourceTapeLTFS
    : public ResourceTape
    {
    public:
        ResourceTapeLTFS()
        {
        }

        virtual
        ~ResourceTapeLTFS()
        {
        }

        int
        StartTapes(const vector<string> & tapes,TapesInUseMap & tapesInUse, int priority)
        {
            LogDebug("Start tapes: " << boost::join(tapes,","));
            int ret = ltfs_management::TapeLibraryMgr::Instance()->SchRequestTapes(tapes,tapesInUse, priority);
            switch ( ret ) {
            case SCH_SUCCESS:
                return START_RETURN_SUCCESS;
            case SCH_WAIT:
                return START_RETURN_WAIT;
            case SCH_BUSY:
                return START_RETURN_BUSY;
            case SCH_NO_RESOURCE:
                return START_RETURN_NO_RESOURCE;
            default:
                LogError(ret);
                return START_RETURN_WAIT;
            }
        }

        bool
        StopTapes(const vector<string> & tapes)
        {
            LogDebug("Stop tapes: " << boost::join(tapes,","));
            return ltfs_management::TapeLibraryMgr::Instance()->SchReleaseTapes(tapes);
        }

        typedef map<string,bool> TapeMap;

        bool
        MountTapes(const vector<string> & tapes)
        {
            LogDebug("Mount tapes: " << boost::join(tapes,","));

            boost::thread_group group;
            TapeMap tapeMap;

            BOOST_FOREACH( const string & tape, tapes ) {
                TapeMap::iterator i = tapeMap.insert( TapeMap::value_type(
                        tape, false ) ).first;
                group.create_thread( boost::bind(
                        &ResourceTapeLTFS::MountTapeThread, this,
                        tape, &i->second ) );
            }

            group.join_all();

            bool ret = true;
            BOOST_FOREACH( TapeMap::value_type & pair, tapeMap ) {
                if ( ! pair.second ) {
                    ret = false;
                }
            }
            LogDebug("Mount tapes " << boost::join(tapes,",") << ": " << ret);
            return ret;
        }

        bool
        UnMountTapes(const vector<string> & tapes)
        {
            LogDebug("UnMount tapes: " << boost::join(tapes,","));

            boost::thread_group group;
            TapeMap tapeMap;

            BOOST_FOREACH( const string & tape, tapes ) {
                TapeMap::iterator i = tapeMap.insert( TapeMap::value_type(
                        tape, false ) ).first;
                group.create_thread( boost::bind(
                        &ResourceTapeLTFS::UnMountTapeThread, this,
                        tape, &i->second ) );
            }

            group.join_all();

            bool ret = true;
            BOOST_FOREACH( TapeMap::value_type & pair, tapeMap ) {
                if ( ! pair.second ) {
                    ret = false;
                }
            }
            LogDebug("UnMount tapes " << boost::join(tapes,",") << ": " << ret);
            return ret;
        }

    private:

        bool
        Refresh()
        {
            return true;
        }

        void
        MountTapeThread(const string & tape,bool * result)
        {
            LogDebug("Mount tape " << tape);
            *result = ltfs_management::TapeLibraryMgr::Instance()->MountTape(
                    tape );
            LogDebug("Mount tape " << tape << ": " << *result);
        }

        void
        UnMountTapeThread(const string & tape,bool * result)
        {
            LogDebug("UnMount tape " << tape);
            *result = ltfs_management::TapeLibraryMgr::Instance()->UnmountTape(
                    tape );
            LogDebug("UnMount tape " << tape << ": " << *result);
        }

    };

}

