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
 * ChangerTest.cpp
 *
 *  Created on: Jul 5, 2012
 *      Author: More Zeng
 */

#include "stdafx.h"
#include "ChangerTest.h"

#include "../Changer.h"


CPPUNIT_TEST_SUITE_REGISTRATION( ChangerTest );


const string configFile = "/tmp/TapeLibraryManager.cfg";

const fs::path fileConfig(configFile);


void
ChangerTest::setUp()
{
    fs::remove(fileConfig);
}


void
ChangerTest::tearDown()
{
    fs::remove(fileConfig);
}


void
ChangerTest::testConstructor()
{
    auto_ptr<TapeLibraryManager> manager;
    {
        ofstream config(configFile.c_str());
        config << "3 1 16 10 2 16 10 2 16 10" << endl;
    }
    manager.reset( new TapeLibraryManager() );
    fs::remove(fileConfig);

    auto_ptr<Changer> changer(new Changer("3"));
    vector<Drive> drives;
    vector<Slot> slots;
    vector<Cartridge> cartridges;
    int slotID;
    int slotID2;
    bool empty;
    string barcode;
    Error error;

    CPPUNIT_ASSERT( true == changer->GetDriveList(drives,error) );
    CPPUNIT_ASSERT( 2 == drives.size() );
    CPPUNIT_ASSERT( true == changer->GetSlotList(slots,error) );
    CPPUNIT_ASSERT( 16 == slots.size() );
    CPPUNIT_ASSERT( true == changer->GetCartridgeList(cartridges,error) );
    CPPUNIT_ASSERT( 10 == cartridges.size() );

    CPPUNIT_ASSERT( true == drives[0].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode == "03000001" );
    empty = true;
    CPPUNIT_ASSERT( true == drives[0].GetEmpty(empty,error) );
    CPPUNIT_ASSERT( false == empty );

    CPPUNIT_ASSERT(
            true == changer->MoveCartridge(slots[0],cartridges[0],error) );
    CPPUNIT_ASSERT( true == drives[0].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode.empty() );
    CPPUNIT_ASSERT( true == drives[0].GetEmpty(empty,error) );
    CPPUNIT_ASSERT( true == empty );
    CPPUNIT_ASSERT( true == slots[0].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode == "03000001" );
    CPPUNIT_ASSERT( true == slots[0].GetEmpty(empty,error) );
    CPPUNIT_ASSERT( false == empty );

    CPPUNIT_ASSERT(
            true == changer->LoadCartridge(drives[0],cartridges[0],error) );
    CPPUNIT_ASSERT( true == cartridges[0].GetSlotID(slotID,error) );
    CPPUNIT_ASSERT( true == drives[0].GetSlotID(slotID2,error) );
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(slotID),
            slotID == slotID2 );
    CPPUNIT_ASSERT( true == drives[0].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode == "03000001" );
    CPPUNIT_ASSERT( true == slots[0].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode.empty() );
    CPPUNIT_ASSERT( true == cartridges[0].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode == "03000001" );
    CPPUNIT_ASSERT( true == drives[0].GetEmpty(empty,error) );
    CPPUNIT_ASSERT( false == empty );
    CPPUNIT_ASSERT( true == slots[0].GetEmpty(empty,error) );
    CPPUNIT_ASSERT( true == empty );

    CPPUNIT_ASSERT(
            false == changer->LoadCartridge(drives[0],cartridges[1],error) );

    CPPUNIT_ASSERT(
            true == changer->LoadCartridge(drives[0],cartridges[0],error) );

    CPPUNIT_ASSERT(
            true == changer->LoadCartridge(drives[1],cartridges[0],error) );
    CPPUNIT_ASSERT( true == cartridges[0].GetSlotID(slotID,error) );
    CPPUNIT_ASSERT( true == drives[1].GetSlotID(slotID2,error) );
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(slotID),
            slotID == slotID2 );
    CPPUNIT_ASSERT( true == drives[1].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode == "03000001" );
    CPPUNIT_ASSERT( true == drives[1].GetEmpty(empty,error) );
    CPPUNIT_ASSERT( false == empty );
    CPPUNIT_ASSERT( true == drives[0].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode.empty() );
    CPPUNIT_ASSERT( true == drives[0].GetEmpty(empty,error) );
    CPPUNIT_ASSERT( true == empty );
    CPPUNIT_ASSERT( true == cartridges[0].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode == "03000001" );

    CPPUNIT_ASSERT(
            false == changer->MoveCartridge(slots[1],cartridges[0],error) );

    CPPUNIT_ASSERT(
            true == changer->MoveCartridge(slots[0],cartridges[0],error) );
    CPPUNIT_ASSERT( true == cartridges[0].GetSlotID(slotID,error) );
    CPPUNIT_ASSERT( 1 == slotID );
    CPPUNIT_ASSERT( true == slots[0].GetSlotID(slotID,error) );
    CPPUNIT_ASSERT( 1 == slotID );
    CPPUNIT_ASSERT( true == slots[0].GetEmpty(empty,error) );
    CPPUNIT_ASSERT( false == empty );
    CPPUNIT_ASSERT( true == cartridges[0].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode == "03000001" );
    CPPUNIT_ASSERT( true == drives[1].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode.empty() );
    CPPUNIT_ASSERT( true == slots[0].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode == "03000001" );
    CPPUNIT_ASSERT( true == drives[1].GetEmpty(empty,error) );
    CPPUNIT_ASSERT( true == empty );

    CPPUNIT_ASSERT(
            true == changer->LoadCartridge(drives[1],cartridges[0],error) );

    CPPUNIT_ASSERT(
            false == changer->MoveCartridge(slots[9],cartridges[0],error) );
    CPPUNIT_ASSERT(
            true == changer->MoveCartridge(slots[10],cartridges[0],error) );
    CPPUNIT_ASSERT( true == cartridges[0].GetSlotID(slotID,error) );
    CPPUNIT_ASSERT( 11 == slotID );
    CPPUNIT_ASSERT( true == slots[10].GetSlotID(slotID,error) );
    CPPUNIT_ASSERT( 11 == slotID );
    CPPUNIT_ASSERT( true == slots[10].GetEmpty(empty,error) );
    CPPUNIT_ASSERT( false == empty );
    CPPUNIT_ASSERT( true == cartridges[0].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode == "03000001" );
    CPPUNIT_ASSERT( true == drives[1].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode.empty() );
    CPPUNIT_ASSERT( true == slots[10].GetBarcode(barcode,error) );
    CPPUNIT_ASSERT( barcode == "03000001" );
    CPPUNIT_ASSERT( true == drives[1].GetEmpty(empty,error) );
    CPPUNIT_ASSERT( true == empty );
}

