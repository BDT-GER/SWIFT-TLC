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
 * SimulatorTest.h
 *
 *  Created on: Oct 15, 2012
 *      Author: chento
 */

#pragma once

class SimulatorTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( SimulatorTest );
	CPPUNIT_TEST( testGetCfgDetail );
	CPPUNIT_TEST( testGetChangerList );
	CPPUNIT_TEST( testGetDriveList );
	CPPUNIT_TEST( testGetSlotList );
	CPPUNIT_TEST( testGetMailSlotList );
	CPPUNIT_TEST( testGetTapeList );
	CPPUNIT_TEST( testMoveTape );
	CPPUNIT_TEST( testFormat );
	CPPUNIT_TEST( testGetChangerInfo );
	CPPUNIT_TEST( testGetDriveInfo );
	CPPUNIT_TEST_SUITE_END();

	public:
		void setUp();
		void tearDown();

		void testGetCfgDetail();
		void testGetChangerList();
		void testGetDriveList();
		void testGetSlotList();
		void testGetMailSlotList();
		void testGetTapeList();
		void testMoveTape();
		void testFormat();
		void testGetChangerInfo();
		void testGetDriveInfo();

};

