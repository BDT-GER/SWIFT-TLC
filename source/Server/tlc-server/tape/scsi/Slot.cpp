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
 *  Created on: Aug 9, 2012
 *      Author: Sam Chen
 */


#include "cmn.h"
#include "../Slot.h"


namespace tape
{

    struct Slot::Detail
    {
        string changerSetting;
        string setting;

        bool GetSlot(LtfsSlotInfo & slot, Error & error)
        {
            LtfsError err;
            vector<LtfsSlotInfo> slots;
            if ( ! LtfsLibraries::Instance()->GetSlotList(
                    changerSetting, slots, err ) ) {
                LtfsLogError("Failed to get slots from HAL: " << changerSetting);
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        string("No such slot: ") + setting );
                return false;
            }
            for ( vector<LtfsSlotInfo>::iterator i = slots.begin();
                    i != slots.end();
                    ++ i ) {
                if ( i->mSlotID == boost::lexical_cast<int>(setting) ) {
                    slot = *i;
                    return true;
                }
            }
            vector<LtfsMailSlotInfo> mailslots;
            if ( ! LtfsLibraries::Instance()->GetMailSlotList(
                    changerSetting, mailslots, err ) ) {
                LtfsLogError("Failed to get mailslots from HAL: "
                        << changerSetting);
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        string("No such mailslot: ") + setting );
                return false;
            }
            for ( vector<LtfsMailSlotInfo>::iterator i = mailslots.begin();
                    i != mailslots.end();
                    ++ i ) {
                if ( i->mSlotID == boost::lexical_cast<int>(setting) ) {
                    slot.mSlotID = i->mSlotID;
                    slot.mIsEmpty = i->mIsEmpty;
                    slot.mBarcode = i->mBarcode;
                    return true;
                }
            }

            LtfsLogError("No such slot in HAL: "
                    << changerSetting << ":" << setting);
            error.SetError(
                    Error::ERROR_NO_ENTRY,
                    string("No such slot: ") + setting );
            return false;
        }
    };


    Slot::Slot(const string & setting)
    : detail_(new Detail())
    {
        istringstream is(setting);
        string changerSetting;
        string tapeSetting;
        is >> changerSetting >> tapeSetting;

        detail_->setting = tapeSetting;
        detail_->changerSetting = changerSetting;
    }


    Slot::Slot(const Slot & slot)
    : detail_(new Detail())
    {
        detail_->setting = slot.detail_->setting;
        detail_->changerSetting = slot.detail_->changerSetting;
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
        slotID = boost::lexical_cast<int>(detail_->setting);
        return true;
    }


    bool
    Slot::GetEmpty(bool & empty, Error & error) const
    {
        LtfsSlotInfo slot;
        if(!detail_->GetSlot(slot, error)){
            return false;
        }

        empty = slot.mIsEmpty;
        return true;
    }


    bool
    Slot::GetBarcode(string & barcode, Error & error) const
    {
        LtfsSlotInfo slot;
        if(!detail_->GetSlot(slot, error)){
            return false;
        }

        barcode = slot.mBarcode;
        return true;
    }

}

