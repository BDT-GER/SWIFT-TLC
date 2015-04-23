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
 * CIFSWait.cpp
 *
 *  Created on: Apr 28, 2012
 *      Author: More Zeng
 */


#include "CIFSWait.h"


namespace bdt
{

    CIFSWait::CIFSWait(int wait)
    : wait_(wait), speed_(0)
    {
    }


    CIFSWait::~CIFSWait()
    {
    }

    int
    CIFSWait::GetWait(int size)
    {
        if ( 0 == wait_ ) {
            return 0;
        }

        if ( 0 == speed_ ) {
            return wait_;
        }

        return wait_ + size / speed_;
    }


    void
    CIFSWait::SetWait(int size,int wait)
    {
        if ( 0 == wait ) {
            return;
        }

        if ( 0 == size ) {
            wait_ = wait;
            return;
        }

        speed_ = size / wait;
        return;
    }

}
