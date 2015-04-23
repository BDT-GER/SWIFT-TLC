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
 *  Created on: Jul 2, 2012
 *      Author: More Zeng
 */

#include "../stdafx.h"
#include "../Cartridge.h"

#include "Factory.h"


namespace tape
{

    struct Cartridge::Detail
    {
        string barcode;

        CartridgeImpl *
        GetCartridgeImpl(Error & error)
        {
            CartridgeImpl * impl = Factory::Instance()->GetCartridge(barcode);
            if ( NULL == impl ) {
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        string("No such tape cartridge: ") + barcode );
            }
            return impl;
        }
    };


    Cartridge::Cartridge(const string & setting)
    : detail_(new Detail())
    {
        detail_->barcode = setting;
    }


    Cartridge::Cartridge(const Cartridge & cartridge)
    : detail_(new Detail())
    {
        detail_->barcode = cartridge.detail_->barcode;
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
        CartridgeImpl * impl = detail_->GetCartridgeImpl(error);
        if ( NULL == impl ) {
            return false;
        }

        barcode = impl->Barcode();
        return true;
    }


    bool
    Cartridge::GetSlotID(int & slotID, Error & error)
    {
        CartridgeImpl * impl = detail_->GetCartridgeImpl(error);
        if ( NULL == impl ) {
            return false;
        }

        slotID = impl->SlotID();
        return true;
    }


}

