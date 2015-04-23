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
 * Throttle.h
 *
 *  Created on: Dec 18, 2012
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class Throttle
    {
    public:
        Throttle(int interval,long long valve);

        virtual
        ~Throttle();

        bool
        Request(int size);

        bool
        Reset(int interval,long long valve);

    private:
        int interval_;
        long long valve_;
        long long valveMillisec_;

        boost::mutex mutex_;

        long long cumulate_;
        boost::posix_time::ptime current_;

    };

}

