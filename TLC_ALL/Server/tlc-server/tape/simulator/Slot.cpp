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
 * Slot.cpp
 *
 *  Created on: Jul 2, 2012
 *      Author: More Zeng
 */

#include "../stdafx.h"
#include "../Slot.h"

#include "Factory.h"


namespace tape
{

    struct Slot::Detail
    {
        int changer;
        int slot;

        SlotImpl *
        GetSlotImpl(Error & error)
        {
            SlotImpl * impl = Factory::Instance()->GetSlot(changer,slot);
            if ( NULL == impl ) {
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        string("No such slot ")
                        + boost::lexical_cast<string>(slot)
                        + " in changer "
                        + boost::lexical_cast<string>(changer) );
            }
            return impl;
        }
    };


    Slot::Slot(const string & setting)
    : detail_(new Detail())
    {
        istringstream is(setting);
        int changer;
        int slot;
        is >> changer >> slot;

        detail_->changer = changer;
        detail_->slot = slot;
    }


    Slot::Slot(const Slot & slot)
    : detail_(new Detail())
    {
        detail_->changer = slot.detail_->changer;
        detail_->slot = slot.detail_->slot;
    }


    void
    Slot::Swap( Slot & slot)
    {
        swap( detail_, slot.detail_ );
    }


    Slot::~Slot()
    {
        delete detail_;
    }


    bool
    Slot::GetSlotID(int & slotID, Error & error) const
    {
        SlotImpl * impl = detail_->GetSlotImpl(error);
        if ( NULL == impl ) {
            return false;
        }

        slotID = impl->SlotID();
        return true;
    }


    bool
    Slot::GetEmpty(bool & empty, Error & error) const
    {
        SlotImpl * impl = detail_->GetSlotImpl(error);
        if ( NULL == impl ) {
            return false;
        }

        empty = impl->Empty();
        return true;
    }


    bool
    Slot::GetBarcode(string & barcode, Error & error) const
    {
        SlotImpl * impl = detail_->GetSlotImpl(error);
        if ( NULL == impl ) {
            return false;
        }

        barcode = impl->Barcode();
        return true;
    }

}

