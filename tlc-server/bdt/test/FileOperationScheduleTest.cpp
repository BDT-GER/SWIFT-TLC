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
 * FileOperationScheduleTest.cpp
 *
 *  Created on: May 7, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../ScheduleElevator.h"
#include "../FileOperationSchedule.h"
#include "../FileOperation.h"
#include "FileOperationScheduleTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( FileOperationScheduleTest );


static const string testFile = "test-file";
static const string testFolder = "test-folder";
static const string testFolderFile = testFolder + "/" + testFile;

static const fs::path fileTest(testFile);
static const fs::path folderTest(testFolder);
static const fs::path fileFolderTest(testFolderFile);


void
FileOperationScheduleTest::setUp()
{
}


void
FileOperationScheduleTest::tearDown()
{
    fs::remove(fileTest);
    fs::remove_all(folderTest);
}


void
FileOperationScheduleTest::testSchedule()
{
    auto_ptr<ScheduleInterface> schedule( new ScheduleElevator(5,10) );
    boost::posix_time::ptime begin;
    boost::posix_time::ptime current;
    fs::path file1 = fileTest;
    fs::path file2 = fileFolderTest;

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file1,true,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    int duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file2,true,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == schedule->Request(file1,0,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 20 );

    schedule->Release(file1,true);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file1,0,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == schedule->Request(file2,0,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 20 );

    schedule->Release(file2,true);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file2,0,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 10 && duration < 20 );

    schedule->Release(file1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file1,1024,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 10 && duration < 20 );

    schedule->Release(file1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file1,2048,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == schedule->Request(file1,2048,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 20 );

    schedule->Release(file1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file1,3072,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    schedule->Release(file1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file1,4096,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    schedule->Release(file1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file1,1024,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 5 && duration < 10 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == schedule->Request(file1,true,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 20 );

    schedule->Release(file1,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file1,true,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    schedule->Release(file2,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file2,1024,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 && duration < 20 );

    schedule->Release(file2,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file2,2048,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == schedule->Request(file2,3072,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 20 );

    schedule->Release(file2,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file2,3072,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    schedule->Release(file2,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file2,4096,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    schedule->Release(file2,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file2,1024,1024,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 5 && duration < 10 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == schedule->Request(file2,true,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 20 );

    schedule->Release(file2,false);

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == schedule->Request(file2,true,20,0) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );
}


void
FileOperationScheduleTest::testFileOperation()
{
    auto_ptr<ScheduleInterface> schedule( new ScheduleElevator(5,10) );
    auto_ptr<FileOperationInterface> file1( new FileOperationSchedule(
            fileTest, new FileOperation(fileTest), schedule.get(), 20, 0 ) );
    auto_ptr<FileOperationInterface> file2( new FileOperationSchedule(
            fileFolderTest, new FileOperation(fileFolderTest),
            schedule.get(), 20, 0 ) );
    boost::posix_time::ptime begin;
    boost::posix_time::ptime current;


    CPPUNIT_ASSERT( false == file1->OpenFile(O_RDWR) );
    CPPUNIT_ASSERT( false == file2->OpenFile(O_RDWR) );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == file1->CreateFile(O_RDWR,0644,false) );
    current = boost::posix_time::microsec_clock::local_time();
    int duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    errno = 0;
    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == file2->CreateFile(O_RDWR,0644,false) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 10 && duration < 20 );
    CPPUNIT_ASSERT_ERRNO_( ENOENT == errno );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == file2->CreateFile(O_RDWR,0644,true) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration <= 1 );


    char bufferWrite[1024] = "Hello,world!\n";
    char bufferRead[sizeof(bufferWrite)];
    size_t size;

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file1->Write(
                0,bufferWrite,sizeof(bufferWrite),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 10 && duration < 20 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file2->Write(
                0,bufferWrite,sizeof(bufferWrite),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 && duration < 20 );


    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file1->Write(
                sizeof(bufferWrite),bufferWrite,sizeof(bufferWrite),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 && duration < 20 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file1->Write(
                2*sizeof(bufferWrite),bufferWrite,sizeof(bufferWrite),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file1->Write(
                0,bufferWrite,sizeof(bufferWrite),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 5 && duration < 10 );


    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file2->Write(
                sizeof(bufferWrite),bufferWrite,sizeof(bufferWrite),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 && duration < 20 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file2->Write(
                2*sizeof(bufferWrite),bufferWrite,sizeof(bufferWrite),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file2->Write(
                0,bufferWrite,sizeof(bufferWrite),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 5 && duration < 10 );


    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file1->Read(
                0,bufferRead,sizeof(bufferRead),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 && duration < 20 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file1->Read(
                sizeof(bufferRead),bufferRead,sizeof(bufferRead),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file1->Read(
                2*sizeof(bufferRead),bufferRead,sizeof(bufferRead),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file1->Read(
                0,bufferRead,sizeof(bufferRead),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 5 && duration < 10 );


    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file2->Read(
                0,bufferRead,sizeof(bufferRead),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 10 && duration < 20 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file2->Read(
                sizeof(bufferRead),bufferRead,sizeof(bufferRead),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file2->Read(
                2*sizeof(bufferRead),bufferRead,sizeof(bufferRead),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file2->Read(
                0,bufferRead,sizeof(bufferRead),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration >= 5 && duration < 10 );


    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == file1->Truncate(sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == file2->Truncate(sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );


    schedule.reset( new ScheduleElevator(5,10) );
    file1.reset( new FileOperationSchedule(
            fileTest, new FileOperation(fileTest), schedule.get(), 20, 0 ) );
    file2.reset( new FileOperationSchedule(
            fileFolderTest, new FileOperation(fileFolderTest),
            schedule.get(), 20, 0 ) );


    errno = 0;
    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == file1->CreateFile(O_RDWR,0644,false) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration <= 1 );
    CPPUNIT_ASSERT_ERRNO_( EEXIST == errno );

    errno = 0;
    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( false == file2->CreateFile(O_RDWR,0644,false) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 10 && duration < 20 );
    CPPUNIT_ASSERT_ERRNO_( EEXIST == errno );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == file1->OpenFile(O_RDWR) );
    CPPUNIT_ASSERT( true == file2->OpenFile(O_RDWR) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration <= 1 );


    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file1->Read(
                0,bufferRead,sizeof(bufferRead),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 10 && duration < 20 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == file2->Read(
                0,bufferRead,sizeof(bufferRead),size) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= 10 && duration < 20 );


    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == file1->Delete() );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == file2->Delete() );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT( duration <= 1 );
}

