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
 * TapeLibraryManager.cpp
 *
 *  Created on: Jul 2, 2012
 *      Author: More Zeng
 */

#include "../stdafx.h"
#include "../TapeLibraryManager.h"

#include "Factory.h"


namespace tape
{

    struct TapeLibraryManager::Detail
    {
        Detail(TapeLibraryManager * manager)
        : manager_(manager)
        {
            Refresh();
            thread_.reset(new boost::thread(&Detail::BackendThread,this));
        }

        ~Detail()
        {
            thread_->interrupt();
            thread_->join();
        }

        void
        BackendThread()
        {
            try {
                while(true) {
                    boost::this_thread::sleep(
                            boost::posix_time::milliseconds(100) );
                    Refresh();
                    manager_->Notify();
                }
            } catch ( const boost::thread_interrupted & e ) {
            }
        }

        void
        Refresh()
        {
            string configFilename = "/tmp/TapeLibraryManager.cfg";

            boost::lock_guard<boost::mutex> lock(mutex);

            if ( ! fs::exists(configFilename) ) {
                if ( 0 == Factory::Instance()->GetChangerTotalNumber() ) {
                    AddChanger(1,2,16,8);
                    AddChanger(2,1,16,8);
                }
                return;
            }

            Factory::Destroy();

            ifstream config(configFilename.c_str());
            int numberChanger = 0;
            config >> numberChanger;
            for ( int i = 1; i <= numberChanger; ++i ) {
                int numberDrive, numberSlot, numberCartridge;
                config >> numberDrive >> numberSlot >> numberCartridge;
                AddChanger(i,numberDrive,numberSlot,numberCartridge);
            }
        }

        boost::mutex mutex;

    private:
        TapeLibraryManager * manager_;
        auto_ptr<boost::thread> thread_;

        void AddChanger(int changer,
                int numberDrive, int numberSlot, int numberCartridge)
        {
            Factory::Instance()->AddChanger(changer);

            for ( int i = 1; i<=numberSlot; ++i ) {
                Factory::Instance()->AddSlot(changer,i);
            }

            for ( int i = 1; i<=numberDrive; ++i ) {
                Factory::Instance()->AddDrive( changer, i );
            }

            for ( int i = 1; i<=numberCartridge; ++i ) {
                Factory::Instance()->AddCartridge(changer,i);
                CartridgeImpl * impl =
                        Factory::Instance()->GetCartridge(changer,i);
                string barcode = impl->Barcode();
                Factory::Instance()->GetSlot(changer,i)->SetBarcode(barcode);
                Factory::Instance()->GetSlot(changer,i)->SetEmpty(false);
                impl->SetSlotID(i);
            }

            CartridgeImpl * cartridge =
                    Factory::Instance()->GetCartridge(changer,1);
            DriveImpl * drive = Factory::Instance()->GetDrive(changer,1);
            SlotImpl * slot = Factory::Instance()->GetSlot(changer,1);
            slot->SetEmpty(true);
            drive->SetEmpty(false);
            drive->SetBarcode(cartridge->Barcode());
            cartridge->SetSlotID(drive->SlotID());
        }

    };


    TapeLibraryManager::TapeLibraryManager()
    {
        detail_ = new Detail(this);
    }


    TapeLibraryManager::~TapeLibraryManager()
    {
        delete detail_;
        Factory::Destroy();
    }


    bool
    TapeLibraryManager::GetChangerList(
            vector<Changer> & changers,
            Error & error) const
    {
        boost::lock_guard<boost::mutex> lock(detail_->mutex);

        changers.clear();

        int numberChanger = Factory::Instance()->GetChangerTotalNumber();
        for ( int i = 1; i <= numberChanger; ++i ) {
            changers.push_back( Changer(boost::lexical_cast<string>(i)) );
        }

        return true;
    }

}
