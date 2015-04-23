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
 * ObserverSubject.h
 *
 *  Created on: Jul 4, 2012
 *      Author: More Zeng
 */


#pragma once


#include <boost/bind.hpp>

#include "ObserverListener.h"


namespace tape
{

    class ObserverSubject
    {
    public:
        ObserverSubject()
        {
        }

        virtual
        ~ObserverSubject()
        {
        }

        bool
        Attach(ObserverListener * listener)
        {
            boost::lock_guard<boost::mutex> lock(mutex_);

            return listeners_.insert(listener).second;
        }

        bool
        Detach(ObserverListener * listener)
        {
            boost::lock_guard<boost::mutex> lock(mutex_);

            listeners_.erase(listener);

            return true;
        }

        void
        Notify()
        {
            boost::lock_guard<boost::mutex> lock(mutex_);

            for_each( listeners_.begin(), listeners_.end(),
                    boost::bind( &ObserverListener::Update, _1 ) );
        }

    private:
        set<ObserverListener *> listeners_;
        boost::mutex mutex_;
    };

}

