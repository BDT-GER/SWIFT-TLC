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
 * Cartridge.cpp
 *
 *  Created on: Aug 9, 2012
 *      Author: Sam Chen
 */


#include "cmn.h"
#include "../Cartridge.h"


namespace tape
{

    struct Cartridge::Detail
    {
        string changerSetting;
        string setting;

        bool GetTape(LtfsTapeInfo & tape, Error & error)
        {
            LtfsError err;
            vector<LtfsTapeInfo> tapes;
            if ( ! LtfsLibraries::Instance()->GetTapeList(
                    changerSetting, tapes, err ) ) {
                LtfsLogError("Failed to get tapes from HAL: " << changerSetting);
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        string("No such tape cartridge: ") + setting );
                return false;
            }
            for ( vector<LtfsTapeInfo>::iterator i = tapes.begin();
                    i != tapes.end();
                    ++ i ) {
                if ( i->mBarcode == setting ) {
                    tape = *i;
                    return true;
                }
            }
            LtfsLogError("No such tape in HAL: "
                    << changerSetting << ":" << setting);
            error.SetError(
                    Error::ERROR_NO_ENTRY,
                    string("No such tape cartridge: ") + setting );
            return false;
        }
    };


    Cartridge::Cartridge(const string & setting)
    : detail_(new Detail())
    {
        istringstream is(setting);
        string changerSetting;
        string tapeSetting;
        is >> changerSetting >> tapeSetting;

        detail_->setting = tapeSetting;
        detail_->changerSetting = changerSetting;
    }


    Cartridge::Cartridge(const Cartridge & cartridge)
    : detail_(new Detail())
    {
        detail_->setting = cartridge.detail_->setting;
        detail_->changerSetting = cartridge.detail_->changerSetting;
    }


    void
    Cartridge::Swap(Cartridge & cartridge)
    {
        swap( detail_, cartridge.detail_ );
    }


    Cartridge::~Cartridge()
    {
        delete detail_;
    }


    bool
    Cartridge::GetBarcode(string & barcode, Error & error)
    {
        barcode = detail_->setting;
        return true;
    }


    bool
    Cartridge::GetSlotID(int & slotID, Error & error)
    {
        LtfsTapeInfo tape;
        if(!detail_->GetTape(tape, error)){
            return false;
        }

        slotID = tape.mSlotID;
        return true;
    }


}

