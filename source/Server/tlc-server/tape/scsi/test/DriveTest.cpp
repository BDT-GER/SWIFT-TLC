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
 *  Created on: Aug 15, 2012
 *      Author: Sam Chen
 */

#include "stdafx.h"
#include "DriveTest.h"

#include "../../Drive.h"
#include "../../../lib/ltfs_library/LtfsError.h"


using namespace ltfs_management;
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION( DriveTest );

void
DriveTest::setUp()
{
	TestBase::setUp();
}


void
DriveTest::tearDown()
{
}

void DriveTest::testDriveOperation()
{
	START_TEST("testConstructor");

  // test in real env///////////////////////////////
  if(m_bRealEnv){

    auto_ptr<TapeLibraryManager> manager;
    manager.reset( new TapeLibraryManager() );

    vector<Changer> changers;
    vector<Drive> drives;
    vector<Slot> slots;
    vector<Cartridge> tapes;
    int slotId;
    int slotId2;
    bool empty;
    string barcode;
    Error error;
    bool bFound = false;
    string label = "";
    int tapeInex = 0;
    bool ret = false;

RETRY_LABEL:

    REFRESH_MANAGER

    CPPUNIT_ASSERT(true == drives[0].GetEmpty(empty, error));
    if(!empty){
    	// get drive slot id
    	int driveSlotId = 0;
    	CPPUNIT_ASSERT(drives[0].GetSlotID(driveSlotId, error));
    	// get cartige in the drive
    	Cartridge tmpTape("");
    	bool bFound = false;
    	for(int i = 0; i < tapes.size(); i++){
			CPPUNIT_ASSERT(tapes[i].GetSlotID(slotId, error));
			if(slotId == driveSlotId){
				tmpTape = tapes[i];
				bFound = true;
				break;
			}
    	}
    	CPPUNIT_ASSERT(true == bFound);
    	bFound = false;
    	// find an empty slot
    	for(int i = 0; i < slots.size(); i++){
    		if(slots[i].GetEmpty(empty, error) && empty){
				//move the tape in drive to this empty slot
    			CPPUNIT_ASSERT(changers[0].MoveCartridge(slots[i], tmpTape, error));
    			bFound = true;
    			break;
    		}
    	}
    	CPPUNIT_ASSERT(true == bFound);
    }

    REFRESH_MANAGER

    CPPUNIT_ASSERT(true == drives[0].GetEmpty(empty, error));
    CPPUNIT_ASSERT(true == empty);

    //format/mount/unmount option on empty should fail
	CPPUNIT_ASSERT(false == drives[0].Format(label, error));
	CPPUNIT_ASSERT(false == drives[0].Mount(error));
	CPPUNIT_ASSERT(false == drives[0].Unmount(error));

    // find a tape with barcode
	Cartridge tmpTape("");
	bFound = false;
	for(int i = tapeInex; i < tapes.size(); i++){
		CPPUNIT_ASSERT(tapes[i].GetBarcode(barcode, error));
		if(barcode != ""){
			tmpTape = tapes[i];
			bFound = true;
			break;
		}
	}
	if(false == bFound){
		CPPUNIT_ASSERT(tapeInex > 0);
		END_TEST("testConstructor");
		return;
	}else{
		tapeInex++;
	}
	CPPUNIT_ASSERT(changers[0].LoadCartridge(drives[0], tmpTape, error));

    REFRESH_MANAGER

	//format the tape
    ret = drives[0].Format(label, error);
	CPPUNIT_ASSERT(true == ret || error.ErrorNumber() == ERR_FORMAT_WRITE_PROTECT);//ERR_FORMAT_WRITE_PROTECT
	if(false == ret){
		goto RETRY_LABEL;
	}

	//mount the tape
	CPPUNIT_ASSERT(true == drives[0].Mount(error));
    fs::path mountPoint = Drive::GetLTFSFolder() / barcode;
    string cmdCheckMount = "mount | grep " + mountPoint.string();
    CPPUNIT_ASSERT(0 == std::system(cmdCheckMount.c_str()));

	//unmount the tape
	CPPUNIT_ASSERT(true == drives[0].Unmount(error));
    CPPUNIT_ASSERT(0 != std::system(cmdCheckMount.c_str()));

  }//if(m_bRealEnv){
  else{
    vector<Changer> changers;
    vector<Drive> drives;
    vector<Slot> slots;
    vector<Cartridge> tapes;
    auto_ptr<TapeLibraryManager> manager;
    Error error;
    bool empty = false;
    int slotId = -1;
    string barcode = "";
    string label = "";

	//manager.reset( new TapeLibraryManager() );
	//CPPUNIT_ASSERT( true == manager->GetChangerList(changers,error) );
	Drive drive("XXXXXX");
	CPPUNIT_ASSERT(false == drive.GetEmpty(empty, error));
	CPPUNIT_ASSERT(false == drive.GetSlotID(slotId, error));
	CPPUNIT_ASSERT(false == drive.GetBarcode(barcode, error));
	CPPUNIT_ASSERT(false == drive.Mount(error));
	CPPUNIT_ASSERT(false == drive.Unmount(error));
	CPPUNIT_ASSERT(false == drive.Format(label, error));
	//CPPUNIT_ASSERT( 0 == changers.size() );
	//CPPUNIT_ASSERT( true == changers[0].GetSlotList(slots,error) );
	//CPPUNIT_ASSERT( 0 == slots.size() );
	//CPPUNIT_ASSERT( true == changers[0].GetCartridgeList(tapes,error) );
	//CPPUNIT_ASSERT( 0 == tapes.size() );
	//CPPUNIT_ASSERT( true == changers[0].GetDriveList(drives,error) );
	//CPPUNIT_ASSERT( 0 == drives.size() );
  }

	END_TEST("testConstructor");
}
