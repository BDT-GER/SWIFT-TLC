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
 * PriorityTapeTest.cpp
 *
 *  Created on: Jul 18, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../PriorityTape.h"
#include "PriorityTapeTask.h"
#include "PriorityTapeTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( PriorityTapeTest );


void
PriorityTapeTest::setUp()
{
    priority_.reset(new PriorityTape());
}


void
PriorityTapeTest::tearDown()
{
}


void
PriorityTapeTest::testSchedule()
{
    boost::posix_time::ptime begin;
    boost::posix_time::ptime current;
    int duration;


    CPPUNIT_ASSERT( false == priority_->Busy() );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == priority_->Request(true,5,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 5 );

    CPPUNIT_ASSERT( false == priority_->Busy() );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == priority_->Request(false,5,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 5 );

    CPPUNIT_ASSERT( false == priority_->Busy() );


    CPPUNIT_ASSERT( true == priority_->Enable(true) );
    CPPUNIT_ASSERT( false == priority_->Busy() );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == priority_->Request(true,5,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    CPPUNIT_ASSERT( true == priority_->Busy() );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == priority_->Request(true,5,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == priority_->Request(false,5,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == priority_->Request(false,5,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 5 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == priority_->Request(true,5,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    priority_->Release(true);
    CPPUNIT_ASSERT( true == priority_->Busy() );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == priority_->Request(false,5,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 5 );

    priority_->Release(true);
    CPPUNIT_ASSERT( true == priority_->Busy() );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == priority_->Request(false,5,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 5 );

    priority_->Release(false);
    CPPUNIT_ASSERT( true == priority_->Busy() );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == priority_->Request(false,5,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == priority_->Request(false,5,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 5 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == priority_->Request(true,5,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    CPPUNIT_ASSERT( true == priority_->Enable(false) );
    CPPUNIT_ASSERT( true == priority_->Busy() );

    priority_->Release(true);

    CPPUNIT_ASSERT( true == priority_->Enable(false) );
    CPPUNIT_ASSERT( true == priority_->Busy() );

    priority_->Release(false);

    CPPUNIT_ASSERT( true == priority_->Enable(false) );
    CPPUNIT_ASSERT( true == priority_->Busy() );

    priority_->Release(true);

    CPPUNIT_ASSERT( true == priority_->Enable(false) );
    CPPUNIT_ASSERT( false == priority_->Busy() );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == priority_->Request(true,5,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 5 );

    CPPUNIT_ASSERT( false == priority_->Busy() );
}


void
PriorityTapeTest::testPriority()
{
    boost::posix_time::ptime begin;
    boost::posix_time::ptime current;
    int duration;


    CPPUNIT_ASSERT( true == priority_->Enable(true) );

    PriorityTapeTask task0(priority_.get(),100,0,10);
    PriorityTapeTask task1(priority_.get(),100,1,10);
    PriorityTapeTask task2(priority_.get(),100,2,10);
    PriorityTapeTask task3(priority_.get(),100,3,10);
    PriorityTapeTask task4(priority_.get(),100,4,10);
    PriorityTapeTask task5(priority_.get(),100,5,10);

    task0.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(5) );
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
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(100) );
    CPPUNIT_ASSERT( true == task0.Finished() );
    CPPUNIT_ASSERT( true == task5.Finished() );
    CPPUNIT_ASSERT( true == task4.Finished() );
    CPPUNIT_ASSERT( true == task3.Finished() );
    CPPUNIT_ASSERT( true == task2.Finished() );
    CPPUNIT_ASSERT( true == task1.Finished() );

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
PriorityTapeTest::testEnable()
{
    boost::posix_time::ptime begin;
    int wait;
    PriorityTapeTask task(priority_.get(),10,0,5);

    priority_->Enable(false);
    begin = boost::posix_time::microsec_clock::local_time();;
    task.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(2) );
    CPPUNIT_ASSERT( false == priority_->Busy() );
    CPPUNIT_ASSERT( true == priority_->Enable(true) );
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(2) );
    CPPUNIT_ASSERT( true == priority_->Busy() );
    CPPUNIT_ASSERT( false == task.Finished() );
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(5) );
    CPPUNIT_ASSERT( false == priority_->Busy() );
    CPPUNIT_ASSERT( true == task.Finished() );
    wait = (task.Begin() - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(wait),
            wait >= 2 && wait <= 5 );
}

