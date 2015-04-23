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
 *  Created on: Aug 8, 2012
 *      Author: Sam Chen
 */

#include "stdafx.h"
#include "ChangerTest.h"

#include "../../Changer.h"


CPPUNIT_TEST_SUITE_REGISTRATION( ChangerTest );

void
ChangerTest::setUp()
{
	TestBase::setUp();

	static bool bPrinted = false;
	if(!bPrinted)
	{
		START_SUIT("ChangerTest");
		bPrinted = true;
	}
}


void
ChangerTest::tearDown()
{
}

void ChangerTest::testConstructor()
{
	START_TEST("testConstructor");

	// real env test ///////////////
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

    // find a tape with barcode
	Cartridge tmpTape("");
	bFound = false;
	for(int i = 0; i < tapes.size(); i++){
		CPPUNIT_ASSERT(tapes[i].GetBarcode(barcode, error));
		if(barcode != ""){
			tmpTape = tapes[i];
			bFound = true;
			break;
		}
	}
	CPPUNIT_ASSERT(true == bFound);
	CPPUNIT_ASSERT(changers[0].LoadCartridge(drives[0], tmpTape, error));

    REFRESH_MANAGER

    CPPUNIT_ASSERT(true == drives[0].GetEmpty(empty, error));
    CPPUNIT_ASSERT(false == empty);

  }//if(m_bRealEnv){
  else{
    auto_ptr<TapeLibraryManager> manager;
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

	manager.reset( new TapeLibraryManager() );
	CPPUNIT_ASSERT( true == manager->GetChangerList(changers,error) );
	//CPPUNIT_ASSERT( 0 == changers.size() );

	Changer changer("xxxx");
	CPPUNIT_ASSERT( false == changer.GetSlotList(slots,error) );
	CPPUNIT_ASSERT(error.ErrorNumber() == Error::ERROR_NO_ENTRY);
	CPPUNIT_ASSERT( false == changer.GetCartridgeList(tapes,error) );
	CPPUNIT_ASSERT(error.ErrorNumber() == Error::ERROR_NO_ENTRY);
	CPPUNIT_ASSERT( false == changer.GetDriveList(drives,error) );
	CPPUNIT_ASSERT(error.ErrorNumber() == Error::ERROR_NO_ENTRY);

	Slot slot("XXXX");
	Drive drive("XXXX");
	Cartridge tape("XXXX");
	CPPUNIT_ASSERT( false == changer.MoveCartridge(slot, tape, error) );
	CPPUNIT_ASSERT(error.ErrorNumber() == Error::ERROR_NO_ENTRY);
	CPPUNIT_ASSERT( false == changer.LoadCartridge(drive, tape, error) );
	CPPUNIT_ASSERT(error.ErrorNumber() == Error::ERROR_NO_ENTRY);

  }//

	END_TEST("testConstructor");
}

