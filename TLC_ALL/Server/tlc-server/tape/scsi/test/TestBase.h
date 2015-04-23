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
 * TestBase.h
 *
 *  Created on: Aug 22, 2012
 *      Author: Sam Chen
 */

#pragma once

class TestBase
{
public:
	TestBase();
	~TestBase();
    virtual void setUp();
    virtual void tearDown();

protected:
	bool	m_bRealEnv;
	int		m_nChangerNum;
	string	m_changerSgName;
	string	m_changerStName;
	int		m_nDriveNum;
	string	m_driveSgName;
	string	m_driveStName;
	int		m_nTapeNum;
	int		m_nSlotNum;
	int		m_nMailSlotNum;
};


#define REFRESH_MANAGER  \
	manager.reset( new TapeLibraryManager() ); \
	CPPUNIT_ASSERT( true == manager->GetChangerList(changers,error) );\
	CPPUNIT_ASSERT( m_nChangerNum == changers.size() );\
	if(m_nChangerNum > 0){ \
		CPPUNIT_ASSERT( true == changers[0].GetSlotList(slots,error) );\
		CPPUNIT_ASSERT( m_nSlotNum == slots.size() );\
		CPPUNIT_ASSERT( true == changers[0].GetCartridgeList(tapes,error) );\
		CPPUNIT_ASSERT( m_nTapeNum == tapes.size() );\
		CPPUNIT_ASSERT( true == changers[0].GetDriveList(drives,error) );\
		CPPUNIT_ASSERT( m_nDriveNum == drives.size() );\
	}
