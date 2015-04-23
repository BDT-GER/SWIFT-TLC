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
 * TapeLibraryManagerTest.cpp
 *
 *  Created on: Jul 3, 2012
 *      Author: More Zeng
 */

#include "stdafx.h"
#include "TapeLibraryManagerTest.h"
#include "ObserverListenerSub.h"

#include "../TapeLibraryManager.h"


CPPUNIT_TEST_SUITE_REGISTRATION( TapeLibraryManagerTest );


const string configFile = "/tmp/TapeLibraryManager.cfg";

const fs::path fileConfig(configFile);


void
TapeLibraryManagerTest::setUp()
{
    fs::remove(fileConfig);
}


void
TapeLibraryManagerTest::tearDown()
{
    fs::remove(fileConfig);
}


void
TapeLibraryManagerTest::testConstructor()
{
    auto_ptr<TapeLibraryManager> manager;
    vector<Changer> changers;
    vector<Drive> drives;
    vector<Cartridge> cartridges;
    vector<Slot> slots;
    Error error;
    bool empty;
    string barcode;
    int slotID;

    manager.reset( new TapeLibraryManager() );
    CPPUNIT_ASSERT( true == manager->GetChangerList(changers,error) );
    CPPUNIT_ASSERT( 2 == changers.size() );


    {
        ofstream config(configFile.c_str());
        config << "1 1 16 10" << endl;
    }
    manager.reset();
    manager.reset( new TapeLibraryManager() );

    CPPUNIT_ASSERT( true == manager->GetChangerList(changers,error) );
    CPPUNIT_ASSERT( 1 == changers.size() );

    CPPUNIT_ASSERT( true == changers[0].GetDriveList(drives,error) );
    CPPUNIT_ASSERT( 1 == drives.size() );
    CPPUNIT_ASSERT( true == drives[0].GetSlotID(slotID,error) );
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(slotID),
            -1 == slotID );
    empty = true;
    CPPUNIT_ASSERT( true == drives[0].GetEmpty(empty,error) );
    CPPUNIT_ASSERT( false == empty );
    barcode = "barcode";
    CPPUNIT_ASSERT( true == drives[0].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( "01000001" == barcode );

    CPPUNIT_ASSERT( true == changers[0].GetSlotList(slots,error) );
    CPPUNIT_ASSERT( 16 == slots.size() );
    for ( int i = 0; i < 16; ++i ) {
        CPPUNIT_ASSERT( true == slots[i].GetSlotID(slotID,error) );
        CPPUNIT_ASSERT( (i+1) == slotID );
    }
    CPPUNIT_ASSERT( true == changers[0].GetCartridgeList(cartridges,error) );
    CPPUNIT_ASSERT( 10 == cartridges.size() );
    for ( int i = 0; i < 10; ++i ) {
        CPPUNIT_ASSERT( true == cartridges[i].GetSlotID(slotID,error) );
        if ( 0 == i ) {
            CPPUNIT_ASSERT( -1 == slotID );
        } else {
            CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(slotID),
                    (i+1) == slotID );
        }
        CPPUNIT_ASSERT( true == cartridges[i].GetBarcode(barcode,error) );
        ostringstream os;
        os << "01" << setw(6) << setfill('0') << (i+1);
        CPPUNIT_ASSERT_MESSAGE( os.str()+":"+barcode, os.str() == barcode );
        CPPUNIT_ASSERT( true == slots[i].GetBarcode(barcode,error) );
        CPPUNIT_ASSERT( true == slots[i].GetEmpty(empty,error) );
        if ( 0 == i ) {
            CPPUNIT_ASSERT( true == empty && barcode.empty() );
        } else {
            CPPUNIT_ASSERT( false == empty && os.str() == barcode );
        }
    }
    for ( int i = 10; i < 16; ++i ) {
        barcode = "barcode";
        CPPUNIT_ASSERT( true == slots[i].GetBarcode(barcode,error) );
        CPPUNIT_ASSERT( barcode.empty() );
        CPPUNIT_ASSERT( true == slots[i].GetEmpty(empty,error) );
        CPPUNIT_ASSERT( true == empty );
    }


    {
        ofstream config(configFile.c_str());
        config << "2 2 16 10 1 8 8" << endl;
    }
    manager.reset();
    manager.reset( new TapeLibraryManager() );

    CPPUNIT_ASSERT( true == manager->GetChangerList(changers,error) );
    CPPUNIT_ASSERT( 2 == changers.size() );
    CPPUNIT_ASSERT( true == changers[0].GetDriveList(drives,error) );
    CPPUNIT_ASSERT( 2 == drives.size() );
    CPPUNIT_ASSERT( true == changers[0].GetCartridgeList(cartridges,error) );
    CPPUNIT_ASSERT( 10 == cartridges.size() );
    CPPUNIT_ASSERT( true == changers[0].GetSlotList(slots,error) );
    CPPUNIT_ASSERT( 16 == slots.size() );
    CPPUNIT_ASSERT( true == changers[1].GetDriveList(drives,error) );
    CPPUNIT_ASSERT( 1 == drives.size() );
    CPPUNIT_ASSERT( true == changers[1].GetCartridgeList(cartridges,error) );
    CPPUNIT_ASSERT( 8 == cartridges.size() );
    CPPUNIT_ASSERT( true == changers[1].GetSlotList(slots,error) );
    CPPUNIT_ASSERT( 8 == slots.size() );


    ObserverListenerSub::UpdateCount = 0;
    ObserverListenerSub listener0, listener1;
    CPPUNIT_ASSERT( true == manager->Attach(&listener0) );
    CPPUNIT_ASSERT( true == manager->Attach(&listener1) );
    boost::this_thread::sleep(boost::posix_time::milliseconds(150));
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(ObserverListenerSub::UpdateCount),
            2 == ObserverListenerSub::UpdateCount );


    manager.reset();
    manager.reset( new TapeLibraryManager() );
    fs::remove(fileConfig);
    boost::this_thread::sleep(boost::posix_time::milliseconds(200));

    CPPUNIT_ASSERT( true == manager->GetChangerList(changers,error) );
    CPPUNIT_ASSERT( 2 == changers.size() );
    CPPUNIT_ASSERT( true == changers[0].GetDriveList(drives,error) );
    CPPUNIT_ASSERT( 2 == drives.size() );
    CPPUNIT_ASSERT( true == changers[0].GetCartridgeList(cartridges,error) );
    CPPUNIT_ASSERT( 10 == cartridges.size() );
    CPPUNIT_ASSERT( true == changers[0].GetSlotList(slots,error) );
    CPPUNIT_ASSERT( 16 == slots.size() );
    CPPUNIT_ASSERT( true == changers[1].GetDriveList(drives,error) );
    CPPUNIT_ASSERT( 1 == drives.size() );
    CPPUNIT_ASSERT( true == changers[1].GetCartridgeList(cartridges,error) );
    CPPUNIT_ASSERT( 8 == cartridges.size() );
    CPPUNIT_ASSERT( true == changers[1].GetSlotList(slots,error) );
    CPPUNIT_ASSERT( 8 == slots.size() );
}

