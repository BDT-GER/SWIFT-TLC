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
 * TapeManagerStop.h
 *
 *  Created on: Apr 25, 2013
 *      Author: More Zeng
 */


#pragma once


#include "TapeManagerEntity.h"


namespace bdt
{

    class TapeManagerStop : public TapeManagerEntity
    {
    public:
        TapeManagerStop(TapeManagerInterface * entity);

        virtual
        ~TapeManagerStop();

        bool
        CheckTapeState( const string & tape, enum TapeState state );

        bool
        SetTapeState( const string & tape, enum TapeState state );

        bool
        StartTape( const string & tape )
        {
            stopSet_.erase(tape);
            return true;
        }

        bool
        StopTape( const string & tape )
        {
            stopSet_.insert(tape);
            return true;
        }

        bool
        IsTapeStopped( const string & tape )
        {
            set<string>::iterator i = stopSet_.find(tape);
            return (i != stopSet_.end());
        }

    private:
        set<string> stopSet_;
        boost::mutex mutex_;
    };

}

