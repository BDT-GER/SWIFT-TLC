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
 * FileOperationTest.cpp
 *
 *  Created on: Mar 2, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../FileOperation.h"
#include "FileOperationTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( FileOperationTest );


static string testFile = "test-file";


void
FileOperationTest::setUp()
{
}


void
FileOperationTest::tearDown()
{
    fs::remove(testFile);
    CPPUNIT_ASSERT( ! fs::exists(testFile) );
}


void
FileOperationTest::testFileOperation()
{
    testFile = "FileOperationTest_testFileOperation";

    auto_ptr<FileOperation> oper;
    auto_ptr<FileOperation> oper2;
    struct stat stat;
    char buffer[4096] = "Hello,world!\n";
    char readBuffer[sizeof(buffer)];
    size_t size;


    CPPUNIT_ASSERT_THROW_MESSAGE(
            strerror(errno),
            oper.reset(new FileOperation(testFile,O_RDWR)),
            ExceptionFileNonExist);

    CPPUNIT_ASSERT_NO_THROW(oper.reset(new FileOperation(testFile,0644,O_RDWR)));

    errno = 0;
    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT_ERRNO_( 0 == errno );
    CPPUNIT_ASSERT_EQUAL( (off_t)0, stat.st_size );
    CPPUNIT_ASSERT( S_ISREG(stat.st_mode) );

    memset(readBuffer,0,sizeof(buffer));
    errno = 0;
    size = sizeof(buffer);
    CPPUNIT_ASSERT( true == oper->Read(0,readBuffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_ERRNO_( 0 == errno );
    CPPUNIT_ASSERT_EQUAL( (size_t)0, size );

    errno = 0;
    size = 0;
    CPPUNIT_ASSERT( true == oper->Write(0,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_ERRNO_( 0 == errno );
    CPPUNIT_ASSERT_EQUAL( sizeof(buffer), size );

    memset(readBuffer,0,sizeof(buffer));
    size = 0;
    CPPUNIT_ASSERT( true == oper->Read(0,readBuffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_EQUAL( sizeof(buffer), size );
    CPPUNIT_ASSERT( 0 == memcmp(buffer,readBuffer,sizeof(buffer)) );
    CPPUNIT_ASSERT_ERRNO_( 0 == errno );

    errno = 0;
    CPPUNIT_ASSERT( true == oper->Sync(true) );
    CPPUNIT_ASSERT_ERRNO_( 0 == errno );

    errno = 0;
    CPPUNIT_ASSERT( true == oper->Sync(false) );
    CPPUNIT_ASSERT_ERRNO_( 0 == errno );

    errno = 0;
    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT_ERRNO_( 0 == errno );
    CPPUNIT_ASSERT_EQUAL( (off_t)sizeof(buffer), stat.st_size );
    CPPUNIT_ASSERT( S_ISREG(stat.st_mode) );

    memset(readBuffer,0,sizeof(buffer));
    size = 0;
    CPPUNIT_ASSERT( true == oper->Read(100,readBuffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_EQUAL( sizeof(buffer) - 100, size );
    CPPUNIT_ASSERT( 0 == memcmp(buffer+100,readBuffer,sizeof(buffer)-100) );

    size = 0;
    CPPUNIT_ASSERT( true == oper->Write(100,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_EQUAL( sizeof(buffer), size );

    memset(readBuffer,0,sizeof(buffer));
    size = 0;
    CPPUNIT_ASSERT( true == oper->Read(100,readBuffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_EQUAL( sizeof(buffer), size );
    CPPUNIT_ASSERT( 0 == memcmp(buffer,readBuffer,sizeof(buffer)) );

    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT_EQUAL( (off_t)(100 + sizeof(buffer)), stat.st_size );
    CPPUNIT_ASSERT( S_ISREG(stat.st_mode) );

    CPPUNIT_ASSERT( true == oper->Sync(true) );
    CPPUNIT_ASSERT( true == oper->Sync(false) );

    CPPUNIT_ASSERT( true == oper->Truncate(sizeof(buffer)) );
    CPPUNIT_ASSERT( true == oper->GetStat(stat) );
    CPPUNIT_ASSERT_EQUAL( (off_t)sizeof(buffer), stat.st_size );
    CPPUNIT_ASSERT( S_ISREG(stat.st_mode) );

    CPPUNIT_ASSERT( true == oper->Sync(true) );
    CPPUNIT_ASSERT( true == oper->Sync(false) );


    CPPUNIT_ASSERT_THROW(
            oper2.reset(new FileOperation(testFile,0644,O_RDWR)),
            ExceptionFileExistedAlready);

    CPPUNIT_ASSERT_NO_THROW(oper2.reset(new FileOperation(testFile,O_RDWR)));

    size = 0;
    CPPUNIT_ASSERT( true == oper2->Write(0,buffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_EQUAL( sizeof(buffer), size );

    memset(readBuffer,0,sizeof(buffer));
    size = 0;
    CPPUNIT_ASSERT( true == oper->Read(0,readBuffer,sizeof(buffer),size) );
    CPPUNIT_ASSERT_EQUAL( sizeof(buffer), size );
    CPPUNIT_ASSERT( 0 == memcmp(buffer,readBuffer,sizeof(buffer)) );

    CPPUNIT_ASSERT( true == oper->Truncate(0) );
    CPPUNIT_ASSERT( true == oper2->GetStat(stat) );
    CPPUNIT_ASSERT_EQUAL( (off_t)0, stat.st_size );
    CPPUNIT_ASSERT( S_ISREG(stat.st_mode) );
}

