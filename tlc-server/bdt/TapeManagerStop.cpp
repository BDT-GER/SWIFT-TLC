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
 * TapeManagerStop.cpp
 *
 *  Created on: Apr 25, 2013
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "TapeManagerStop.h"
#ifdef MORE_TEST
#else
#include "TapeManagerSE.h"
#endif


namespace bdt
{

    TapeManagerStop::TapeManagerStop(TapeManagerInterface * entity)
    : TapeManagerEntity(entity)
    {
    }


    TapeManagerStop::~TapeManagerStop()
    {
    }


    bool
    TapeManagerStop::CheckTapeState(const string & tape,enum TapeState state)
    {
        bool ret = TapeManagerEntity::CheckTapeState(tape,state);
        if ( ret ) {
            switch (state) {
            case TapeManagerInterface::STATE_READ:
            case TapeManagerInterface::STATE_WRITE:
                StartTape(tape);
                break;
            default:
                break;
            }
        }
        return ret;
    }


    bool
    TapeManagerStop::SetTapeState(const string & tape,enum TapeState state)
    {
#ifdef MORE_TEST
        bool ret = TapeManagerEntity::SetTapeState(tape,state);
        if ( ret ) {
            switch (state) {
            case TapeManagerInterface::STATE_READ:
            case TapeManagerInterface::STATE_WRITE:
                StartTape(tape);
                break;
            default:
                break;
            }
        }
        return ret;
#else
        int retry = 10;
        do {
            LogDebug(tape);
            if ( IsTapeStopped(tape) ) {
                LogDebug(tape);
                switch (state) {
                case TapeManagerInterface::STATE_READ:
                case TapeManagerInterface::STATE_WRITE:
                    return false;
                    break;
                default:
                    break;
                }
                bool ret = TapeManagerEntity::SetTapeStateNoLock(tape,state);
                LogDebug(tape << " " << state << ": " << ret);
                return ret;
            }
            if ( TapeManagerEntity::SetTapeState(tape,state) ) {
                LogDebug(tape);
                return true;
            }
        } while ( retry -- );
        return false;
#endif
    }

}
