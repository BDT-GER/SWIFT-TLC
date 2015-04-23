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
 * PriorityTapeGroup.cpp
 *
 *  Created on: May 22, 2013
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "PriorityTape.h"
#include "PriorityTapeGroup.h"


namespace bdt
{

    PriorityTapeGroup::PriorityTapeGroup(
            const vector<string> & tapes,
            const vector<PriorityTape *> & priorities )
    : tapes_(tapes)
    {
        assert(tapes.size() == priorities.size());

        BOOST_FOREACH( PriorityTape * priority, priorities ) {
            priorities_.insert( PriorityMap::value_type( priority, false ) );
        }
    }

    PriorityTapeGroup::~PriorityTapeGroup()
    {
    }


    void
    PriorityTapeGroup::RequestThread(
            PriorityTape * priority, bool share, int timeout, int number,
            bool * result)
    {
        * result = priority->Request(tapes_,share,timeout,number);
    }


    bool
    PriorityTapeGroup::Request(bool share,int timeout,int priority)
    {
        boost::thread_group group;
        BOOST_FOREACH( PriorityMap::value_type & pair, priorities_ ) {
            group.create_thread( boost::bind(
                    &PriorityTapeGroup::RequestThread, this,
                    pair.first, share, timeout, priority, &pair.second ) );
        }

        try {
            group.join_all();
        } catch ( const boost::thread_interrupted & e ) {
            LogWarn("Interrupt threads " << group.size());
            group.interrupt_all();
            group.join_all();
        }

        bool ret = true;
        BOOST_FOREACH( PriorityMap::value_type & pair, priorities_ ) {
            if ( ! pair.second ) {
                LogDebug("Failure to request " << pair.first);
                ret = false;
            }
        }

        if ( ! ret ) {
            BOOST_FOREACH( PriorityMap::value_type & pair, priorities_ ) {
                if ( pair.second ) {
                    LogWarn("Release previous request " << pair.first);
                    pair.first->Release(share);
                }
            }
        }

        return ret;
    }


    bool
    PriorityTapeGroup::Enable(bool enable)
    {
        bool ret = true;
        BOOST_FOREACH( PriorityMap::value_type & pair, priorities_ ) {
            ret = pair.first->Enable(enable) && ret;
        }
        return ret;
    }


    void
    PriorityTapeGroup::Release(bool share)
    {
        BOOST_FOREACH( PriorityMap::value_type & pair, priorities_ ) {
            pair.first->Release(share);
        }
    }


}
