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
 * CartridgeTest.cpp
 *
 *  Created on: Aug 15, 2012
 *      Author: Sam Chen
 */

#include "stdafx.h"
#include "CartridgeTest.h"

#include "../../Cartridge.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CartridgeTest );

void
CartridgeTest::setUp()
{
	TestBase::setUp();
}


void
CartridgeTest::tearDown()
{
}

void CartridgeTest::testCartridgeOperation()
{
	START_TEST("testConstructor");

  // test in real env
  if(m_bRealEnv){
	  //TODO
  }//if(m_bRealEnv){
  else{
    Error error;
    int slotId = -1;
    string barcode = "";

	Cartridge tape("XXXXXX");
	CPPUNIT_ASSERT(false == tape.GetSlotID(slotId, error));
	CPPUNIT_ASSERT(false == tape.GetBarcode(barcode, error));
  }

	END_TEST("testConstructor");
}
