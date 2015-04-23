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
 * Changer.h
 *
 *  Created on: Jul 2, 2012
 *      Author: More Zeng
 */

#pragma once


#include "Drive.h"
#include "Cartridge.h"
#include "Slot.h"


namespace tape
{

    class Changer
    {
    public:
        Changer(const string & setting);

        Changer(const Changer & changer);

        Changer &
        operator = (const Changer & changer)
        {
            Changer temp(changer);
            Swap(temp);
            return * this;
        }

        void
        Swap( Changer & changer);

        virtual
        ~Changer();

        bool
        GetDriveList(vector<Drive> & drives, Error & error) const;

        bool
        GetSlotList(vector<Slot> & slots, Error & error) const;

        bool
        GetCartridgeList(vector<Cartridge> & cartridges, Error & error) const;

        bool
        MoveCartridge(
                Slot & slot,
                Cartridge & cartridge,
                Error & error);

        bool
        LoadCartridge(
                Drive & drive,
                Cartridge & cartridge,
                Error & error);

    private:
        struct Detail;
        Detail * detail_;

    };

}
