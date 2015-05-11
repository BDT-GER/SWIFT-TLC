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
 * SimulatorTest.cpp
 *
 *  Created on: Oct 15, 2012
 *      Author: chento
 */

#include "stdafx.h"
#include "../SimChanger.h"
#include "../CfgfileParser.h"
#include "../CfgfileSerialization.h"
#include "../Simulator.h"
#include "SimulatorTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( SimulatorTest );

using namespace ltfs_management;

Simulator * simulator = NULL;

void PrintSlot(vector<struct SlotDetail>& slots);
void PrintTape(vector<struct TapeDetail>& tapes);

void
SimulatorTest::setUp()
{
	// TODO Auto-generated constructor stub
	simulator = Simulator::Instance();
}

void
SimulatorTest::tearDown()
{
	// TODO Auto-generated destructor stub
	//Simulator::Destory();
}

void
SimulatorTest::testGetCfgDetail()
{
	START_TEST(__func__);

	struct CfgDetail detail;
	CPPUNIT_ASSERT(simulator->GetCfgDetail(detail));

	cout << "m_SizeReadErr:" << detail.m_SizeReadErr << endl;
	cout << "m_SizeWriteErr" << detail.m_SizeWriteErr << endl;
	cout << "m_NumReadErr" << detail.m_NumReadErr << endl;
	cout << "m_NumWriteErr" << detail.m_NumWriteErr << endl;
	cout << "m_ReadDelay" << detail.m_ReadDelay << endl;
	cout << "m_WriteDelay" << detail.m_WriteDelay << endl;
	cout << "m_MoveDelay" << detail.m_MoveDelay << endl;
	cout << "m_MoveFailure" << detail.m_MoveFailure << endl;
	cout << "m_MountDelay" << detail.m_MountDelay << endl;
	cout << "m_MountError" << detail.m_MountError << endl;
	cout << "m_UmontDelay" << detail.m_UmontDelay << endl;
	cout << "m_UmontError" << detail.m_UmontError << endl;
	cout << "m_FormatDelay" << detail.m_FormatDelay << endl;
	cout << "m_FormatError" << detail.m_FormatError << endl;

	END_TEST(__func__);
}

void
SimulatorTest::testGetChangerList()
{
	START_TEST(__func__);

	vector<struct ChangerDetail> changers;
	int errId;
	CPPUNIT_ASSERT(simulator!=NULL);
	CPPUNIT_ASSERT(simulator->GetChangerList(changers, errId));
	cout << "changers size is " << changers.size() << endl;
	CPPUNIT_ASSERT(changers.size() == 1);

	END_TEST(__func__);
}

void
SimulatorTest::testGetDriveList()
{
	START_TEST(__func__);

	string serial = "DE64000F22";
	vector<struct DriveDetail> drives;
	int errId;

	CPPUNIT_ASSERT(simulator->GetDriveList(serial, drives, errId));
	cout << "drives size is " << drives.size() << endl;
	CPPUNIT_ASSERT(drives.size() == 2);

	END_TEST(__func__);
}

void
SimulatorTest::testGetSlotList()
{
	START_TEST(__func__);

	string serial = "DE64000F22";
	vector<struct SlotDetail> slots;
	int errId;

	CPPUNIT_ASSERT(simulator->GetSlotList(serial, slots, errId));
	cout << "slots size is " << slots.size() << endl;
	CPPUNIT_ASSERT(slots.size() == 23);

	END_TEST(__func__);
}

void
SimulatorTest::testGetMailSlotList()
{
	START_TEST(__func__);

	string serial = "DE64000F22";
	vector<struct MailSlotDetail> mailslots;
	int errId;

	CPPUNIT_ASSERT(simulator->GetMailSlotList(serial, mailslots, errId));
	cout << "mailslots size is " << mailslots.size() << endl;
	CPPUNIT_ASSERT(mailslots.size() == 1);

	END_TEST(__func__);
}

void
SimulatorTest::testGetTapeList()
{
	START_TEST(__func__);

	string serial = "DE64000F22";
	vector<struct TapeDetail> tapes;
	int errId;

	CPPUNIT_ASSERT(simulator->GetTapeList(serial, tapes, errId));
	cout << "tapes size is " << tapes.size() << endl;
	CPPUNIT_ASSERT(tapes.size() == 10);

	END_TEST(__func__);
}

void
SimulatorTest::testMoveTape()
{
	START_TEST(__func__);

	string serial = "DE64000F22";
	vector<struct SlotDetail> slots;
	vector<struct TapeDetail> tapes;
	int errId;

	CPPUNIT_ASSERT(simulator->GetSlotList(serial, slots, errId));
	PrintSlot(slots);
	CPPUNIT_ASSERT(simulator->GetTapeList(serial, tapes, errId));\
	PrintTape(tapes);

	cout << "**********MoveTape from 1001 to 1002************************" << endl;
	CPPUNIT_ASSERT(simulator->MoveTape(serial, 1001, 1007, errId));
	CPPUNIT_ASSERT(simulator->GetSlotList(serial, slots, errId));
	PrintSlot(slots);
	CPPUNIT_ASSERT(simulator->GetTapeList(serial, tapes, errId));\
	PrintTape(tapes);

	cout << "**********MoveTape from 1001 to 1003************************" << endl;
	CPPUNIT_ASSERT(!simulator->MoveTape(serial, 1001, 1003, errId));
	CPPUNIT_ASSERT(simulator->GetSlotList(serial, slots, errId));
	PrintSlot(slots);
	CPPUNIT_ASSERT(simulator->GetTapeList(serial, tapes, errId));\
	PrintTape(tapes);

	cout << "**********MoveTape from 1002 to 1003************************" << endl;
	CPPUNIT_ASSERT(!simulator->MoveTape(serial, 1002, 1003, errId));
	CPPUNIT_ASSERT(simulator->GetSlotList(serial, slots, errId));
	PrintSlot(slots);
	CPPUNIT_ASSERT(simulator->GetTapeList(serial, tapes, errId));\
	PrintTape(tapes);


	END_TEST(__func__);
}

void
SimulatorTest::testFormat()
{
	START_TEST(__func__);


	END_TEST(__func__);
}

void
SimulatorTest::testGetChangerInfo()
{
	START_TEST(__func__);

	string serial = "DE64000F22";
	struct ChangerDetail changer;

	CPPUNIT_ASSERT(simulator->GetChangerInfo(serial, changer));


	END_TEST(__func__);
}

void
SimulatorTest::testGetDriveInfo()
{
	START_TEST(__func__);

	string serial = "HU1033BW9H";
	struct DriveDetail drive;

	CPPUNIT_ASSERT(simulator->GetDriveInfo(serial, drive));

	END_TEST(__func__);
}

void
PrintSlot(vector<struct SlotDetail>& slots)
{
	for(vector<struct SlotDetail>::iterator iter=slots.begin();
			iter!=slots.end(); ++iter)
	{
		cout << "SlotID:"<<iter->m_SlotID << " Barcode:" << iter->m_Barcode << endl;
	}

	cout << endl;
}

void
PrintTape(vector<struct TapeDetail>& tapes)
{
	for(vector<struct TapeDetail>::iterator iter=tapes.begin();
			iter!=tapes.end(); ++iter)
	{
		cout << "Barcode:" << iter->m_Barcode << " SlotID:"<<iter->m_SlotID << endl;
	}

	cout << endl;
}
