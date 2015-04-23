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
 * FileOperationCIFSTest.cpp
 *
 *  Created on: Apr 24, 2012
 *      Author: More Zeng
 */


#include "stdafx.h"
#include "../MetaManager.h"
#include "FileOperationDelay.h"
#include "../CIFSWait.h"
#include "../FileOperationCIFS.h"
#include "FileOperationCIFSTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( FileOperationCIFSTest );


static const string cacheFolder = "FileOperationCIFSTest-cache";
static const string metaFolder = "FileOperationCIFSTest-meta";
static const string tapeFolder = "FileOperationCIFSTest-tape";
static string testFile = "/test-file";


void
FileOperationCIFSTest::setUp()
{
    fs::create_directory(cacheFolder);
    fs::create_directory(metaFolder);
    fs::create_directory(tapeFolder);

    Factory::SetCacheFolder(tapeFolder);
    Factory::SetMetaFolder(metaFolder);
    Factory::CreateCacheManager();
}


void
FileOperationCIFSTest::tearDown()
{
    Factory::ReleaseCacheManager();
    fs::remove_all(cacheFolder);
    fs::remove_all(metaFolder);
    fs::remove_all(tapeFolder);
}


void
FileOperationCIFSTest::testFileOperation()
{
    testFile = "FileOperationCIFSTest_testFileOperation";
    fs::path testFilePath = fs::path(tapeFolder) / testFile;

    MetaManager meta;
    auto_ptr<Inode> inode;
    auto_ptr<FileOperationInterface> oper;

    char bufferWrite[1024] = "hello,world!\n";
    char bufferRead[sizeof(bufferWrite)];
    size_t size;
    const size_t sizeFile = 2 * 1024 * 1024;
    boost::posix_time::ptime begin;
    boost::posix_time::ptime current;


    CPPUNIT_ASSERT( true == meta.CreateFile(testFile,0600) );
    inode.reset( meta.GetInode(testFile) );
    CPPUNIT_ASSERT( NULL != inode.get() );

    oper.reset( new FileOperationCIFS( inode.release(),
            new FileOperationDelay(testFilePath,0600,O_RDWR,10), 0 ) );
    CPPUNIT_ASSERT( NULL != oper.get() );
    {
        size_t offset = 0;
        while ( offset < sizeFile ) {
            size = 0;
            CPPUNIT_ASSERT( true == oper->Write(
                    offset,bufferWrite,sizeof(bufferWrite),size) );
            CPPUNIT_ASSERT( size == sizeof(bufferWrite) );
            offset += size;
        }
    }

    oper.reset();


    oper.reset( new FileOperationCIFS( meta.GetInode(testFile),
            new FileOperationDelay(testFilePath,O_RDONLY,10), 0 ) );
    CPPUNIT_ASSERT( NULL != oper.get() );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    int duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration <= 2 );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(
            true == oper->Read(1024*1024,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration >= 8 );

    errno = 0;
    CPPUNIT_ASSERT(
            false == oper->Write(0,bufferWrite,sizeof(bufferWrite),size) );
    CPPUNIT_ASSERT_ERRNO_( EBADF == errno );

    oper.reset();


    oper.reset( new FileOperationCIFS( meta.GetInode(testFile),
            new FileOperationDelay(testFilePath,O_RDWR,10), 0 ) );
    CPPUNIT_ASSERT( NULL != oper.get() );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration <= 2 );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT( true == oper->Read(
            1024*1024-1, bufferRead, sizeof(bufferRead), size ) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(
            bufferRead+1, bufferWrite, sizeof(bufferRead)-1 ) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration >= 8 );

    oper.reset();


    oper.reset( new FileOperationCIFS( meta.GetInode(testFile),
            new FileOperationDelay(testFilePath,O_RDWR,10), 0 ) );
    CPPUNIT_ASSERT( NULL != oper.get() );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT( true == oper->Read(
            1024*1024-1, bufferRead, sizeof(bufferRead), size ) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(
            bufferRead+1, bufferWrite, sizeof(bufferRead)-1 ) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration >= 8 );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    current = boost::posix_time::microsec_clock::local_time();
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration <= 2 );

    oper.reset();


    oper.reset( new FileOperationCIFS( meta.GetInode(testFile),
            new FileOperationDelay(testFilePath,O_RDWR,10), 0 ) );
    CPPUNIT_ASSERT( NULL != oper.get() );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration <= 2 );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(
            true == oper->Read( sizeFile - sizeof(bufferRead), bufferRead,
                sizeof(bufferRead), size ) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration <= 2 );

    CPPUNIT_ASSERT( true == oper->Truncate(1024 * 1024 + sizeof(bufferRead)) );

    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(
            true == oper->Read(1024*1024,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );

    oper.reset();


    oper.reset( new FileOperationCIFS( meta.GetInode(testFile),
            new FileOperationDelay(testFilePath,O_RDWR,10), 0 ) );
    CPPUNIT_ASSERT( NULL != oper.get() );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration <= 2 );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(
            true == oper->Read(1024*1024,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration <= 2 );

    oper.reset();


    CPPUNIT_ASSERT( true == meta.DeleteInode(testFile) );
    CPPUNIT_ASSERT( true == meta.CreateFile(testFile,0600) );
    inode.reset(meta.GetInode(testFile));
    CPPUNIT_ASSERT( NULL != inode.get() );

    oper.reset( new FileOperationCIFS( inode.release(),
            new FileOperationDelay(testFilePath,O_RDWR,10), 0 ) );
    CPPUNIT_ASSERT( NULL != oper.get() );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration >= 8 );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(
            true == oper->Read(1024*1024,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration <= 2 );

    oper.reset();


    CPPUNIT_ASSERT( true == meta.DeleteInode(testFile) );
    CPPUNIT_ASSERT( true == meta.CreateFile(testFile,0600) );
    inode.reset( meta.GetInode(testFile) );
    CPPUNIT_ASSERT( NULL != inode.get() );

    oper.reset( new FileOperationCIFS( inode.release(),
            new FileOperationDelay(testFilePath,O_RDWR,10), 0 ) );
    CPPUNIT_ASSERT( NULL != oper.get() );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT(
            true == oper->Read(1024*1024,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration >= 8 );

    begin = boost::posix_time::microsec_clock::local_time();
    size = 0;
    memset(bufferRead,0,sizeof(bufferRead));
    CPPUNIT_ASSERT( true == oper->Read(0,bufferRead,sizeof(bufferRead),size) );
    CPPUNIT_ASSERT( size == sizeof(bufferRead) );
    CPPUNIT_ASSERT( 0 == memcmp(bufferRead,bufferWrite,sizeof(bufferRead)) );
    current = boost::posix_time::microsec_clock::local_time();
    duration = (current - begin).total_milliseconds();
    CPPUNIT_ASSERT_MESSAGE(
            boost::lexical_cast<string>(duration), duration <= 2 );

    oper.reset();
}


