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
 * ThrottleTest.cpp
 *
 *  Created on: Dec 18, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../Throttle.h"
#include "ThrottleTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( ThrottleTest );


void
ThrottleTest::setUp()
{
}


void
ThrottleTest::tearDown()
{
}


void
ThrottleTest::testRequest()
{
    auto_ptr<Throttle> throttle;

    throttle.reset(new Throttle(10,4096));

    boost::posix_time::ptime begin =
            boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == throttle->Request(8192) );
    boost::posix_time::ptime end =
            boost::posix_time::microsec_clock::local_time();
    int duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );
    begin = end;

    CPPUNIT_ASSERT( true == throttle->Request(0) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );
    begin = end;

    CPPUNIT_ASSERT( true == throttle->Request(1024) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 9 && duration <= 11 );
    begin = end;

    CPPUNIT_ASSERT( true == throttle->Request(1024) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 2 && duration <= 3 );
    begin = end;

    boost::this_thread::sleep(
            boost::posix_time::milliseconds(10) );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == throttle->Request(1024) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );
    begin = end;

    CPPUNIT_ASSERT( true == throttle->Request(1024) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );
    begin = end;

    CPPUNIT_ASSERT( true == throttle->Request(1024) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );
    begin = end;

    CPPUNIT_ASSERT( true == throttle->Request(1024) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration <= 1 );
    begin = end;

    CPPUNIT_ASSERT( true == throttle->Request(1024) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 2 && duration <= 3 );
    begin = end;

    CPPUNIT_ASSERT( true == throttle->Reset(10,4096) );
    CPPUNIT_ASSERT( true == throttle->Request(1024) );
    end = boost::posix_time::microsec_clock::local_time();
    duration = (end - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );
}

