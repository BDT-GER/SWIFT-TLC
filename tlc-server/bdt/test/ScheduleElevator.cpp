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
 * ScheduleElevator.cpp
 *
 *  Created on: May 7, 2012
 *      Author: More Zeng
 */

#include "../stdafx.h"
#include "ScheduleElevator.h"


namespace bdt
{

    ScheduleElevator::ScheduleElevator(int delayElevator,int delayFile)
    : delayElevator_(delayElevator), delayFile_(delayFile)
    {
        assert( delayFile_ >= delayElevator );
    }


    ScheduleElevator::~ScheduleElevator()
    {
    }


    bool
    ScheduleElevator::Request(
            const fs::path & path, off_t offset, size_t size,
            int timeout, int priority )
    {
        offset += 1024;

        RequestMap::iterator i = requests_.find(path);
        if ( (i != requests_.end()) && (i->second >= 0) ) {
            boost::this_thread::sleep(boost::posix_time::milliseconds(timeout));
            return false;
        }

        if ( current_ != path ) {
            if ( current_.empty() ) {
                current_ = path;
            } else {
                current_ = path;
                requests_[path] = offset + size;
                boost::this_thread::sleep(
                        boost::posix_time::milliseconds(delayFile_) );
                return true;
            }
        }

        if ( i != requests_.end() ) {
            if ( offset < -(i->second) ) {
                boost::this_thread::sleep(
                        boost::posix_time::milliseconds(delayElevator_) );
            }
        }

        requests_[path] = offset + size;
        return true;
    }


    bool
    ScheduleElevator::Request(const fs::path & path,int timeout,int priority)
    {
        RequestMap::iterator i = requests_.find(path);
        if ( i == requests_.end() ) {
            requests_[path] = 0;
            return true;
        } else if ( i->second < 0 ) {
            requests_[path] = 0;
            return true;
        } else {
            boost::this_thread::sleep(boost::posix_time::milliseconds(timeout));
            return false;
        }
    }


    void
    ScheduleElevator::Release(const fs::path & path,bool isTape)
    {
        RequestMap::iterator i = requests_.find(path);
        if ( i != requests_.end() ) {
            if ( i->second == 0 ) {
                assert( true == isTape );
                requests_.erase(i);
            } else {
                assert( false == isTape );
                assert( i->second > 0 );
                i->second = -(i->second);
            }
        }
    }

}

