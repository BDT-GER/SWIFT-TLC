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
 * SlotImpl.h
 *
 *  Created on: Aug 6, 2012
 *      Author: More Zeng
 */

#pragma once

namespace tape
{

    class SlotImpl
    {
    public:
        SlotImpl(int slotID)
        : slotID_(slotID), empty_(true)
        {
        }

        virtual
        ~SlotImpl()
        {
        }

        int
        SlotID() const
        {
            return slotID_;
        }

        bool
        Empty() const
        {
            return empty_;
        }

        string
        Barcode() const
        {
            return barcode_;
        }

        void
        SetEmpty(bool empty)
        {
            empty_ = empty;
            if ( empty_ ) {
                barcode_.clear();
            }
        }

        void
        SetBarcode(const string & barcode)
        {
            barcode_ = barcode;
        }

    private:
        int slotID_;
        bool empty_;
        string barcode_;
    };

}

