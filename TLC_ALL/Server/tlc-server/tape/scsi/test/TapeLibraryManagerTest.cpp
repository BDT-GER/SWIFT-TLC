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
 *  Created on: Aug 7, 2012
 *      Author: Sam Chen
 */

#include "stdafx.h"
#include "TapeLibraryManagerTest.h"

#include "../../TapeLibraryManager.h"


CPPUNIT_TEST_SUITE_REGISTRATION( TapeLibraryManagerTest );

void
TapeLibraryManagerTest::setUp()
{
	TestBase::setUp();

	static bool bPrinted = false;
	if(!bPrinted)
	{
		START_SUIT("TapeLibraryManagerTest");
		bPrinted = true;
	}
}


void
TapeLibraryManagerTest::tearDown()
{
}

void TapeLibraryManagerTest::testConstructor()
{
	START_TEST("testConstructor");

  // test in real env /////////
  if(m_bRealEnv){
    auto_ptr<TapeLibraryManager> manager;
    vector<Changer> changers;
    Error error;

    manager.reset( new TapeLibraryManager() );
    CPPUNIT_ASSERT( true == manager->GetChangerList(changers,error) );
    CPPUNIT_ASSERT( 1 == changers.size() );

  }//if(m_bRealEnv){
  else{
    auto_ptr<TapeLibraryManager> manager;
    vector<Changer> changers;
    Error error;

    manager.reset( new TapeLibraryManager() );
    CPPUNIT_ASSERT( true == manager->GetChangerList(changers,error) );
    //CPPUNIT_ASSERT( 0 == changers.size() );
  }

	END_TEST("testConstructor");
}

