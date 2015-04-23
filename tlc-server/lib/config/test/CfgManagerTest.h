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
 * CfgManagerTest.h
 *
 *  Created on: Nov 6, 2012
 *      Author: chento
 */

#ifndef CFGMANAGERTEST_H_
#define CFGMANAGERTEST_H_

class CfgManagerTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( CfgManagerTest );
	//CPPUNIT_TEST( testGetCfgDetail );
	CPPUNIT_TEST( testGet );
	CPPUNIT_TEST( testGetUInt64 );
	CPPUNIT_TEST( testGetUInt32 );
	CPPUNIT_TEST( testGetUInt16 );
	CPPUNIT_TEST( testGetFloat );
	CPPUNIT_TEST( testGetBool );
	CPPUNIT_TEST( testGetSize );
	//CPPUNIT_TEST( testSetFileRetentionStartTime );
	//CPPUNIT_TEST( testSetShareRetention );
	CPPUNIT_TEST( testSetChild );
	CPPUNIT_TEST( testDeleteNode );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

	//void testGetCfgDetail();
	void testGet();
	void testGetUInt64();
	void testGetUInt32();
	void testGetUInt16();
	void testGetFloat();
	void testGetBool();
	void testGetSize();

	//void testSetFileRetentionStartTime();
	//void testSetShareRetention();
	void testSetChild();

	void testDeleteNode();
};

#endif /* CFGMANAGERTEST_H_ */
