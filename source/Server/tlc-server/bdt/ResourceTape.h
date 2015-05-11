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
 * ResourceTape.h
 *
 *  Created on: Mar 21, 2013
 *      Author: More Zeng
 */


#pragma once


namespace bdt
{

    class ResourceTape
    {
    public:
        ResourceTape()
        {
        }

        virtual
        ~ResourceTape()
        {
        }

        enum {
            START_RETURN_SUCCESS = 0,
            START_RETURN_WAIT,
            START_RETURN_BUSY,
            START_RETURN_NO_RESOURCE,
            START_RETURN_NO_USE,
        };

        typedef map<string,vector<string> > TapesInUseMap;

        virtual int
        StartTapes(
                const vector<string> & tapes,
                TapesInUseMap & tapesInUse, int priority) = 0;

        int
        StartTape(const string & tape,vector<string> & tapesInUse, int priority)
        {
            tapesInUse.clear();

            vector<string> tapes;
            tapes.push_back(tape);
            TapesInUseMap mapTapesInUse;

            int ret = StartTapes(tapes,mapTapesInUse, priority);
            assert( mapTapesInUse.size() <= 1 );

            if ( ! mapTapesInUse.empty() ) {
                tapesInUse = mapTapesInUse.begin()->second;
            }
            return ret;
        }

        virtual bool
        StopTapes(const vector<string> & tapes) = 0;

        bool
        StopTape(const string & tape)
        {
            vector<string> tapes;
            tapes.push_back(tape);

            return StopTapes(tapes);
        }

        virtual bool
        MountTapes(const vector<string> & tapes) = 0;

        bool
        MountTape(const string & tape)
        {
            vector<string> tapes;
            tapes.push_back(tape);

            return MountTapes(tapes);
        }

        virtual bool
        UnMountTapes(const vector<string> & tapes) = 0;

        bool
        UnMountTape(const string & tape)
        {
            vector<string> tapes;
            tapes.push_back(tape);

            return UnMountTapes(tapes);
        }

    protected:

        virtual bool
        Refresh() = 0;

    };

}

