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
 * Factory.cpp
 *
 *  Created on: Jul 31, 2012
 *      Author: More Zeng
 */

#include "../stdafx.h"
#include "Factory.h"


namespace tape
{

    Factory * Factory::instance_ = NULL;


    Factory::Factory()
    : changer_(0)
    {
    }


    Factory::~Factory()
    {
    }


    void
    Factory::AddChanger(int changer)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        assert( changer == (changer_ + 1) );
        changer_ = changer;
        changers_.insert(ChangerMap::value_type(changer,ChangerImpl(changer)));
    }


    void
    Factory::AddDrive(int changer,int number)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        ChangerMap::iterator i = changers_.find(changer);
        assert( changers_.end() != i );
        i->second.SetNumberDrive(number);

        drives_.insert( DriveMap::value_type(
                Key(changer,number),
                DriveImpl(- number) ) );
    }


    void
    Factory::AddSlot(int changer,int number)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        ChangerMap::iterator i = changers_.find(changer);
        assert( changers_.end() != i );
        i->second.SetNumberSlot(number);

        slots_.insert( SlotMap::value_type(
                Key(changer,number),
                SlotImpl(number) ) );
    }


    void
    Factory::AddCartridge(int changer,int number)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        ChangerMap::iterator i = changers_.find(changer);
        assert( changers_.end() != i );
        i->second.SetNumberCartridge(number);

        cartridges_.insert( CartridgeMap::value_type(
                Key(changer,number),
                CartridgeImpl(changer,number) ) );
    }


    ChangerImpl *
    Factory::GetChanger(int changer)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        ChangerMap::iterator i = changers_.find(changer);
        if ( changers_.end() == i ) {
            return NULL;
        }
        return &i->second;
    }


    DriveImpl *
    Factory::GetDrive(int changer,int number)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        if ( number < 0 ) {
            number = - number;
        }
        DriveMap::iterator i = drives_.find(Key(changer,number));
        if ( drives_.end() == i ) {
            return NULL;
        }
        return &i->second;
    }


    SlotImpl *
    Factory::GetSlot(int changer,int number)
    {
        if ( number < 0 ) {
            return GetDrive(changer,number);
        }

        boost::lock_guard<boost::mutex> lock(mutex_);

        SlotMap::iterator i = slots_.find(Key(changer,number));
        if ( slots_.end() == i ) {
            return NULL;
        }
        return &i->second;
    }


    CartridgeImpl *
    Factory::GetCartridge(int changer,int number)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        CartridgeMap::iterator i = cartridges_.find(Key(changer,number));
        if ( cartridges_.end() == i ) {
            return NULL;
        }
        return &i->second;
    }


    CartridgeImpl *
    Factory::GetCartridge(const string & barcode)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        CartridgeMap::iterator i =
                cartridges_.find(boost::lexical_cast<int>(barcode));
        if ( cartridges_.end() == i ) {
            return NULL;
        }
        return &i->second;
    }

}
