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
 * DriveTest.cpp
 *
 *  Created on: Jul 9, 2012
 *      Author: More Zeng
 */

#include "stdafx.h"
#include "DriveTest.h"

#include "../Drive.h"


CPPUNIT_TEST_SUITE_REGISTRATION( DriveTest );


const fs::path ltfsFolder = Drive::GetLTFSFolder();
const fs::path tapeFolder = "/opt/LTFStor/ltfsTapes";

const string configFile = "/tmp/TapeLibraryManager.cfg";
const fs::path fileConfig(configFile);


void
DriveTest::setUp()
{
    fs::remove(fileConfig);
}


void
DriveTest::tearDown()
{
    fs::remove(fileConfig);
}


void
DriveTest::testFormat()
{
    auto_ptr<TapeLibraryManager> manager;
    {
        ofstream config(configFile.c_str());
        config << "4 1 16 10 2 16 10 2 16 10 1 16 10" << endl;
    }
    manager.reset( new TapeLibraryManager() );
    fs::remove(fileConfig);

    auto_ptr<Changer> changer(new Changer("3"));
    Error error;
    vector<Drive> drives;
    CPPUNIT_ASSERT( true == changer->GetDriveList(drives,error) );
    vector<Slot> slots;
    CPPUNIT_ASSERT( true == changer->GetSlotList(slots,error) );
    vector<Cartridge> cartridges;
    CPPUNIT_ASSERT( true == changer->GetCartridgeList(cartridges,error) );
    string barcode;
    CPPUNIT_ASSERT( true == drives[0].GetBarcode(barcode,error) );

    //CPPUNIT_ASSERT( fs::exists(tapeFolder/barcode) );
    //CPPUNIT_ASSERT( fs::exists(ltfsFolder/barcode) );

    fs::remove_all(tapeFolder/barcode);
    fs::remove_all(ltfsFolder/barcode);

    CPPUNIT_ASSERT(
            true == changer->MoveCartridge(slots[0],cartridges[0],error) );

    CPPUNIT_ASSERT( ! fs::exists(tapeFolder/barcode) );
    CPPUNIT_ASSERT( ! fs::exists(ltfsFolder/barcode) );

    CPPUNIT_ASSERT( false == drives[0].Format(error) );
    CPPUNIT_ASSERT( ! fs::exists(tapeFolder/barcode) );
    CPPUNIT_ASSERT( ! fs::exists(ltfsFolder/barcode) );

    CPPUNIT_ASSERT( false == drives[0].Mount(error) );
    CPPUNIT_ASSERT( ! fs::exists(tapeFolder/barcode) );
    CPPUNIT_ASSERT( ! fs::exists(ltfsFolder/barcode) );

    CPPUNIT_ASSERT(
            true == changer->LoadCartridge(drives[0],cartridges[0],error) );
    CPPUNIT_ASSERT( ! fs::exists(tapeFolder/barcode) );
    CPPUNIT_ASSERT( ! fs::exists(ltfsFolder/barcode) );

    CPPUNIT_ASSERT( true == drives[0].Format(error) );
    CPPUNIT_ASSERT( fs::exists(tapeFolder/barcode) );
    CPPUNIT_ASSERT( ! fs::exists(ltfsFolder/barcode) );

    CPPUNIT_ASSERT( true == drives[0].Format(error) );
    CPPUNIT_ASSERT( fs::exists(tapeFolder/barcode) );
    CPPUNIT_ASSERT( ! fs::exists(ltfsFolder/barcode) );

    CPPUNIT_ASSERT( true == drives[0].Mount(error) );
    CPPUNIT_ASSERT( fs::exists(tapeFolder/barcode) );
    CPPUNIT_ASSERT( fs::exists(ltfsFolder/barcode) );

    CPPUNIT_ASSERT( false == drives[0].Format(error) );
    CPPUNIT_ASSERT( fs::exists(tapeFolder/barcode) );
    CPPUNIT_ASSERT( fs::exists(ltfsFolder/barcode) );

    CPPUNIT_ASSERT( true == drives[0].Unmount(error) );
    CPPUNIT_ASSERT( fs::exists(tapeFolder/barcode) );
    CPPUNIT_ASSERT( ! fs::exists(ltfsFolder/barcode) );

    CPPUNIT_ASSERT( true == drives[0].Format(error) );
    CPPUNIT_ASSERT( fs::exists(tapeFolder/barcode) );
    CPPUNIT_ASSERT( ! fs::exists(ltfsFolder/barcode) );

    CPPUNIT_ASSERT(
            true == changer->MoveCartridge(slots[0],cartridges[0],error) );
    CPPUNIT_ASSERT( fs::exists(tapeFolder/barcode) );
    CPPUNIT_ASSERT( ! fs::exists(ltfsFolder/barcode) );

    CPPUNIT_ASSERT( false == drives[0].Format(error) );
    CPPUNIT_ASSERT( fs::exists(tapeFolder/barcode) );
    CPPUNIT_ASSERT( ! fs::exists(ltfsFolder/barcode) );

    CPPUNIT_ASSERT( false == drives[0].Mount(error) );
    CPPUNIT_ASSERT( fs::exists(tapeFolder/barcode) );
    CPPUNIT_ASSERT( ! fs::exists(ltfsFolder/barcode) );
}
