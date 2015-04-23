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
 * BackendTask.h
 *
 *  Created on: Aug 31, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class BackendTask
    {
    public:
        BackendTask()
        : run_(false)
        {
        }

        virtual
        ~BackendTask()
        {
            while ( run_ ) {
                boost::this_thread::sleep(boost::posix_time::seconds(1));
            }
        }

        virtual void
        Start() = 0;

    protected:
        bool run_;
    };

}

