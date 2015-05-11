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
 * Changer.cpp
 *
 *  Created on: Jul 2, 2012
 *      Author: More Zeng
 */

#include "../stdafx.h"
#include "../Changer.h"

#include "Factory.h"


namespace tape
{

    struct Changer::Detail
    {
        int changer;
        boost::mutex mutex;

        bool
        MoveCartridge(
                int slotIDSrc,
                int slotIDDst,
                Cartridge & cartridge,
                Error & error)
        {
            if ( slotIDSrc == slotIDDst ) {
                return true;
            }

            if ( fs::exists( "/tmp/.bdt.move_error") ) {
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        "Force error for debug" );
                return false;
            }

            string barcode;
            if ( ! cartridge.GetBarcode(barcode,error) ) {
                return false;
            }

            Factory * factory = Factory::Instance();

            SlotImpl * slotSrc = factory->GetSlot(changer,slotIDSrc);
            if ( NULL == slotSrc ) {
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        string("No such slot(drive) ")
                        + boost::lexical_cast<string>(slotIDSrc)
                        + " in changer "
                        + boost::lexical_cast<string>(changer) );
                return false;
            }

            SlotImpl * slotDst = factory->GetSlot(changer,slotIDDst);
            if ( NULL == slotDst ) {
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        string("No such slot(drive) ")
                        + boost::lexical_cast<string>(slotIDDst)
                        + " in changer "
                        + boost::lexical_cast<string>(changer) );
                return false;
            }

            CartridgeImpl * cartridgeImpl = factory->GetCartridge(barcode);
            if ( NULL == slotDst ) {
                error.SetError(
                        Error::ERROR_NO_ENTRY,
                        string("No such cartridge ")
                        + barcode
                        + " in changer "
                        + boost::lexical_cast<string>(changer) );
                return false;
            }

            if ( ! slotDst->Empty() ) {
                error.SetError(
                        Error::ERROR_ENTRY_FULL,
                        string("Slot(Drive) ")
                        + boost::lexical_cast<string>(slotIDDst)
                        + " is full in changer "
                        + boost::lexical_cast<string>(changer) );
                return false;
            }

            //boost::this_thread::sleep(boost::posix_time::seconds(1));

            slotDst->SetEmpty(false);
            slotDst->SetBarcode(barcode);

            slotSrc->SetEmpty(true);

            cartridgeImpl->SetSlotID(slotIDDst);

            return true;
        }

    };


    Changer::Changer(const string & setting)
    : detail_(new Changer::Detail())
    {
        detail_->changer = boost::lexical_cast<int>(setting);
    }


    Changer::Changer(const Changer & changer)
    : detail_(new Changer::Detail())
    {
        detail_->changer = changer.detail_->changer;
    }


    void
    Changer::Swap( Changer & changer)
    {
        swap( detail_, changer.detail_ );
    }


    Changer::~Changer()
    {
        delete detail_;
    }


    bool
    Changer::GetDriveList(vector<Drive> & drives, Error & error) const
    {
        boost::lock_guard<boost::mutex> lock(detail_->mutex);

        drives.clear();

        ChangerImpl * impl = Factory::Instance()->GetChanger(detail_->changer);
        int numberDrive = impl->NumberDrive();
        for ( int i = 1; i <= numberDrive; ++i ) {
            ostringstream os;
            os << detail_->changer << " " << i;

            drives.push_back( Drive(os.str()) );
        }

        return true;
    }


    bool
    Changer::GetSlotList(vector<Slot> & slots, Error & error) const
    {
        boost::lock_guard<boost::mutex> lock(detail_->mutex);

        slots.clear();

        ChangerImpl * impl = Factory::Instance()->GetChanger(detail_->changer);
        int numberSlot = impl->NumberSlot();
        for ( int i = 1; i <= numberSlot; ++i ) {
            ostringstream os;
            os << detail_->changer << " " << i;

            slots.push_back( Slot(os.str()) );
        }

        return true;
    }


    bool
    Changer::GetCartridgeList(vector<Cartridge> & cartridges, Error & error)
    const
    {
        boost::lock_guard<boost::mutex> lock(detail_->mutex);

        cartridges.clear();

        ChangerImpl * impl = Factory::Instance()->GetChanger(detail_->changer);
        int numberCartridge = impl->NumberCartridge();
        for ( int i = 1; i <= numberCartridge; ++i ) {
            ostringstream os;
            os << setw(2) << setfill('0') << detail_->changer
                    << setw(6) << setfill('0') << i;

            cartridges.push_back( Cartridge(os.str()) );
        }

        return true;
    }


    bool
    Changer::MoveCartridge(
            Slot & slot,
            Cartridge & cartridge,
            Error & error)
    {
        boost::lock_guard<boost::mutex> lock(detail_->mutex);

        int slotIDSrc;
        if ( ! cartridge.GetSlotID(slotIDSrc,error) ) {
            return false;
        }
        int slotIDDst;
        if ( ! slot.GetSlotID(slotIDDst,error) ) {
            return false;
        }

        return detail_->MoveCartridge(slotIDSrc,slotIDDst,cartridge,error);
    }


    bool
    Changer::LoadCartridge(
            Drive & drive,
            Cartridge & cartridge,
            Error & error)
    {
        boost::lock_guard<boost::mutex> lock(detail_->mutex);

        int slotIDSrc;
        if ( ! cartridge.GetSlotID(slotIDSrc,error) ) {
            return false;
        }
        int slotIDDst;
        if ( ! drive.GetSlotID(slotIDDst,error) ) {
            return false;
        }

        return detail_->MoveCartridge(slotIDSrc,slotIDDst,cartridge,error);
    }

}

