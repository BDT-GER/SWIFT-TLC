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
 * Drive.h
 *
 *  Created on: Jul 2, 2012
 *      Author: More Zeng
 */

#pragma once


#include "Slot.h"
#include "LTFSHistory.h"
#include "LTFSRepairResult.h"


namespace tape
{

    class Drive
    {
    public:
        Drive(const string & setting);

        Drive(const Drive & drive);

        Drive &
        operator = (const Drive & drive)
        {
            Drive temp(drive);
            Swap(temp);
            return * this;
        }

        void
        Swap( Drive & drive);

        virtual
        ~Drive();

        bool
        GetSlotID(int & slotID, Error & error) const;

        bool
        GetEmpty(bool & empty, Error & error) const;

        bool
        GetBarcode(string & barcode, Error & error) const;

        bool
        Mount(Error & error);

        bool
        Unmount(Error & error);

        bool
        Format(Error & error);

        bool
        GetHistoryList(vector<LTFSHistory> histories, Error & error);

        bool
        RollbackHistory(const LTFSHistory & history, Error & error);

        bool
        Repair(bool repair, LTFSRepairResult & result, Error & error);

        static fs::path
        GetLTFSFolder();

    private:
        struct Detail;
        Detail * detail_;

    };

}
