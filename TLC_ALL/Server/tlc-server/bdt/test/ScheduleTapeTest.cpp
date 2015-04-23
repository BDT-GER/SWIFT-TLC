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
 * ScheduleTapeTest.cpp
 *
 *  Created on: May 21, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "ScheduleTapeTest.h"
#include "../ScheduleTape.h"


CPPUNIT_TEST_SUITE_REGISTRATION( ScheduleTapeTest );


const string testTapeFolder = "test.tape.folder";
const string testTape1 = "barcode1";
const string testTape2 = "barcode2";
const string testFile = "/file";
const string testFolderFile = "/folder";

const fs::path folderTape = testTapeFolder;
const fs::path fileTest1(folderTape/testTape1/testFile);
const fs::path fileFolderTest1(folderTape/testTape1/testFolderFile);
const fs::path fileTest2(folderTape/testTape2/testFile);
const fs::path fileFolderTest2(folderTape/testTape2/testFolderFile);


void
ScheduleTapeTest::setUp()
{
    Factory::SetTapeFolder(testTapeFolder);
}


void
ScheduleTapeTest::tearDown()
{
}


void
ScheduleTapeTest::testOperationTape()
{
    auto_ptr<ScheduleInterface> schedule;
    boost::posix_time::ptime begin;
    boost::posix_time::ptime current;
    int duration;

    schedule.reset( new ScheduleTape(5,10,20) );


    CPPUNIT_ASSERT( false == schedule->Request(testTapeFolder,false,25,0) );
    CPPUNIT_ASSERT( false == schedule->Request(testTapeFolder,true,25,0) );
    CPPUNIT_ASSERT( false == schedule->Request(testTape1,false,25,0) );
    CPPUNIT_ASSERT( false == schedule->Request(testTape1,true,25,0) );
    CPPUNIT_ASSERT( false == schedule->Request(testFile,false,25,0) );
    CPPUNIT_ASSERT( false == schedule->Request(testFile,true,25,0) );
    CPPUNIT_ASSERT( false == schedule->Request(testFolderFile,false,25,0) );
    CPPUNIT_ASSERT( false == schedule->Request(testFolderFile,true,25,0) );
    CPPUNIT_ASSERT( false == schedule->Request(testTapeFolder,0,1024,25,0) );
    CPPUNIT_ASSERT( false == schedule->Request(testTape1,0,1024,25,0) );
    CPPUNIT_ASSERT( false == schedule->Request(testFile,0,1024,25,0) );
    CPPUNIT_ASSERT( false == schedule->Request(testFolderFile,0,1024,25,0) );


    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest1,true,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileFolderTest1,true,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == schedule->Request(fileTest2,true,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 30 );

    schedule->Release(fileTest1,true);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == schedule->Request(fileTest2,true,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 30 );

    schedule->Release(fileFolderTest1,true);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest2,true,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 20 && duration < 30 );

    schedule->Release(fileTest2,true);


    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest1,false,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 20 && duration < 30 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == schedule->Request(fileTest1,false,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 30 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest1,true,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration <= 1 );

    schedule->Release(fileTest1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest1,false,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration <= 1 );

    schedule->Release(fileTest1,true);
    schedule->Release(fileTest1,false);
}


void
ScheduleTapeTest::testOperationFile()
{
    auto_ptr<ScheduleInterface> schedule;
    boost::posix_time::ptime begin;
    boost::posix_time::ptime current;
    int duration;

    schedule.reset( new ScheduleTape(5,10,20) );


    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest1,true,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    schedule->Release(fileTest1,true);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest1,0,1024,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == schedule->Request(fileTest2,true,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 30 );

    schedule->Release(fileTest1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest2,true,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 20 && duration < 30 );

    schedule->Release(fileTest2,true);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest2,0,1024,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration <= 1 );

    schedule->Release(fileTest2,false);


    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest1,true,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 20 && duration < 30 );

    schedule->Release(fileTest1,true);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest1,0,1024,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    schedule->Release(fileTest1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest1,1024,1024,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    schedule->Release(fileTest1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest1,0,1024,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 5 && duration < 10 );

    schedule->Release(fileTest1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest1,0,1024,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 5 && duration < 10 );

    schedule->Release(fileTest1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileTest1,2048,1024,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    schedule->Release(fileTest1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileFolderTest1,true,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    schedule->Release(fileFolderTest1,true);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileFolderTest1,0,1024,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 10 && duration < 20 );

    schedule->Release(fileFolderTest1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(fileFolderTest1,1024,1024,30,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    schedule->Release(fileFolderTest1,false);
}

