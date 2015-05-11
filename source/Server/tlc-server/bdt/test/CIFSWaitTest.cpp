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
 * CIFSWaitTest.cpp
 *
 *  Created on: Apr 28, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../CIFSWait.h"
#include "CIFSWaitTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CIFSWaitTest );


void
CIFSWaitTest::setUp()
{
}


void
CIFSWaitTest::tearDown()
{
}


void
CIFSWaitTest::testWait()
{
    auto_ptr<CIFSWait> wait;
    int timeWait = 0;


    wait.reset( new CIFSWait(30) );
    CPPUNIT_ASSERT( NULL != wait.get() );

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 30 == timeWait );
    wait->SetWait(128*1024,timeWait);

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 60 == timeWait );
    wait->SetWait(128*1024,timeWait);

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 90 == timeWait );
    wait->SetWait(128*1024,timeWait);

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 120 == timeWait );
    wait->SetWait(128*1024,timeWait);

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 150 == timeWait );


    wait->SetWait(0,60);
    wait->SetWait(128*1024,30);

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 90 == timeWait );
    wait->SetWait(128*1024,timeWait);

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 150 == timeWait );
    wait->SetWait(128*1024,timeWait);

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 210 == timeWait );
    wait->SetWait(128*1024,timeWait);

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 270 == timeWait );


    wait.reset( new CIFSWait(60) );
    CPPUNIT_ASSERT( NULL != wait.get() );

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 60 == timeWait );
    wait->SetWait(128*1024,timeWait);

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 120 == timeWait );
    wait->SetWait(128*1024,timeWait);

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 180 == timeWait );
    wait->SetWait(128*1024,timeWait);

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 240 == timeWait );
    wait->SetWait(128*1024,timeWait);

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 300 == timeWait );
    wait->SetWait(128*1024,timeWait);


    wait.reset( new CIFSWait(0) );
    CPPUNIT_ASSERT( NULL != wait.get() );

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 0 == timeWait );

    wait->SetWait(128*1024,60);

    timeWait = wait->GetWait(128*1024);
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(timeWait), 0 == timeWait );
}
