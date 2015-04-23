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
 * Throttle.cpp
 *
 *  Created on: Dec 18, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "Throttle.h"


namespace bdt
{

    Throttle::Throttle(int interval,long long valve)
    {
        Reset(interval,valve);
    }


    Throttle::~Throttle()
    {
    }


    bool
    Throttle::Request(int size)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        if ( size <= 0 ) {
            return true;
        }
        if ( interval_ == 0 ) {
            return true;
        }

        boost::posix_time::ptime current =
                boost::posix_time::microsec_clock::local_time();
        int duration = (current - current_).total_milliseconds();
        if ( duration >= interval_ ) {
            cumulate_ = cumulate_ - duration * valveMillisec_;
            if ( cumulate_ < 0 ) {
                cumulate_ = 0;
            }
            current_ = current;
        }

        if ( cumulate_ <= valve_ ) {
            cumulate_ += size;
            return true;
        }

        int wait = (cumulate_ - valve_) / valveMillisec_;
        cumulate_ += size;
        boost::this_thread::sleep(boost::posix_time::milliseconds(wait));
        return true;
    }


    bool
    Throttle::Reset(int interval,long long valve)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        interval_ = interval;
        if ( interval_ < 0 ) {
            interval_ = 0;
        }

        valve_ = valve;
        if ( valve_ < 0 ) {
            valve_ = 0;
        }

        if ( interval_ == 0 ) {
            valveMillisec_ = 0;
        } else {
            valveMillisec_ = valve/interval;
        }

        cumulate_ = 0;
        current_ = boost::posix_time::microsec_clock::local_time();

        return true;
    }

}

