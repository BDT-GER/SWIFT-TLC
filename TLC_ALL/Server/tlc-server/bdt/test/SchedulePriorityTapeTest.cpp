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
 * SchedulePriorityTapeTest.cpp
 *
 *  Created on: Jul 20, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../SchedulePriorityTape.h"
#include "../ResourceTapeSimulator.h"
#include "SchedulePriorityTapeTest.h"
#include "ScheduleTask.h"


CPPUNIT_TEST_SUITE_REGISTRATION( SchedulePriorityTapeTest );

const string tapeFolder = "tape.folder";
const string testTape11 = "01000001";    // changer 1 with 2 drives
const string testTape12 = "01000002";    // changer 1 with 2 drives
const string testTape21 = "02000001";    // changer 2 with 1 drive
const string testTape22 = "02000002";    // changer 2 with 1 drive


void
SchedulePriorityTapeTest::setUp()
{
    Factory::ReleaseTapeLibraryManager();
    Factory::SetTapeFolder(tapeFolder);
    Factory::CreateTapeLibraryManager();

    schedule_.reset( new SchedulePriorityTape(new ResourceTapeSimulator()) );
}


void
SchedulePriorityTapeTest::tearDown()
{
    Factory::ReleaseTapeLibraryManager();
}


void
SchedulePriorityTapeTest::testPriorityMultipleDrive()
{
    int duration;

    ScheduleTask task0(schedule_.get(),testTape11,50,0,10);
    ScheduleTask task1(schedule_.get(),testTape12,50,1,10);
    ScheduleTask task2(schedule_.get(),testTape11,50,2,10);
    ScheduleTask task3(schedule_.get(),testTape12,50,3,10);
    ScheduleTask task4(schedule_.get(),testTape11,50,4,10);
    ScheduleTask task5(schedule_.get(),testTape12,50,5,10);

    task0.Start();
    task1.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(2) );
    task2.Start();
    task3.Start();
    task4.Start();
    task5.Start();
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( false == task2.Finished() );
    CPPUNIT_ASSERT( false == task3.Finished() );
    CPPUNIT_ASSERT( false == task4.Finished() );
    CPPUNIT_ASSERT( false == task5.Finished() );
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(100) );
    CPPUNIT_ASSERT( true == task0.Finished() );
    CPPUNIT_ASSERT( true == task1.Finished() );
    CPPUNIT_ASSERT( true == task2.Finished() );
    CPPUNIT_ASSERT( true == task3.Finished() );
    CPPUNIT_ASSERT( true == task4.Finished() );
    CPPUNIT_ASSERT( true == task5.Finished() );

    duration = (task0.End() - task0.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task1.End() - task1.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task2.End() - task2.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task3.End() - task3.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task4.End() - task4.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task5.End() - task5.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    CPPUNIT_ASSERT( task0.End() <= task4.Begin() );
    CPPUNIT_ASSERT( task1.End() <= task5.Begin() );
    CPPUNIT_ASSERT( task5.End() <= task3.Begin() );
    CPPUNIT_ASSERT( task4.End() <= task2.Begin() );
}


void
SchedulePriorityTapeTest::testPrioritySingleDrive()
{
    LogDebug("Begin test now...");
    int duration;

    ScheduleTask task0(schedule_.get(),testTape21,100,0,10);
    ScheduleTask task1(schedule_.get(),testTape22,100,1,10);
    ScheduleTask task2(schedule_.get(),testTape21,100,2,10);
    ScheduleTask task3(schedule_.get(),testTape22,100,3,10);
    ScheduleTask task4(schedule_.get(),testTape21,100,4,10);
    ScheduleTask task5(schedule_.get(),testTape22,100,5,10);

    LogDebug("Begin test now 1...");

    task0.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(2) );
    task1.Start();
    task2.Start();
    task3.Start();
    task4.Start();
    task5.Start();
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( false == task2.Finished() );
    CPPUNIT_ASSERT( false == task3.Finished() );
    CPPUNIT_ASSERT( false == task4.Finished() );
    CPPUNIT_ASSERT( false == task5.Finished() );
    LogDebug("Begin test now 2...");
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(100) );
    CPPUNIT_ASSERT( true == task0.Finished() );
    CPPUNIT_ASSERT( true == task5.Finished() );
    CPPUNIT_ASSERT( true == task4.Finished() );
    CPPUNIT_ASSERT( true == task3.Finished() );
    CPPUNIT_ASSERT( true == task2.Finished() );
    CPPUNIT_ASSERT( true == task1.Finished() );
    LogDebug("Begin test now 3...");

    duration = (task0.End() - task0.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task1.End() - task1.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task2.End() - task2.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task3.End() - task3.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task4.End() - task4.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task5.End() - task5.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );

    CPPUNIT_ASSERT( task0.End() <= task5.Begin() );
    CPPUNIT_ASSERT( task5.End() <= task4.Begin() );
    CPPUNIT_ASSERT( task4.End() <= task3.Begin() );
    CPPUNIT_ASSERT( task3.End() <= task2.Begin() );
    CPPUNIT_ASSERT( task2.End() <= task1.Begin() );
}


void
SchedulePriorityTapeTest::testPriorityMultipleChanger()
{
    int duration;

    ScheduleTask task0(schedule_.get(),testTape11,100,0,10);
    ScheduleTask task1(schedule_.get(),testTape12,100,1,10);
    ScheduleTask task2(schedule_.get(),testTape21,100,2,10);
    ScheduleTask task3(schedule_.get(),testTape22,100,3,10);
    ScheduleTask task4(schedule_.get(),testTape11,100,4,10);
    ScheduleTask task5(schedule_.get(),testTape12,100,5,10);
    ScheduleTask task6(schedule_.get(),testTape21,100,6,10);
    ScheduleTask task7(schedule_.get(),testTape22,100,7,10);
    ScheduleTask task8(schedule_.get(),testTape11,100,8,10);
    ScheduleTask task9(schedule_.get(),testTape12,100,9,10);
    ScheduleTask taska(schedule_.get(),testTape21,100,10,10);
    ScheduleTask taskb(schedule_.get(),testTape22,100,11,10);

    task0.Start();
    task1.Start();
    task2.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(2) );
    task3.Start();
    task4.Start();
    task5.Start();
    task6.Start();
    task7.Start();
    task8.Start();
    task9.Start();
    taska.Start();
    taskb.Start();
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( false == task2.Finished() );
    CPPUNIT_ASSERT( false == task3.Finished() );
    CPPUNIT_ASSERT( false == task4.Finished() );
    CPPUNIT_ASSERT( false == task5.Finished() );
    CPPUNIT_ASSERT( false == task6.Finished() );
    CPPUNIT_ASSERT( false == task7.Finished() );
    CPPUNIT_ASSERT( false == task8.Finished() );
    CPPUNIT_ASSERT( false == task9.Finished() );
    CPPUNIT_ASSERT( false == taska.Finished() );
    CPPUNIT_ASSERT( false == taskb.Finished() );
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(200) );
    CPPUNIT_ASSERT( true == task0.Finished() );
    CPPUNIT_ASSERT( true == task1.Finished() );
    CPPUNIT_ASSERT( true == task2.Finished() );
    CPPUNIT_ASSERT( true == task3.Finished() );
    CPPUNIT_ASSERT( true == task4.Finished() );
    CPPUNIT_ASSERT( true == task5.Finished() );
    CPPUNIT_ASSERT( true == task6.Finished() );
    CPPUNIT_ASSERT( true == task7.Finished() );
    CPPUNIT_ASSERT( true == task8.Finished() );
    CPPUNIT_ASSERT( true == task9.Finished() );
    CPPUNIT_ASSERT( true == taska.Finished() );
    CPPUNIT_ASSERT( true == taskb.Finished() );

    duration = (task0.End() - task0.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task1.End() - task1.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task2.End() - task2.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task3.End() - task3.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task4.End() - task4.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task5.End() - task5.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task6.End() - task6.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task7.End() - task7.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task8.End() - task8.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (task9.End() - task9.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (taska.End() - taska.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );
    duration = (taskb.End() - taskb.Begin()).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 );

    CPPUNIT_ASSERT( task0.End() <= task8.Begin() );
    CPPUNIT_ASSERT( task8.End() <= task4.Begin() );
    CPPUNIT_ASSERT( task1.End() <= task9.Begin() );
    CPPUNIT_ASSERT( task9.End() <= task5.Begin() );
    CPPUNIT_ASSERT( task2.End() <= taskb.Begin() );
    CPPUNIT_ASSERT( taskb.End() <= taska.Begin() );
    CPPUNIT_ASSERT( taska.End() <= task7.Begin() );
    CPPUNIT_ASSERT( task7.End() <= task6.Begin() );
    CPPUNIT_ASSERT( task6.End() <= task3.Begin() );
}

