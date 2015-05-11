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
 * ResourceManagerTest.cpp
 *
 *  Created on: Aug 9, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../PriorityTape.h"
#include "../ResourceTapeSimulator.h"
#include "../ResourceManager.h"
#include "ResourceManagerTest.h"
#include "ScheduleTask.h"


CPPUNIT_TEST_SUITE_REGISTRATION( ResourceManagerTest );


const string testFolder = "test.folder/";
const string testTape11 = "01000001";    // changer 1 with 2 drives
const string testTape12 = "01000002";    // changer 1 with 2 drives
const string testTape21 = "02000001";    // changer 2 with 1 drive
const string testTape22 = "02000002";    // changer 2 with 1 drive

const fs::path folderTest(testFolder);
const fs::path folderTape11(testFolder+testTape11);
const fs::path folderTape12(testFolder+testTape12);
const fs::path folderTape21(testFolder+testTape21);
const fs::path folderTape22(testFolder+testTape22);




void
ResourceManagerTest::setUp()
{
    Factory::ReleaseTapeLibraryManager();
    Factory::CreateTapeLibraryManager();
}


void
ResourceManagerTest::tearDown()
{
    Factory::ReleaseTapeLibraryManager();
}


int
ResourceManagerTest::GetTapeSlotID(int changer,const string & tape)
{
    tape::TapeLibraryManager * manager = Factory::GetTapeLibraryManager();
    vector<tape::Changer> changers;
    vector<tape::Slot> slots;
    vector<tape::Drive> drives;
    tape::Error err;

    if ( ! manager->GetChangerList(changers,err) ) {
        return -1;
    }
    if ( changer >= changers.size() ) {
        return -2;
    }

    if ( ! changers[changer].GetSlotList(slots,err) ) {
        return -3;
    }

    for ( size_t i = 0; i < slots.size(); ++ i ) {
        bool empty;
        if ( ! slots[i].GetEmpty(empty,err) ) {
            continue;
        }
        if ( ! empty ) {
            string barcode;
            if ( slots[i].GetBarcode(barcode,err) ) {
                if ( barcode == tape ) {
                    int id;
                    if ( slots[i].GetSlotID(id,err) ) {
                        return id;
                    } else {
                        return -16;
                    }
                }
            }
        }
    }

    if ( ! changers[changer].GetDriveList(drives,err) ) {
        return -4;
    }

    for ( size_t i = 0; i < drives.size(); ++ i ) {
        bool empty;
        if ( ! drives[i].GetEmpty(empty,err) ) {
            continue;
        }
        if ( ! empty ) {
            string barcode;
            if ( drives[i].GetBarcode(barcode,err) ) {
                if ( barcode == tape ) {
                    int id;
                    if ( drives[i].GetSlotID(id,err) ) {
                        return id;
                    } else {
                        return -32;
                    }
                }
            }
        }
    }

    return -5;
}


void
ResourceManagerTest::testTape()
{
    auto_ptr<PriorityTape> priority0(new PriorityTape());
    auto_ptr<PriorityTape> priority1(new PriorityTape());
    auto_ptr<ResourceTapeSimulator> simulator(new ResourceTapeSimulator());
    auto_ptr<ResourceManager> resource(new ResourceManager(simulator.get()));

    ScheduleTask task0(priority0.get(),folderTape21/"task0",10,0,10);
    ScheduleTask task1(priority1.get(),folderTape22/"task1",10,1,10);

    bool busy;


    boost::posix_time::ptime begin0 = task0.Begin();
    boost::posix_time::ptime begin1 = task1.Begin();

    task0.Start();
    task1.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( begin0 == task0.Begin() );
    CPPUNIT_ASSERT( begin1 == task1.Begin() );


    CPPUNIT_ASSERT( false == priority0->Enabled() );
    CPPUNIT_ASSERT( false == priority1->Enabled() );
    CPPUNIT_ASSERT(
            true == simulator->DeregisterTape(testTape21) );
    CPPUNIT_ASSERT(
            true == simulator->DeregisterTape(testTape22) );
    CPPUNIT_ASSERT( false == priority0->Enabled() );
    CPPUNIT_ASSERT( false == priority1->Enabled() );

    CPPUNIT_ASSERT(
            true == simulator->RegisterTape(testTape21,priority0.get()) );
    CPPUNIT_ASSERT(
            true == simulator->RegisterTape(testTape22,priority1.get()) );
    CPPUNIT_ASSERT( false == priority0->Enabled() );
    CPPUNIT_ASSERT( false == priority1->Enabled() );
    CPPUNIT_ASSERT(
            false == simulator->RegisterTape(testTape21,priority0.get()) );
    CPPUNIT_ASSERT(
            false == simulator->RegisterTape(testTape22,priority1.get()) );
    CPPUNIT_ASSERT( false == priority0->Enabled() );
    CPPUNIT_ASSERT( false == priority1->Enabled() );

    CPPUNIT_ASSERT(
            true == simulator->DeregisterTape(testTape21) );
    CPPUNIT_ASSERT(
            true == simulator->DeregisterTape(testTape22) );

    CPPUNIT_ASSERT( false == priority0->Enabled() );
    CPPUNIT_ASSERT( false == priority1->Enabled() );
    CPPUNIT_ASSERT(
            true == simulator->RegisterTape(testTape21,priority0.get()) );
    CPPUNIT_ASSERT(
            true == simulator->RegisterTape(testTape22,priority1.get()) );
    CPPUNIT_ASSERT( false == priority0->Enabled() );
    CPPUNIT_ASSERT( false == priority1->Enabled() );


    begin0 = task0.Begin();
    begin1 = task1.Begin();

    task0.Start();
    task1.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( begin0 == task0.Begin() );
    CPPUNIT_ASSERT( begin1 == task1.Begin() );


    begin0 = task0.Begin();
    begin1 = task1.Begin();

    resource->RequestResource(testTape21,10,0);
    CPPUNIT_ASSERT( true == priority0->Enabled() );
    CPPUNIT_ASSERT( false == priority1->Enabled() );

    task1.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(10) );
    task0.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(5) );
    CPPUNIT_ASSERT( true == priority0->Enabled() );
    CPPUNIT_ASSERT( false == priority1->Enabled() );
    CPPUNIT_ASSERT(
            false == simulator->DeregisterTape(testTape21) );
    CPPUNIT_ASSERT( true == priority0->Enabled() );
    CPPUNIT_ASSERT( false == priority1->Enabled() );
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(15) );
    CPPUNIT_ASSERT( true == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( begin0 < task0.Begin() );
    CPPUNIT_ASSERT( begin1 == task1.Begin() );
    begin0 = task0.Begin();


    begin0 = task0.Begin();
    begin1 = task1.Begin();

    resource->RequestResource(testTape22,10,1);

    CPPUNIT_ASSERT( false == priority0->Enabled() );
    CPPUNIT_ASSERT( true == priority1->Enabled() );

    task0.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(10) );
    task1.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(5) );
    CPPUNIT_ASSERT( false == priority0->Enabled() );
    CPPUNIT_ASSERT( true == priority1->Enabled() );
    CPPUNIT_ASSERT(
            false == simulator->DeregisterTape(testTape22) );
    CPPUNIT_ASSERT( false == priority0->Enabled() );
    CPPUNIT_ASSERT( true == priority1->Enabled() );
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(15) );
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( true == task1.Finished() );
    CPPUNIT_ASSERT( begin0 == task0.Begin() );
    CPPUNIT_ASSERT( begin1 < task1.Begin() );

    CPPUNIT_ASSERT( false == priority0->Enabled() );
    CPPUNIT_ASSERT( true == priority1->Enabled() );
    CPPUNIT_ASSERT(
            true == simulator->DeregisterTape(testTape21) );
    CPPUNIT_ASSERT(
            true == simulator->DeregisterTape(testTape22) );
    CPPUNIT_ASSERT( false == priority0->Enabled() );
    CPPUNIT_ASSERT( false == priority1->Enabled() );
}


void
ResourceManagerTest::testSingleDrive()
{
    auto_ptr<PriorityTape> priority0(new PriorityTape());
    auto_ptr<PriorityTape> priority1(new PriorityTape());
    auto_ptr<ResourceTapeSimulator> simulator(new ResourceTapeSimulator());
    auto_ptr<ResourceManager> resource(new ResourceManager(simulator.get()));

    ScheduleTask task0(priority0.get(),folderTape21/"task0",10,0,10);
    ScheduleTask task1(priority1.get(),folderTape22/"task1",10,1,10);


    boost::posix_time::ptime begin0 = task0.Begin();
    boost::posix_time::ptime begin1 = task1.Begin();

    task0.Start();
    task1.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( begin0 == task0.Begin() );
    CPPUNIT_ASSERT( begin1 == task1.Begin() );


    begin0 = task0.Begin();
    begin1 = task1.Begin();

    CPPUNIT_ASSERT(
            true == simulator->RegisterTape(testTape21,priority0.get()) );
    CPPUNIT_ASSERT(
            true == simulator->RegisterTape(testTape22,priority1.get()) );
    CPPUNIT_ASSERT(
            false == simulator->RegisterTape(testTape21,priority0.get()) );
    CPPUNIT_ASSERT(
            false == simulator->RegisterTape(testTape22,priority1.get()) );

    task0.Start();
    task1.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( begin0 == task0.Begin() );
    CPPUNIT_ASSERT( begin1 == task1.Begin() );


    begin0 = task0.Begin();
    begin1 = task1.Begin();

    resource->RequestResource(testTape21,10,0);

    task1.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(10) );
    task0.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( true == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( begin0 < task0.Begin() );
    CPPUNIT_ASSERT( begin1 == task1.Begin() );
    begin0 = task0.Begin();


    begin0 = task0.Begin();
    begin1 = task1.Begin();

    resource->RequestResource(testTape22,10,1);

    task0.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(10) );
    task1.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( true == task1.Finished() );
    CPPUNIT_ASSERT( begin0 == task0.Begin() );
    CPPUNIT_ASSERT( begin1 < task1.Begin() );
}


void
ResourceManagerTest::testMultipleDrive()
{
    auto_ptr<PriorityTape> priority0(new PriorityTape());
    auto_ptr<PriorityTape> priority1(new PriorityTape());
    auto_ptr<ResourceTapeSimulator> simulator(new ResourceTapeSimulator());
    auto_ptr<ResourceManager> resource(new ResourceManager(simulator.get()));

    ScheduleTask task0(priority0.get(),folderTape11/"task0",10,0,10);
    ScheduleTask task1(priority1.get(),folderTape12/"task1",10,1,10);


    boost::posix_time::ptime begin0 = task0.Begin();
    boost::posix_time::ptime begin1 = task1.Begin();

    task0.Start();
    task1.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( begin0 == task0.Begin() );
    CPPUNIT_ASSERT( begin1 == task1.Begin() );


    begin0 = task0.Begin();
    begin1 = task1.Begin();

    CPPUNIT_ASSERT(
            true == simulator->RegisterTape(testTape11,priority0.get()) );
    CPPUNIT_ASSERT(
            true == simulator->RegisterTape(testTape12,priority1.get()) );
    CPPUNIT_ASSERT(
            false == simulator->RegisterTape(testTape11,priority0.get()) );
    CPPUNIT_ASSERT(
            false == simulator->RegisterTape(testTape12,priority1.get()) );

    task0.Start();
    task1.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( begin0 == task0.Begin() );
    CPPUNIT_ASSERT( begin1 == task1.Begin() );


    begin0 = task0.Begin();
    begin1 = task1.Begin();

    resource->RequestResource(testTape11,10,0);

    task0.Start();
    task1.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( true == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( begin0 < task0.Begin() );
    CPPUNIT_ASSERT( begin1 == task1.Begin() );


    begin0 = task0.Begin();
    begin1 = task1.Begin();

    resource->RequestResource(testTape11,10,1);
    task0.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(2) );
    resource->RequestResource(testTape12,10,1);
    task1.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( true == task0.Finished() );
    CPPUNIT_ASSERT( true == task1.Finished() );
    CPPUNIT_ASSERT( begin0 < task0.Begin() );
    CPPUNIT_ASSERT( begin1 < task1.Begin() );
}


void
ResourceManagerTest::testMultipleChanger()
{
    auto_ptr<PriorityTape> priority0(new PriorityTape());
    auto_ptr<PriorityTape> priority1(new PriorityTape());
    auto_ptr<PriorityTape> priority2(new PriorityTape());
    auto_ptr<PriorityTape> priority3(new PriorityTape());
    auto_ptr<ResourceTapeSimulator> simulator(new ResourceTapeSimulator());
    auto_ptr<ResourceManager> resource(new ResourceManager(simulator.get()));

    ScheduleTask task0(priority0.get(),folderTape11/"task0",10,0,10);
    ScheduleTask task1(priority1.get(),folderTape12/"task1",10,1,10);
    ScheduleTask task2(priority2.get(),folderTape21/"task2",10,2,10);
    ScheduleTask task3(priority3.get(),folderTape22/"task3",10,3,10);


    boost::posix_time::ptime begin0 = task0.Begin();
    boost::posix_time::ptime begin1 = task1.Begin();
    boost::posix_time::ptime begin2 = task2.Begin();
    boost::posix_time::ptime begin3 = task3.Begin();

    task0.Start();
    task1.Start();
    task2.Start();
    task3.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( false == task2.Finished() );
    CPPUNIT_ASSERT( false == task3.Finished() );
    CPPUNIT_ASSERT( begin0 == task0.Begin() );
    CPPUNIT_ASSERT( begin1 == task1.Begin() );
    CPPUNIT_ASSERT( begin2 == task2.Begin() );
    CPPUNIT_ASSERT( begin3 == task3.Begin() );


    begin0 = task0.Begin();
    begin1 = task1.Begin();
    begin2 = task2.Begin();
    begin3 = task3.Begin();

    CPPUNIT_ASSERT(
            true == simulator->RegisterTape(testTape11,priority0.get()) );
    CPPUNIT_ASSERT(
            true == simulator->RegisterTape(testTape12,priority1.get()) );
    CPPUNIT_ASSERT(
            true == simulator->RegisterTape(testTape21,priority2.get()) );
    CPPUNIT_ASSERT(
            true == simulator->RegisterTape(testTape22,priority3.get()) );
    CPPUNIT_ASSERT(
            false == simulator->RegisterTape(testTape11,priority0.get()) );
    CPPUNIT_ASSERT(
            false == simulator->RegisterTape(testTape12,priority1.get()) );
    CPPUNIT_ASSERT(
            false == simulator->RegisterTape(testTape21,priority2.get()) );
    CPPUNIT_ASSERT(
            false == simulator->RegisterTape(testTape22,priority3.get()) );

    task0.Start();
    task1.Start();
    task2.Start();
    task3.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( false == task2.Finished() );
    CPPUNIT_ASSERT( false == task3.Finished() );
    CPPUNIT_ASSERT( begin0 == task0.Begin() );
    CPPUNIT_ASSERT( begin1 == task1.Begin() );
    CPPUNIT_ASSERT( begin2 == task2.Begin() );
    CPPUNIT_ASSERT( begin3 == task3.Begin() );


    begin0 = task0.Begin();
    begin1 = task1.Begin();
    begin2 = task2.Begin();
    begin3 = task3.Begin();

    resource->RequestResource(testTape11,10,0);
    resource->RequestResource(testTape21,10,0);

    task0.Start();
    task1.Start();
    task2.Start();
    task3.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( true == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( true == task2.Finished() );
    CPPUNIT_ASSERT( false == task3.Finished() );
    CPPUNIT_ASSERT( begin0 < task0.Begin() );
    CPPUNIT_ASSERT( begin1 == task1.Begin() );
    CPPUNIT_ASSERT( begin2 < task2.Begin() );
    CPPUNIT_ASSERT( begin3 == task3.Begin() );


    begin0 = task0.Begin();
    begin1 = task1.Begin();
    begin2 = task2.Begin();
    begin3 = task3.Begin();

    resource->RequestResource(testTape11,10,1);
    task0.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(2) );
    resource->RequestResource(testTape12,10,1);
    task1.Start();
    resource->RequestResource(testTape22,10,1);
    task2.Start();
    task3.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(20) );
    CPPUNIT_ASSERT( true == task0.Finished() );
    CPPUNIT_ASSERT( true == task1.Finished() );
    CPPUNIT_ASSERT( false == task2.Finished() );
    CPPUNIT_ASSERT( true == task3.Finished() );
    CPPUNIT_ASSERT( begin0 < task0.Begin() );
    CPPUNIT_ASSERT( begin1 < task1.Begin() );
    CPPUNIT_ASSERT( begin2 == task2.Begin() );
    CPPUNIT_ASSERT( begin3 < task3.Begin() );
}

