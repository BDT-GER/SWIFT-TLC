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
 * PriorityTapeGroupTest.cpp
 *
 *  Created on: 2013-5-23
 *      Author: more
 */


#include "stdafx.h"
#include "../PriorityTape.h"
#include "../PriorityTapeGroup.h"
#include "PriorityTapeTask.h"
#include "PriorityTapeGroupTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( PriorityTapeGroupTest );


void
PriorityTapeGroupTest::setUp()
{
}


void
PriorityTapeGroupTest::tearDown()
{
}


void
PriorityTapeGroupTest::testSchedule()
{
    auto_ptr<PriorityTape> priority0(new PriorityTape());
    auto_ptr<PriorityTape> priority1(new PriorityTape());
    auto_ptr<PriorityTape> priority2(new PriorityTape());
    auto_ptr<PriorityTape> priority3(new PriorityTape());

    PriorityTapeTask task0(priority0.get(),10,0,1);
    PriorityTapeTask task1(priority1.get(),10,0,1);
    PriorityTapeTask task2(priority2.get(),10,0,1);
    PriorityTapeTask task3(priority3.get(),10,0,1);

    vector<string> tapes;
    tapes.push_back("01000001");
    tapes.push_back("01000002");
    tapes.push_back("02000001");
    tapes.push_back("02000002");

    vector<PriorityTape *> priorities;
    priorities.push_back(priority0.get());
    priorities.push_back(priority1.get());
    priorities.push_back(priority2.get());
    priorities.push_back(priority3.get());

    auto_ptr<PriorityTapeGroup> group(new PriorityTapeGroup(tapes,priorities));


    task0.Start();
    task1.Start();
    task2.Start();
    task3.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(10) );
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( false == task2.Finished() );
    CPPUNIT_ASSERT( false == task3.Finished() );


    group->Enable(true);

    task0.Start();
    task1.Start();
    task2.Start();
    task3.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(10) );
    CPPUNIT_ASSERT( true == task0.Finished() );
    CPPUNIT_ASSERT( true == task1.Finished() );
    CPPUNIT_ASSERT( true == task2.Finished() );
    CPPUNIT_ASSERT( true == task3.Finished() );


    group->Enable(false);

    task0.Start();
    task1.Start();
    task2.Start();
    task3.Start();
    boost::this_thread::sleep(
            boost::posix_time::milliseconds(10) );
    CPPUNIT_ASSERT( false == task0.Finished() );
    CPPUNIT_ASSERT( false == task1.Finished() );
    CPPUNIT_ASSERT( false == task2.Finished() );
    CPPUNIT_ASSERT( false == task3.Finished() );


    boost::posix_time::ptime begin =
            boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == group->Request(false,10,0) );
    boost::posix_time::ptime end =
            boost::posix_time::microsec_clock::local_time();
    int duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 && duration <= 12 );


    group->Enable(true);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == group->Request(false,10,0) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 5 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == group->Request(false,10,0) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 10 && duration <= 12 );
    CPPUNIT_ASSERT( duration >= 10 && duration <= 12 );

    group->Release(false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == group->Request(false,10,0) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 5 );

    group->Release(false);


    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == group->Request(true,10,0) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 5 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == group->Request(true,10,0) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 5 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == group->Request(false,10,0) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 5 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == group->Request(false,10,0) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 10 && duration <= 12 );
    CPPUNIT_ASSERT( duration >= 10 && duration <= 12 );

    group->Release(true);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == group->Request(false,10,0) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 && duration <= 12 );

    group->Release(false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == group->Request(false,10,0) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 5 );

    group->Release(true);

    group->Release(false);
}
