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
 * Factory.h
 *
 *  Created on: Jul 31, 2012
 *      Author: More Zeng
 */

#pragma once


#include "ChangerImpl.h"
#include "DriveImpl.h"
#include "SlotImpl.h"
#include "CartridgeImpl.h"


namespace tape
{

    class Factory
    {
    public:
        Factory();

        virtual
        ~Factory();

        static Factory *
        Instance()
        {
            if ( NULL == instance_ ) {
                instance_ = new Factory();
            }
            return instance_;
        }

        static void
        Destroy()
        {
            if ( NULL != instance_ ) {
                boost::lock_guard<boost::mutex> lock(instance_->mutex_);

                instance_->changer_ = 0;
                instance_->changers_.clear();
                instance_->drives_.clear();
                instance_->slots_.clear();
                instance_->cartridges_.clear();
            }
        }

        void
        AddChanger(int changer);

        void
        AddSlot(int changer,int number);

        void
        AddDrive(int changer,int number);

        void
        AddCartridge(int changer,int number);

        int
        GetChangerTotalNumber()
        {
            boost::lock_guard<boost::mutex> lock(instance_->mutex_);

            return changer_;
        }

        ChangerImpl *
        GetChanger(int changer);

        DriveImpl *
        GetDrive(int changer,int number);

        SlotImpl *
        GetSlot(int changer,int number);

        CartridgeImpl *
        GetCartridge(int changer,int number);

        CartridgeImpl *
        GetCartridge(const string & barcode);

    private:
        typedef map<int,ChangerImpl> ChangerMap;
        typedef map<int,DriveImpl> DriveMap;
        typedef map<int,SlotImpl> SlotMap;
        typedef map<int,CartridgeImpl> CartridgeMap;

        static Factory * instance_;

        boost::mutex mutex_;

        int changer_;
        ChangerMap changers_;
        DriveMap drives_;
        SlotMap slots_;
        CartridgeMap cartridges_;

        int Key(int changer,int number)
        {
            return changer * 1000000 + number;
        }
    };

}

