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
 * FileOperationPriorityTest.cpp
 *
 *  Created on: Jul 26, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../SchedulePriorityTape.h"
#include "../ResourceTapeSimulator.h"
#include "../FileOperation.h"
#include "../FileOperationPriority.h"
#include "FileOperationPriorityTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( FileOperationPriorityTest );


static const string tapeFolder = "test.tape.folder";
static const string testTape1 = "02000001";
static const string testTape2 = "02000002";
static const string testFile1 = "test-file1";
static const string testFile2 = "test-file2";

static const fs::path folderTape(tapeFolder);
static const fs::path fileTest1Tape1(folderTape/testTape1/testFile1);
static const fs::path fileTest2Tape1(folderTape/testTape1/testFile2);
static const fs::path fileTest1Tape2(folderTape/testTape2/testFile1);
static const fs::path fileTest2Tape2(folderTape/testTape2/testFile2);

static int timeout = 10;


void
FileOperationPriorityTest::setUp()
{
    fs::remove_all(folderTape);

    fs::create_directory(folderTape);
    fs::create_directory(folderTape/testTape1);
    fs::create_directory(folderTape/testTape2);

    Factory::ReleaseTapeLibraryManager();
    Factory::SetTapeFolder(folderTape);
    Factory::CreateTapeLibraryManager();
}


void
FileOperationPriorityTest::tearDown()
{
    fs::remove_all(folderTape);

    Factory::ReleaseTapeLibraryManager();
}


void
FileOperationPriorityTest::testFileOperation()
{
    auto_ptr<SchedulePriorityTape> priority;
    auto_ptr<FileOperationPriority> fileHighPriority;
    auto_ptr<FileOperationPriority> fileLowPriority;

    priority.reset( new SchedulePriorityTape(new ResourceTapeSimulator()) );

    LogDebug("Step 1");
    fileHighPriority.reset( new FileOperationPriority(
            new FileOperation(fileTest1Tape1), testTape1,
            priority.get(), timeout, 1) );
    fileLowPriority.reset( new FileOperationPriority(
            new FileOperation(fileTest2Tape1), testTape1,
            priority.get(), timeout, 0) );
    testFileOperationCreate( fileHighPriority, fileLowPriority );

    LogDebug("Step 2");
    fileHighPriority.reset( new FileOperationPriority(
            new FileOperation(fileTest1Tape1), testTape1,
            priority.get(), timeout, 1) );
    fileLowPriority.reset( new FileOperationPriority(
            new FileOperation(fileTest2Tape1), testTape1,
            priority.get(), timeout, 0) );
    testFileOperationOpen( fileHighPriority, fileLowPriority );

    LogDebug("Step 3");
    fileHighPriority.reset( new FileOperationPriority(
            new FileOperation(fileTest1Tape2), testTape2,
            priority.get(), timeout, 1) );
    fileLowPriority.reset( new FileOperationPriority(
            new FileOperation(fileTest2Tape2), testTape2,
            priority.get(), timeout, 0) );
    testFileOperationCreate( fileHighPriority, fileLowPriority );

    LogDebug("Step 4");
    fileHighPriority.reset( new FileOperationPriority(
            new FileOperation(fileTest1Tape2), testTape2,
            priority.get(), timeout, 1) );
    fileLowPriority.reset( new FileOperationPriority(
            new FileOperation(fileTest2Tape2), testTape2,
            priority.get(), timeout, 0) );
    testFileOperationOpen( fileHighPriority, fileLowPriority );
}


void
FileOperationPriorityTest::testFileOperationCreate(
        auto_ptr<FileOperationPriority> & fileHighPriority,
        auto_ptr<FileOperationPriority> & fileLowPriority)
{
    boost::posix_time::ptime begin;
    boost::posix_time::ptime current;
    int duration;

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == fileHighPriority->CreateFile(O_RDWR,0644,false) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    begin = boost::posix_time::microsec_clock::local_time();
    errno = 0;
    CPPUNIT_ASSERT( false == fileLowPriority->CreateFile(O_RDWR,0644,false) );
    CPPUNIT_ASSERT_ERRNO_( ENOMEDIUM == errno );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration >= timeout );

    fileHighPriority.reset();

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == fileLowPriority->CreateFile(O_RDWR,0644,false) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    fileLowPriority.reset();
}


void
FileOperationPriorityTest::testFileOperationOpen(
        auto_ptr<FileOperationPriority> & fileHighPriority,
        auto_ptr<FileOperationPriority> & fileLowPriority)
{
    boost::posix_time::ptime begin;
    boost::posix_time::ptime current;
    int duration;
    char bufferWrite[1024] = "Hello,world!\n";
    char bufferRead[sizeof(bufferWrite)];
    size_t size;

    CPPUNIT_ASSERT( true == fileHighPriority->OpenFile(O_RDWR) );
    CPPUNIT_ASSERT( false == fileLowPriority->OpenFile(O_RDWR) );

    begin = boost::posix_time::microsec_clock::local_time();
    errno = 0;
    size = sizeof(bufferRead);
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(
            true == fileHighPriority->Read(
                0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT_ERRNO_( 0 == errno );
    CPPUNIT_ASSERT( 0 == size );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    begin = boost::posix_time::microsec_clock::local_time();
    errno = 0;
    CPPUNIT_ASSERT(
            false == fileLowPriority->Write(
                0,bufferWrite,sizeof(bufferWrite),size) );
    CPPUNIT_ASSERT_ERRNO_( ENOMEDIUM == errno );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    begin = boost::posix_time::microsec_clock::local_time();
    errno = 0;
    CPPUNIT_ASSERT(
            false == fileLowPriority->Read(
                0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT_ERRNO_( ENOMEDIUM == errno );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    begin = boost::posix_time::microsec_clock::local_time();
    errno = 0;
    size = 0;
    CPPUNIT_ASSERT(
            true == fileHighPriority->Write(
                0,bufferWrite,sizeof(bufferWrite),size) );
    CPPUNIT_ASSERT_ERRNO_( 0 == errno );
    CPPUNIT_ASSERT( sizeof(bufferWrite) == size );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    begin = boost::posix_time::microsec_clock::local_time();
    errno = 0;
    CPPUNIT_ASSERT(
            false == fileLowPriority->Write(
                0,bufferWrite,sizeof(bufferWrite),size) );
    CPPUNIT_ASSERT_ERRNO_( ENOMEDIUM == errno );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    begin = boost::posix_time::microsec_clock::local_time();
    errno = 0;
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(
            true == fileHighPriority->Read(
                0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT_ERRNO_( 0 == errno );
    CPPUNIT_ASSERT( sizeof(bufferRead) == size );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == fileHighPriority->Truncate(2*sizeof(bufferWrite)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == fileHighPriority->Delete() );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    CPPUNIT_ASSERT( true == fileLowPriority->OpenFile(O_RDWR) );

    begin = boost::posix_time::microsec_clock::local_time();
    errno = 0;
    size = sizeof(bufferRead);
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(
            true == fileLowPriority->Read(
                0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT_ERRNO_( 0 == errno );
    CPPUNIT_ASSERT( 0 == size );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    begin = boost::posix_time::microsec_clock::local_time();
    errno = 0;
    size = 0;
    CPPUNIT_ASSERT(
            true == fileLowPriority->Write(
                0,bufferWrite,sizeof(bufferWrite),size) );
    CPPUNIT_ASSERT_ERRNO_( 0 == errno );
    CPPUNIT_ASSERT( sizeof(bufferWrite) == size );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    begin = boost::posix_time::microsec_clock::local_time();
    errno = 0;
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(
            true == fileLowPriority->Read(
                0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT_ERRNO_( 0 == errno );
    CPPUNIT_ASSERT( sizeof(bufferRead) == size );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT(
            true == fileLowPriority->Truncate(2*sizeof(bufferWrite)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

    begin = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == fileLowPriority->Delete() );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE( boost::lexical_cast<string>(duration),
            duration < timeout );

}


