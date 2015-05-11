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
 * ScheduleElevator.h
 *
 *  Created on: May 7, 2012
 *      Author: More Zeng
 */

#pragma once


#include "../ScheduleInterface.h"


namespace bdt
{

    class ScheduleElevator : public ScheduleInterface
    {
    public:
        ScheduleElevator(int delayElevator,int delayFile);

        virtual
        ~ScheduleElevator();

        bool
        Request(const fs::path & path,off_t offset,size_t size,
                int timeout,int priority);

        bool
        Request(const fs::path & path,int timeout,int priority);

        void
        Release(const fs::path & path,bool isTape);

    private:
        int delayElevator_;
        int delayFile_;

        fs::path current_;
        typedef map<fs::path,off_t> RequestMap;
        RequestMap requests_;
    };

}

