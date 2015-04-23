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
 * CartridgeImpl.h
 *
 *  Created on: Aug 3, 2012
 *      Author: More Zeng
 */

#pragma once

namespace tape
{

    class CartridgeImpl
    {
    public:
        CartridgeImpl(int changer,int number)
        : slotID_(0)
        {
            ostringstream os;
            os << setw(2) << setfill('0') << changer
                    << setw(6) << setfill('0') << number;
            barcode_ = os.str();
        }

        virtual
        ~CartridgeImpl()
        {
        }

        string
        Barcode()
        {
            return barcode_;
        }

        int
        SlotID()
        {
            return slotID_;
        }

        void
        SetSlotID(int slotID)
        {
            slotID_ = slotID;
        }

    private:
        string barcode_;
        int slotID_;
    };

}

