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
 * ResourceTapeSimulator.h
 *
 *  Created on: 2013-3-21
 *      Author: more
 */


#pragma once


#include "../ResourceTape.h"


namespace bdt
{

    class ResourceTapeSimulator
    : public ResourceTape, private tape::ObserverListener
    {
    public:
        ResourceTapeSimulator();

        virtual
        ~ResourceTapeSimulator();

        int
        StartTapes(
                const vector<string> & tapes,
                TapesInUseMap & tapesInUse,
                int priority);

        bool
        StopTapes(const vector<string> & tapes);

        bool
        MountTapes(const vector<string> & tapes);

        bool
        UnMountTapes(const vector<string> & tapes);

    private:
        virtual void
        Update();

        tape::TapeLibraryManager * changer_;

        enum {
            DRIVE_STATE_STOPPED = 0,
            DRIVE_STATE_STARTING,
            DRIVE_STATE_STARTED,
        };
        struct ChangerResource
        {
            vector<string> drive;
            vector<int> state;
            int driveBusy;

            ChangerResource()
            : driveBusy(-1)
            {
            }
        };
        vector<ChangerResource> resources_;

        bool
        Refresh();

        void
        ClearPriorities();

        void
        RefreshChanger(size_t changerIndex, tape::Changer & changer);

        bool
        LoadTape( boost::unique_lock<boost::mutex> & lock,
                size_t changer, size_t drive, const string & barcode );

        bool
        LoadTapeTask(size_t changer,const string & barcode,bool reset);

        bool
        UnloadTapeTask(size_t changer,bool isExport,bool reset);

    };

}

